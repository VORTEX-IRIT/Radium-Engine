#include <Core/Tasks/Task.hpp>
#include <Core/Tasks/TaskQueue.hpp>

#include <catch2/catch.hpp>
#include <memory>
#include <string>

using namespace Ra::Core;
using namespace Ra::Core::Utils;

// run some dummy task, to check that there is no deadlock
TEST_CASE( "Core/TaskQueueInit", "[Core][TaskQueue]" ) {
    for ( int i = 0; i < 5; ++i ) {
        TaskQueue taskQueue1( 10 );
        for ( int j = 0; j < 20; ++j ) {
            taskQueue1.registerTask( std::make_unique<FunctionTask>(
                []() { std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) ); }, "" ) );
        }
        taskQueue1.startTasks();
        taskQueue1.waitForTasks();
        taskQueue1.flushTaskQueue();
    }
    for ( int i = 0; i < 50; ++i ) {
        TaskQueue taskQueue1( 10 );
        TaskQueue taskQueue2( 20 );
        TaskQueue taskQueue3( 30 );
        taskQueue1.startTasks();
        taskQueue1.waitForTasks();
        taskQueue1.flushTaskQueue();
        taskQueue2.startTasks();
        taskQueue2.waitForTasks();
        taskQueue2.flushTaskQueue();
        taskQueue3.startTasks();
        taskQueue3.waitForTasks();
        taskQueue3.flushTaskQueue();
    }
}

