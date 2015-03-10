#ifndef PEERCONNECTION_H
#define PEERCONNECTION_H

#include <QObject>
#include <QTcpSocket>

#include "macro.h"
#include "peerinfo.h"

class TorrentClient;
class Torrent;
class QTimer;

class PeerConnection : public QTcpSocket
{
    Q_OBJECT
public:
    PeerConnection(Torrent *parentTorrent, TorrentClient *torrentClient);
    void clear();
    ~PeerConnection();

    //// Read only
    PROP_GET( PeerInfo,         PeerInfo )
    PROP_GET( Torrent *,        Torrent )
    PROP_GET( TorrentClient *,  TorrentClient )

    PROP_GET( uint,             LastVisited )
    PROP_GET( uint,             ConnectStart )
    PROP_GET( uint,             ConnectTime )

    PROP_GET( QByteArray,       InfoHash )

    //// Read-Write-Notification
    PROP_SIMPLE( bool,         IsPeerChoking )       // Этот пир блокирует меня
    PROP_SIMPLE( bool,         IsPeerInterested )    // Этот пир заинтересован во мне
    PROP_SIMPLE( bool,         IsAmChoking )         // Я блокирую этого пира
    PROP_SIMPLE( bool,         IsAmInterested )      // Я заинтересован в пире
    PROP_SIMPLE( bool,         IsSeed )

    PROP_SIMPLE( QBitArray,    Pieces )
    PROP_SIMPLE( int,          NumCompletedPieces )

    PROP_GET_SET(quint32,      HandshakeTimeout  )

public slots:
    void connectToPeer( const PeerInfo &peerInfo );
    void peerConnectMe( qintptr socketDescriptor );

private:
    QTimer     *m_timerHandhsake;

signals:
    void IsPeerChokingChanged( bool newVal );
    void IsPeerInterestedChanged( bool newVal );
    void IsAmChokingChanged( bool newVal );
    void IsAmInterestedChanged( bool newVal );
    void IsSeedChanged( bool newVal );

    void PiecesChanged( const QBitArray &newVal );
    void NumCompletedPiecesChanged( int newVal );

    void handshakeIsDone();
    void handshakeFailed();

    void incomingHandhsakeData( const QByteArray &infoHash, const QByteArray &peerId );

//private
public slots:
    // handshake: <pstrlen><pstr><reserved><info_hash><peer_id>
    void prepareHandshakeMsg();
    void sendChoke();
    void sendUnchoke();
    void sendInterested();
    void sendNotInterested();
    void sendHave( quint32 pieceIndex );

    void responseHandler();

    bool handshakeHandler();

private:
    QByteArray      m_outgoingBuffer;
    QByteArray      m_incomingBuffer;
    bool            m_handshakeHandled;         // Рукопожатие было обработано мной
    bool            m_handshakeSended;          // Рукопожатие было послано мной

private:
    enum PacketType {
        PacketInvalid = -2,
        PacketKeepAlive = -1,
        PacketChoke = 0,
        PacketUnchoke = 1,
        PacketInterested = 2,
        PacketNotInterested = 3,
        PacketHave = 4,
        PacketBitField = 5,
        PacketRequest = 6,
        PacketPiece = 7,
        PacketCancel = 8,
        PacketPort = 9
    };

    PacketType incomingPacetType();
};

#endif // PEERCONNECTION_H
