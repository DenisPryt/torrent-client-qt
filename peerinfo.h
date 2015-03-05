#ifndef PEERINFO_H
#define PEERINFO_H

#include "macro.h"
#include <QObject>
#include <QHostAddress>
#include <QBitArray>
#include <QHash>

class PeerInfo
{
public:
    PeerInfo(){ clear(); }
    PeerInfo( const QHostAddress &address, quint16 port )
        : m_Address(address), m_Port(port)
        {}
    ~PeerInfo(){}
    void clear();

    //// Getters only properties
    PROP_GET( QHostAddress, Address )
    PROP_GET( quint16,      Port )
    PROP_GET( QString,      Id )

public:
    friend inline bool operator==(const PeerInfo &l, const PeerInfo &r){
        return  l.m_Port        == r.m_Port &&
                l.m_Address     == r.m_Address &&
                l.m_Id          == r.m_Id;
    }
};

inline uint qHash( const PeerInfo &peerInfo ){
    return qHash( peerInfo.GetAddress() ) ^ qHash( peerInfo.GetPort() );
}

#endif // PEERINFO_H
