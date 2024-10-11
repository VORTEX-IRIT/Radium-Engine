#include <Core/Tasks/Task.hpp>
#include <Core/Tasks/TaskQueue.hpp>
#include <Core/Utils/Log.hpp>

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <stack>

namespace Ra {
namespace Core {

TaskQueue::TaskQueue( uint numThreads ) : m_processingTasks( 0 ), m_shuttingDown( false ) {
    wlock lock( m_mutex );
    m_workerThreads.reserve( numThreads );
    for ( uint i = 0; i < numThreads; ++i ) {
        m_workerThreads.emplace_back( &TaskQueue::runThread, this, i );
    }
}

TaskQueue::~TaskQueue() {
    {
        wlock lock( m_mutex );
        m_shuttingDown = true;
    }
    flushTaskQueue();
    for ( auto& t : m_workerThreads ) {
        t.join();
    }
}

TaskQueue::TaskId TaskQueue::registerTask( std::unique_ptr<Task> task ) {
    wlock lock( m_mutex );
    TimerData tdata;
    // init tdata with task name before moving ownership
    tdata.taskName = task->getName();
    m_timerData.push_back( tdata );

    m_tasks.push_back( std::move( task ) );
    m_dependencies.push_back( std::vector<TaskId>() );
    m_remainingDependencies.push_back( 0 );

    CORE_ASSERT( m_tasks.size() == m_dependencies.size(), "Inconsistent task list" );
    CORE_ASSERT( m_tasks.size() == m_remainingDependencies.size(), "Inconsistent task list" );
    CORE_ASSERT( m_tasks.size() == m_timerData.size(), "Inconsistent task list" );
    return TaskId { m_tasks.size() - 1 };
}

void TaskQueue::removeTask( TaskId taskId ) {

    if ( taskId.isInvalid() || taskId > m_tasks.size() ) {
        LOG( Utils::logDEBUG ) << "try to remove task " << taskId << " which is out of bounds "
                               << m_tasks.size();
        return;
    }
    wlock lock( m_mutex );

    // set task as dummy noop
    m_tasks[taskId] = std::make_unique<FunctionTask>( []() {}, m_timerData[taskId].taskName );

    CORE_ASSERT( m_tasks.size() == m_dependencies.size(), "Inconsistent task list" );
    CORE_ASSERT( m_tasks.size() == m_remainingDependencies.size(), "Inconsistent task list" );
    CORE_ASSERT( m_tasks.size() == m_timerData.size(), "Inconsistent task list" );
}

TaskQueue::TaskId TaskQueue::getTaskId( const std::string& taskName ) const {
    rlock lock( m_mutex );
    auto itr = std::find_if( m_tasks.begin(), m_tasks.end(), [taskName]( const auto& task ) {
        return task->getName() == taskName;
    } );

    if ( itr == m_tasks.end() ) return {};
    return TaskId { itr - m_tasks.begin() };
}

void TaskQueue::addDependency( TaskQueue::TaskId predecessor, TaskQueue::TaskId successor ) {
    wlock lock( m_mutex );

    CORE_ASSERT( predecessor.isValid() && ( predecessor < m_tasks.size() ),
                 "Invalid predecessor task" );
    CORE_ASSERT( successor.isValid() && ( successor < m_tasks.size() ), "Invalid successor task" );
    CORE_ASSERT( predecessor != successor, "Cannot add self-dependency" );

    CORE_ASSERT( std::find( m_dependencies[predecessor].begin(),
                            m_dependencies[predecessor].end(),
                            successor ) == m_dependencies[predecessor].end(),
                 "Cannot add a dependency twice" );

    m_dependencies[predecessor].push_back( successor );
    ++m_remainingDependencies[successor];
}

bool TaskQueue::addDependency( const std::string& predecessor, TaskQueue::TaskId successor ) {
    bool added = false;

    auto predecessorId = getTaskId( predecessor );

    if ( predecessorId.isValid() ) {
        added = true;
        addDependency( predecessorId, successor );
    }
    return added;
}

bool TaskQueue::addDependency( TaskQueue::TaskId predecessor, const std::string& successor ) {
    bool added = false;

    auto successorId = getTaskId( successor );
    if ( successorId.isValid() ) {
        added = true;
        addDependency( predecessor, successorId );
    }
    return added;
}

void TaskQueue::addPendingDependency( const std::string& predecessors,
                                      TaskQueue::TaskId successor ) {
    wlock lock( m_mutex );
    m_pendingDepsSucc.emplace_back( predecessors, successor );
}

void TaskQueue::addPendingDependency( TaskId predecessor, const std::string& successors ) {
    wlock lock( m_mutex );
    m_pendingDepsPre.emplace_back( predecessor, successors );
}

void TaskQueue::resolveDependencies() {
    for ( const auto& pre : m_pendingDepsPre ) {
        ON_ASSERT( bool result = ) addDependency( pre.first, pre.second );
        CORE_WARN_IF( !result,
                      "Pending dependency unresolved : " << m_tasks[pre.first]->getName() << " -> ("
                                                         << pre.second << ")" );
    }
    for ( const auto& pre : m_pendingDepsSucc ) {
        ON_ASSERT( bool result = ) addDependency( pre.first, pre.second );
        CORE_WARN_IF( !result,
                      "Pending dependency unresolved : (" << pre.first << ") -> "
                                                          << m_tasks[pre.second]->getName() );
    }
    {
        wlock lock( m_mutex );
        m_pendingDepsPre.clear();
        m_pendingDepsSucc.clear();
    }
}

// queueTask is always called with m_taskQueueMutex locked
void TaskQueue::queueTask( TaskQueue::TaskId task ) {
    CORE_ASSERT( m_remainingDependencies[task] == 0,
                 " Task" << m_tasks[task]->getName() << "has unmet dependencies" );

    m_taskQueue.push_front( task );
}

void TaskQueue::detectCycles() {
#if defined( CORE_DEBUG )
    // Do a depth-first search of the nodes.
    std::vector<bool> visited( m_tasks.size(), false );
    std::stack<TaskId> pending;
    rlock lock( m_mutex );
    for ( uint id = 0; id < m_tasks.size(); ++id ) {
        if ( m_dependencies[id].size() == 0 ) { pending.push( TaskId( id ) ); }
    }

    // If you hit this assert, there are tasks in the list but
    // all tasks have dependencies so no task can start.
    CORE_ASSERT( m_tasks.empty() || !pending.empty(), "No free tasks." );

    while ( !pending.empty() ) {
        TaskId id = pending.top();
        pending.pop();

        // The task has already been visited. It means there is a cycle in the task graph.
        CORE_ASSERT( !( visited[id] ), "Cycle detected in tasks !" );

        visited[id] = true;
        for ( const auto& dep : m_dependencies[id] ) {
            pending.push( dep );
        }
    }
#endif
}

void TaskQueue::startTasks() {
    using namespace Ra::Core::Utils;
    {
        rlock lock( m_mutex );
        if ( m_workerThreads.empty() ) {
            LOG( logERROR ) << "TaskQueue as 0 threads, could not start tasks in parallel. Either "
                               "create a task queue with more threads, or use runTasksInThisThread";
            return;
        }
    }
    // Add pending dependencies.
    resolveDependencies();

    // Do a debug check
    detectCycles();

    // Enqueue all tasks with no dependencies.
    {
        wlock lock( m_mutex );
        for ( uint t = 0; t < m_tasks.size(); ++t ) {
            // only queue non null m_tasks

            if ( m_tasks[t] && m_remainingDependencies[t] == 0 ) { queueTask( TaskId { t } ); }
        }
    }
    // Wake up all threads.
    m_threadNotifier.notify_all();
}

void TaskQueue::runTasksInThisThread() {
    // this method should not be called between startTasks/waitForTasks, we do not lock anything
    // here.

    // use local task queue, preventing workers to pickup jobs.
    std::deque<TaskId> taskQueue;

    // Add pending dependencies.
    resolveDependencies();

    // Do a debug check
    detectCycles();
    {
        // Enqueue all tasks with no dependencies.
        for ( uint t = 0; t < m_tasks.size(); ++t ) {
            // only queue non null m_tasks
            if ( m_tasks[t] && m_remainingDependencies[t] == 0 ) {
                taskQueue.push_front( TaskId { t } );
            }
        }
    }
    {
        while ( !taskQueue.empty() ) {
            TaskId task;
            {
                task = taskQueue.back();
                taskQueue.pop_back();
            }
            // Run task
            m_timerData[task].start    = Utils::Clock::now();
            m_timerData[task].threadId = 0;
            m_tasks[task]->process();
            m_timerData[task].end = Utils::Clock::now();

            for ( auto t : m_dependencies[task] ) {
                uint& nDepends = m_remainingDependencies[t];
                CORE_ASSERT( nDepends > 0, "Inconsistency in dependencies" );
                --nDepends;
                if ( nDepends == 0 ) { taskQueue.push_front( TaskId { t } ); }
            }
        }
    }
    flushTaskQueue();
}

void TaskQueue::waitForTasks() {
    rlock lock( m_mutex );
    m_waitForTasksNotifier.wait(
        lock, [this]() { return ( m_taskQueue.empty() && m_processingTasks == 0 ); } );
}

const std::vector<TaskQueue::TimerData>& TaskQueue::getTimerData() {
    return m_timerData;
}

void TaskQueue::flushTaskQueue() {
    m_threadNotifier.notify_all();
    wlock lock( m_mutex );

    CORE_ASSERT( m_processingTasks == 0, "You have tasks still in process" );
    CORE_ASSERT( m_taskQueue.empty(), " You have unprocessed tasks " );

    m_tasks.clear();
    m_dependencies.clear();
    m_timerData.clear();
    m_remainingDependencies.clear();
}

void TaskQueue::runThread( uint id ) {
    while ( true ) {
        TaskId task;

        // Acquire mutex.
        {
            wlock lock( m_mutex );
            // Wait for a new task
            m_threadNotifier.wait( lock,
                                   [this]() { return m_shuttingDown || !m_taskQueue.empty(); } );
            // If the task queue is shutting down we quit, releasing
            // the lock.
            if ( m_shuttingDown ) { return; }

            // If we are here it means we got a task
            task = m_taskQueue.back();
            m_taskQueue.pop_back();
            ++m_processingTasks;
            CORE_ASSERT( task.isValid() && task < m_tasks.size(), "Invalid task" );
        }
        // Release mutex.

        // Run task
        {
            {
                rlock lock( m_mutex );
                m_timerData[task].start    = Utils::Clock::now();
                m_timerData[task].threadId = id;
            }
            m_tasks[task]->process();
            {
                rlock lock( m_mutex );
                m_timerData[task].end = Utils::Clock::now();
            }
        }
        // Critical section : mark task as finished and en-queue dependencies.
        uint newTasks = 0;
        {
            wlock lock( m_mutex );
            for ( auto t : m_dependencies[task] ) {
                uint& nDepends = m_remainingDependencies[t];
                CORE_ASSERT( nDepends > 0, "Inconsistency in dependencies" );
                --nDepends;
                if ( nDepends == 0 ) {
                    queueTask( t );
                    ++newTasks;
                }
            }
            --m_processingTasks;
            if ( m_processingTasks == 0 ) { m_waitForTasksNotifier.notify_one(); }
        }
        // If we added new tasks, we wake up threads to execute thems.
        if ( newTasks > 0 ) { m_threadNotifier.notify_all(); }
    } // End of while(true)
}

void TaskQueue::printTaskGraph( std::ostream& output ) const {
    output << "digraph tasks {" << std::endl;

    for ( const auto& t : m_tasks ) {
        output << "\"" << t->getName() << "\"" << std::endl;
    }

    for ( uint i = 0; i < m_dependencies.size(); ++i ) {
        const auto& task1 = m_tasks[i];
        for ( const auto& dep : m_dependencies[i] ) {
            const auto& task2 = m_tasks[dep];
            output << "\"" << task1->getName() << "\""
                   << " -> ";
            output << "\"" << task2->getName() << "\"" << std::endl;
        }
    }

    for ( const auto& preDep : m_pendingDepsPre ) {
        const auto& task1  = m_tasks[preDep.first];
        std::string t2name = preDep.second;

        if ( std::find_if( m_tasks.begin(), m_tasks.end(), [=]( const auto& task ) {
                 return task->getName() == t2name;
             } ) == m_tasks.end() ) {
            t2name += "?";
        }
        output << "\"" << task1->getName() << "\""
               << " -> ";
        output << "\"" << t2name << "\"" << std::endl;
    }

    for ( const auto& postDep : m_pendingDepsSucc ) {
        std::string t1name = postDep.first;
        const auto& t2     = m_tasks[postDep.second];

        if ( std::find_if( m_tasks.begin(), m_tasks.end(), [=]( const auto& task ) {
                 return task->getName() == t1name;
             } ) == m_tasks.end() ) {
            t1name += "?";
        }
        output << "\"" << t1name << "\""
               << " -> ";
        output << "\"" << t2->getName() << "\"" << std::endl;
    }

    output << "}" << std::endl;
}
} // namespace Core
} // namespace Ra
