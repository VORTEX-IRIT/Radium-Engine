#pragma once

#include <Core/RaCore.hpp>
#include <Core/Utils/Index.hpp>
#include <Core/Utils/Timer.hpp> // Ra::Core::TimePoint

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

namespace Ra {
namespace Core {
class Task;
}
} // namespace Ra

namespace Ra {
namespace Core {
/** \brief This class allows tasks to be registered and then executed in parallel on separate
 * threads.
 *
 * It maintains an internal pool of threads. When instructed, it dispatches the tasks to the
 * pooled threads.
 * Task are allowed to have dependencies. A task will be executed only when all its dependencies
 * are satisfied, i.e. all dependant tasks are finished.
 * Note that most functions are not thread safe and must not be called when the task queue is
 * running.
 * Typical usage:
\code
    TaskQueue taskQueue( 4 );
    auto task = std::make_unique<FunctionTask>( ... );
    auto tid = taskQueue.registerTask( std::move( task ) );
    // [...]
    taskQueue.addDependency( tid, ... );
    // [...]
    taskQueue.startTasks();
    taskQueue.waitForTasks();
    taskQueue.flushTaskQueue();
\endcode
 */
class RA_CORE_API TaskQueue
{
  public:
    /// Identifier for a task in the task queue.
    using TaskId = Utils::Index;

    /// Record of a task's start and end time.
    struct TimerData {
        Utils::TimePoint start;
        Utils::TimePoint end;
        uint threadId;
        std::string taskName;
    };

  public:
    /// Constructor. Initializes the thread worker pools with numThreads threads.
    /// if numThreads == 0, its a runTasksInThisThread only task queue
    explicit TaskQueue( uint numThreads );

    /// Destructor. Waits for all the threads and safely deletes them.
    ~TaskQueue();

    //
    // Task management
    //

    /// Registers a task to be executed.
    /// Task must have been created with new and be initialized with its parameter.
    /// The task queue assumes ownership of the task.
    TaskId registerTask( std::unique_ptr<Task> task );

    /// remove a task, in fact simply replace the task by a dummy empty one.
    /// hence do not affect dependencies
    /// don't affect other task id's
    void removeTask( TaskId taskId );

    TaskId getTaskId( const std::string& taskName ) const;

    /// Add dependency between two tasks. The successor task will be executed only when all
    /// its predecessor completed.
    void addDependency( TaskId predecessor, TaskId successor );

    /// Add dependency between a task and all task with a given name.
    /// Will return false if no dependency has been added.
    bool addDependency( const std::string& predecessors, TaskId successor );
    bool addDependency( TaskId predecessor, const std::string& successors );

    /// Add a dependency between a task an all tasks with a given name, even
    /// if the task is not present yet, the name being resolved when task start.
    void addPendingDependency( const std::string& predecessors, TaskId successor );
    void addPendingDependency( TaskId predecessor, const std::string& successors );

    //
    // Task queue operations
    //

    /// Launches the execution of all the tasks in the task queue.
    /// No more tasks should be added at this point.
    void startTasks();

    /// Launches the execution of all task in the thread of the caller.
    /// Return when all tasks are done. Usefull for instance for opengl related tasks that must run
    /// in the context thread.
    /// Once tasks are all processed, this method call flushTasksQueue.
    /// \warning use either this method, either a startTasks/waitForTasks. calling
    /// runTasksInThisThead between startTasks/waitForTasks calls may produce unexpected results.
    void runTasksInThisThread();

    /// Blocks until all tasks and dependencies are finished.
    void waitForTasks();

    /// Access the data from the last frame execution after processTaskQueue();
    const std::vector<TimerData>& getTimerData();

    /// Erases all tasks. Will assert if tasks are unprocessed.
    void flushTaskQueue();

    /// Prints the current task graph in dot format
    void printTaskGraph( std::ostream& output ) const;

  private:
    /// Function called by a new thread.
    void runThread( uint id );

    /// Puts the task on the queue to be executed. A task can only be queued if it has
    /// no dependencies.
    void queueTask( TaskId task );

    /// Detect if there are any cycles in the task graph, and asserts if it is the case.
    /// (this function is compiled to nothing in release).
    void detectCycles();

    /// Resolves the pending named dependencies. Will assert if dependencies don't resolve.
    void resolveDependencies();

  private:
    /// write lock, only one at a time
    using wlock = std::unique_lock<std::shared_mutex>;
    /// read lock, multiple lock allowed
    using rlock = std::shared_lock<std::shared_mutex>;

    /// Threads working on tasks.
    std::vector<std::thread> m_workerThreads;

    //
    // mutex protected variables.
    //

    /// Storage for the tasks (task will be deleted after flushQueue()).
    std::vector<std::unique_ptr<Task>> m_tasks;
    /// For each task, stores which tasks depend on it.
    std::vector<std::vector<TaskId>> m_dependencies;

    /// List of pending dependencies
    std::vector<std::pair<TaskId, std::string>> m_pendingDepsPre;
    std::vector<std::pair<std::string, TaskId>> m_pendingDepsSucc;

    /// Stores the timings of each frame after execution.
    std::vector<TimerData> m_timerData;

    /// Number of tasks each task is waiting on.
    std::vector<uint> m_remainingDependencies;
    /// Queue holding the pending tasks.
    std::deque<TaskId> m_taskQueue;
    /// Number of tasks currently being processed.
    uint m_processingTasks;

    /// Flag to signal threads to quit.
    std::atomic_bool m_shuttingDown;
    /// Variable on which threads wait for new tasks.
    std::condition_variable_any m_threadNotifier;
    std::condition_variable_any m_waitForTasksNotifier;

    /// mutex for protected variable
    mutable std::shared_mutex m_mutex;
};

} // namespace Core
} // namespace Ra
