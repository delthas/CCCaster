#include "Event.h"
#include "Log.h"

#include <cassert>

using namespace std;

EventManager::EventManager()
    : running ( false )
    , reaperThread ( zombieThreads )
{
    socketGroup.setCmdOnAccept ( &socketAcceptCmd );
    socketGroup.setCmdOnDisconnect ( &socketDisconnectCmd );
    socketGroup.setCmdOnRead ( &socketReadCmd );
}

EventManager::~EventManager()
{
}

void EventManager::start()
{
    {
        LOCK ( mutex );
        running = true;
    }

    LOG ( "Starting timer thread" );

    timerThread.start();

    LOG ( "Starting listen loop" );

    socketListenLoop();

    LOG ( "Finished listen loop" );

    timerThread.join();

    LOG ( "Joined timer thread" );

    reaperThread.join();

    LOG ( "Joined reaper thread" );
}

void EventManager::stop()
{
    LOG ( "Stopping everything" );

    LOCK ( mutex );

    running = false;
    socketsCond.signal();
    timersCond.signal();
}

void EventManager::release()
{
    stop();

    LOG ( "Releasing everything" );

    timerThread.release();
    reaperThread.release();
}

EventManager& EventManager::get()
{
    static EventManager em;
    return em;
}