TEST_CASE( "Core/TaskQueue", "[Core][TaskQueue]" ) {
    TaskQueue taskQueue( 4 );

    const int arraySize  = 7; // if changed, update test values also
    int array[arraySize] = { -1, -1, -1, -1, -1, -1, -1 };

    SECTION( "no dependency" ) {
        for ( int tidx = 0; tidx < arraySize; ++tidx ) {
            auto task =
                std::make_unique<FunctionTask>( [&array, tidx]() { array[tidx] = tidx; },
                                                std::string( "task " ) + std::to_string( tidx ) );
            auto tid = taskQueue.registerTask( std::move( task ) );
            REQUIRE( tid ==
                     taskQueue.getTaskId( std::string( "task " ) + std::to_string( tidx ) ) );
        }
        std::ostringstream oss;
        taskQueue.printTaskGraph( oss );
        std::string s = oss.str();
        REQUIRE( s == "digraph tasks {\n"
                      "\"task 0\"\n"
                      "\"task 1\"\n"
                      "\"task 2\"\n"
                      "\"task 3\"\n"
                      "\"task 4\"\n"
                      "\"task 5\"\n"
                      "\"task 6\"\n"
                      "}\n" );
        SECTION( "parallel run" ) {
            taskQueue.startTasks();
            taskQueue.waitForTasks();
            taskQueue.flushTaskQueue();
        }
        SECTION( "one thread run" ) {
            taskQueue.runTasksInThisThread();
        }
        for ( int tidx = 0; tidx < arraySize; ++tidx ) {
            REQUIRE( array[tidx] == tidx );
        }
    }
    SECTION( "dependencies" ) {
        TaskQueue::TaskId tids[arraySize];
        for ( int tidx = 0; tidx < 4; ++tidx ) {
            auto task =
                std::make_unique<FunctionTask>( [&array, tidx]() { array[tidx] = tidx; },
                                                std::string( "task " ) + std::to_string( tidx ) );
            tids[tidx] = taskQueue.registerTask( std::move( task ) );
            REQUIRE( tids[tidx] ==
                     taskQueue.getTaskId( std::string( "task " ) + std::to_string( tidx ) ) );
            if ( tidx < 2 ) {
                taskQueue.addPendingDependency( tids[tidx],
                                                std::string( "task " ) + std::to_string( 4 ) );
            }
        }
        for ( int tidx = 4; tidx < 6; ++tidx ) {
            int pred1 = 2 * ( tidx - 4 );
            int pred2 = 2 * ( tidx - 4 ) + 1;
            auto task = std::make_unique<FunctionTask>(
                [&array, tidx, pred1, pred2]() { array[tidx] = array[pred1] + array[pred2]; },
                std::string( "task " ) + std::to_string( tidx ) );
            auto tid   = taskQueue.registerTask( std::move( task ) );
            tids[tidx] = tid;
            if ( tidx == 5 ) {
                taskQueue.addDependency( std::string( "task " ) + std::to_string( pred1 ), tid );
                taskQueue.addDependency( std::string( "task " ) + std::to_string( pred2 ), tid );
            }
        }
        {
            auto task =
                std::make_unique<FunctionTask>( [&array]() { array[6] = array[4] + array[5]; },
                                                std::string( "task " ) + std::to_string( 6 ) );
            taskQueue.registerTask( std::move( task ) );
            taskQueue.addDependency( tids[4], std::string( "task " ) + std::to_string( 6 ) );
            taskQueue.addDependency( tids[5], std::string( "task " ) + std::to_string( 6 ) );
        }

        SECTION( "parallel run" ) {
            taskQueue.startTasks();
            taskQueue.waitForTasks();
            taskQueue.flushTaskQueue();
        }
        SECTION( "one thread run" ) {
            taskQueue.runTasksInThisThread();
        }
        REQUIRE( array[0] == 0 );
        REQUIRE( array[1] == 1 );
        REQUIRE( array[2] == 2 );
        REQUIRE( array[3] == 3 );
        REQUIRE( array[4] == 1 ); // 0+1
        REQUIRE( array[5] == 5 ); // 2+3
        REQUIRE( array[6] == 6 ); // 5+1
    }

    SECTION( "remove with dependencies" ) {
        TaskQueue::TaskId tids[arraySize];
        for ( int tidx = 0; tidx < 4; ++tidx ) {
            auto task =
                std::make_unique<FunctionTask>( [&array, tidx]() { array[tidx] = tidx; },
                                                std::string( "task " ) + std::to_string( tidx ) );
            tids[tidx] = taskQueue.registerTask( std::move( task ) );
            REQUIRE( tids[tidx] ==
                     taskQueue.getTaskId( std::string( "task " ) + std::to_string( tidx ) ) );
            if ( tidx < 2 ) {
                taskQueue.addPendingDependency( tids[tidx],
                                                std::string( "task " ) + std::to_string( 4 ) );
            }
        }
        for ( int tidx = 4; tidx < 6; ++tidx ) {
            int pred1 = 2 * ( tidx - 4 );
            int pred2 = 2 * ( tidx - 4 ) + 1;
            auto task = std::make_unique<FunctionTask>(
                [&array, tidx, pred1, pred2]() { array[tidx] = array[pred1] + array[pred2]; },
                std::string( "task " ) + std::to_string( tidx ) );
            auto tid   = taskQueue.registerTask( std::move( task ) );
            tids[tidx] = tid;
            if ( tidx == 5 ) {
                taskQueue.addDependency( std::string( "task " ) + std::to_string( pred1 ), tid );
                taskQueue.addDependency( std::string( "task " ) + std::to_string( pred2 ), tid );
            }
        }
        {
            auto task =
                std::make_unique<FunctionTask>( [&array]() { array[6] = array[4] + array[5]; },
                                                std::string( "task " ) + std::to_string( 6 ) );
            tids[6] = taskQueue.registerTask( std::move( task ) );
            taskQueue.addDependency( tids[4], std::string( "task " ) + std::to_string( 6 ) );
            taskQueue.addDependency( tids[5], std::string( "task " ) + std::to_string( 6 ) );
        }
        taskQueue.removeTask( tids[0] );
        taskQueue.removeTask( tids[6] );

        // remove invalid task id, silently
        taskQueue.removeTask( TaskQueue::TaskId {} );
        taskQueue.removeTask( 100 );

        SECTION( "parallel run" ) {
            taskQueue.startTasks();
            taskQueue.waitForTasks();
            taskQueue.flushTaskQueue();
        }

        SECTION( "one thread run" ) {
            taskQueue.runTasksInThisThread();
        }

        REQUIRE( array[0] == -1 ); // task 0 removed
        REQUIRE( array[1] == 1 );
        REQUIRE( array[2] == 2 );
        REQUIRE( array[3] == 3 );
        REQUIRE( array[4] == 0 );  // 0+1
        REQUIRE( array[5] == 5 );  // 2+3
        REQUIRE( array[6] == -1 ); // task 6 removed
    }
}
