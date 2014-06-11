#include "Socket.h"
#include "EventManager.h"
#include "Log.h"

using namespace std;

Socket::Socket ( Owner& owner, NL::Socket *socket )
    : owner ( owner ), socket ( socket ), address ( socket ), protocol ( socket->protocol() == NL::TCP ? TCP : UDP )
{
    EventManager::get().addSocket ( this );
}

Socket::Socket ( Owner& owner, unsigned port, Protocol protocol )
    : owner ( owner ), socket ( 0 ), address ( "", port ), protocol ( protocol )
{
    EventManager::get().addSocket ( this );
}

Socket::Socket ( Owner& owner, const string& address, unsigned port, Protocol protocol )
    : owner ( owner ), socket ( 0 ), address ( address, port ), protocol ( protocol )
{
    EventManager::get().addSocket ( this );
}

Socket::~Socket()
{
    EventManager::get().removeSocket ( this );
}

void Socket::disconnect()
{
    EventManager::get().removeSocket ( this );
}

Socket *Socket::listen ( Owner& owner, unsigned port, Protocol protocol )
{
    return new Socket ( owner, port, protocol );
}

Socket *Socket::connect ( Owner& owner, const string& address, unsigned port, Protocol protocol )
{
    return new Socket ( owner, address, port, protocol );
}

Socket *Socket::accept ( Owner& owner )
{
    if ( socket->type() != NL::SERVER )
        return 0;

    return new Socket ( owner, socket->accept() );
}

bool Socket::isConnected() const
{
    return ( socket != 0 );
}

IpAddrPort Socket::getRemoteAddress() const
{
    if ( !socket )
        return IpAddrPort();

    return IpAddrPort ( socket );
}

void Socket::send ( const Serializable& msg, const IpAddrPort& address )
{
    string bytes = Serializable::encode ( msg );
    LOG ( "Encoded '%s' to [ %u bytes ]", msg.type().c_str(), bytes.size() );
    send ( &bytes[0], bytes.size(), address );
}

void Socket::send ( char *bytes, size_t len, const IpAddrPort& address )
{
    if ( socket )
    {
        if ( address.empty() )
        {
            LOG ( "socket->send ( [ %u bytes ] ); address='%s'", len, IpAddrPort ( socket ).c_str() );
            socket->send ( bytes, len );
        }
        else
        {
            LOG ( "socket->sendTo ( [ %u bytes ], '%s' )", len, address.c_str() );
            socket->sendTo ( bytes, len, address.addr, address.port );
        }
    }
    else
    {
        LOG ( "Unconnected socket!" );
    }
}
