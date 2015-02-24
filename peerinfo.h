#ifndef PEERINFO_H
#define PEERINFO_H

#include "macro.h"
#include <QObject>
#include <QHostAddress>
#include <QBitArray>

class PeerInfo : public QObject
{
    Q_OBJECT
public:
    explicit PeerInfo(QObject *parent = 0) : QObject(parent){ clear(); }
    ~PeerInfo(){}

    void clear();

    //// Getters only properties
    PROP_GET( QHostAddress, Address )
    PROP_GET( quint16,      Port )
    PROP_GET( QString,      Id)
    PROP_GET( bool,         Intesting )
    PROP_GET( bool,         Seed)
    PROP_GET( uint,         LastVisited )
    PROP_GET( uint,         ConnectStart )
    PROP_GET( uint,         ConnectTime )
    PROP_GET( QBitArray,    Pieces)
    PROP_GET( int,          NumCompletedPieces )

public:
    inline bool operator==(const PeerInfo &other)
    {
        return m_Port == other.m_Port && m_Address == other.m_Address && m_Id == other.m_Id;
    }

signals:

public slots:
};

#endif // PEERINFO_H
