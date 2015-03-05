#ifndef PEERCONNECTION_H
#define PEERCONNECTION_H

#include <QObject>
#include <QTcpSocket>

#include "macro.h"
#include "peerinfo.h"

class TorrentClient;
class Torrent;

class PeerConnection : public QTcpSocket
{
public:
    PeerConnection(const PeerInfo &peerInfo, Torrent *parentTorrent, TorrentClient *torrentClient);
    void clear();
    ~PeerConnection();

    //// Read only
    PROP_GET( PeerInfo,         PeerInfo )
    PROP_GET( Torrent *,        Torrent )
    PROP_GET( TorrentClient *,  TorrentClient )

    PROP_GET( uint,             LastVisited )
    PROP_GET( uint,             ConnectStart )
    PROP_GET( uint,             ConnectTime )

    //// Read-Write-Notification
    PROP_SIMPLE( bool,         IsPeerChoking )       // Этот пир блокирует меня
    PROP_SIMPLE( bool,         IsPeerInterested )    // Этот пир заинтересован во мне
    PROP_SIMPLE( bool,         IsAmChoking )         // Я блокирую этого пира
    PROP_SIMPLE( bool,         IsAmInterested )      // Я заинтересован в пире
    PROP_SIMPLE( bool,         IsSeed )

    PROP_SIMPLE( QBitArray,    Pieces )
    PROP_SIMPLE( int,          NumCompletedPieces )

public slots:

signals:
    void IsPeerChokingChanged( bool newVal );
    void IsPeerInterestedChanged( bool newVal );
    void IsAmChokingChanged( bool newVal );
    void IsAmInterestedChanged( bool newVal );
    void IsSeedChanged( bool newVal );

    void PiecesChanged( const QBitArray &newVal );
    void NumCompletedPiecesChanged( int newVal );

public slots:
    // handshake: <pstrlen><pstr><reserved><info_hash><peer_id>
    void makeHandshake();

};

#endif // PEERCONNECTION_H
