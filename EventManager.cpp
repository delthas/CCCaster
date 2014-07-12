#include "EventManager.h"
#include "TimerManager.h"
#include "SocketManager.h"
#include "JoystickManager.h"
#include "Log.h"

#include <winsock2.h>
#include <windows.h>
#include <mmsystem.h>

using namespace std;

void EventManager::checkEvents()
{
    TimerManager::get().update();
    TimerManager::get().check();
    SocketManager::get().check();
    JoystickManager::get().check();
}

void EventManager::eventLoop()
{
    if ( TimerManager::get().isHiRes() )
    {
        while ( running )
        {
            Sleep ( 1 );
            checkEvents();
        }
    }
    else
    {
        timeBeginPeriod ( 1 );

        while ( running )
        {
            Sleep ( 1 );
            checkEvents();
        }

        timeEndPeriod ( 1 );
    }
}

EventManager::EventManager() : running ( false )
{
}

bool EventManager::poll()
{
    if ( !running )
        return false;

    checkEvents();

    if ( running )
        return true;

    LOG ( "Finished polling" );

    TimerManager::get().clear();
    SocketManager::get().clear();
    JoystickManager::get().clear();

    LOG ( "Joining reaper thread" );

    reaperThread.join();

    LOG ( "Joined reaper thread" );

    return false;
}

void EventManager::start()
{
    running = true;

    LOG ( "Starting event loop" );

    eventLoop();

    LOG ( "Finished event loop" );

    TimerManager::get().clear();
    SocketManager::get().clear();
    JoystickManager::get().clear();

    LOG ( "Joining reaper thread" );

    reaperThread.join();

    LOG ( "Joined reaper thread" );
}

void EventManager::stop()
{
    LOG ( "Stopping everything" );

    running = false;
}

void EventManager::release()
{
    stop();

    LOG ( "Releasing everything" );

    reaperThread.release();
}

void EventManager::initialize()
{
    TimerManager::get().initialize();
    SocketManager::get().initialize();
    JoystickManager::get().initialize();
}

void EventManager::initializePolling()
{
    initialize();

    running = true;
}

void EventManager::deinitialize()
{
    TimerManager::get().deinitialize();
    SocketManager::get().deinitialize();
    JoystickManager::get().deinitialize();
}

EventManager& EventManager::get()
{
    static EventManager em;
    return em;
}

void EventManager::ReaperThread::run()
{
    for ( ;; )
    {
        shared_ptr<Thread> thread = zombieThreads.pop();

        LOG ( "Joining %08x", thread.get() );

        if ( thread )
            thread->join();
        else
            return;

        LOG ( "Joined %08x", thread.get() );
    }
}

void EventManager::ReaperThread::join()
{
    zombieThreads.push ( shared_ptr<Thread>() );
    Thread::join();
    zombieThreads.clear();
}

void EventManager::addThread ( const shared_ptr<Thread>& thread )
{
    reaperThread.start();
    reaperThread.zombieThreads.push ( thread );
}