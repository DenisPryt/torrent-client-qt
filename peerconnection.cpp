#include "peerconnection.h"
#include "torrentclient.h"
#include "torrent.h"
#include "torrentfileinfo.h"

#include <QByteArray>
#include <QDataStream>
#include <QTimerEvent>
#include <QTimer>

PeerConnection::PeerConnection(Torrent *parentTorrent, TorrentClient *torrentClient )
    : QTcpSocket( parentTorrent )
    , m_TorrentClient( torrentClient )
    , m_Torrent( parentTorrent )
    , m_HandshakeTimeout( 5000 )
{
    Q_ASSERT( parentTorrent != nullptr );
    Q_ASSERT( torrentClient != nullptr );

    m_timerHandhsake = new QTimer( this );
    m_timerHandhsake->setInterval( m_HandshakeTimeout );
    m_timerHandhsake->setSingleShot( true );
    connect( this, &PeerConnection::handshakeIsDone, m_timerHandhsake, &QTimer::stop );
    connect( m_timerHandhsake, &QTimer::timeout, [this](){
        qWarning() << "CONNECTION TIMEOUT";
        disconnectFromHost();
        emit handshakeFailed();
    } );

    clear();

    m_InfoHash = parentTorrent->GetTorrentFileInfo()->GetInfoHashSHA1();

    connect( this, &PeerConnection::stateChanged, [this]( QAbstractSocket::SocketState state ){
        qDebug() << state;
    } );
    connect( this, &QAbstractSocket::readyRead, this, &PeerConnection::responseHandler );
    connect( this, &PeerConnection::connected, [this](){
        qDebug() << write( m_outgoingBuffer );
        m_handshakeSended = true;
    } );
}

void PeerConnection::clear()
{
    m_InfoHash.clear();
    m_PeerInfo.clear();
    m_Pieces.clear();
    m_NumCompletedPieces = 0;

    m_IsAmChoking = true;
    m_IsAmInterested = false;
    m_IsPeerChoking = true;
    m_IsPeerInterested = false;

    m_IsSeed = false;
    m_LastVisited = 0;
    m_ConnectStart = 0;
    m_ConnectTime = 0;
    m_Pieces.clear();

    m_handshakeHandled = false;
    m_handshakeSended = false;
}

PeerConnection::~PeerConnection()
{

}

void PeerConnection::connectToPeer(const PeerInfo &peerInfo)
{
    qDebug() << Q_FUNC_INFO << peerInfo.GetAddress() << peerInfo.GetPort();
    m_PeerInfo = peerInfo;
    connectToHost( m_PeerInfo.GetAddress(), m_PeerInfo.GetPort(), QIODevice::ReadWrite );
    m_timerHandhsake->start();

    prepareHandshakeMsg();
}

void PeerConnection::peerConnectMe(qintptr socketDescriptor)
{
    qDebug() << Q_FUNC_INFO << setSocketDescriptor( socketDescriptor );
}

void PeerConnection::prepareHandshakeMsg()
{
    if ( m_handshakeHandled ){

    }
    const QByteArray HANDSHAKE_PSTR = "BitTorrent protocol";
    const int HANDSHAKE_PSTRLEN = HANDSHAKE_PSTR.size();

    //const auto &infoHash  = m_Torrent->GetTorrentFileInfo()->GetInfoHashSHA1();
    const auto &peerId    = m_TorrentClient->GetClientId();

    m_outgoingBuffer.clear();
    QDataStream msgStream( &m_outgoingBuffer, QIODevice::WriteOnly );

    msgStream << quint8( HANDSHAKE_PSTRLEN );
    m_outgoingBuffer.append( HANDSHAKE_PSTR );

    QByteArray msg2;
    QDataStream msgStream2( &msg2, QIODevice::WriteOnly );
    msgStream2 << qint64(0);

    m_outgoingBuffer.append( msg2 );
    m_outgoingBuffer.append( m_InfoHash );
    m_outgoingBuffer.append( peerId );

}

void PeerConnection::sendChoke()
{
    const char message[] = {0, 0, 0, 1, 0};
    write(message, sizeof(message));

    //pendingBlocks.clear();
    //pendingBlockSizes = 0;

    SetIsAmChoking( true );
}

void PeerConnection::sendUnchoke()
{
    const char message[] = {0, 0, 0, 1, 1};
    write(message, sizeof(message));

    //if (pendingRequestTimer)
    //    killTimer(pendingRequestTimer);

    SetIsAmChoking( false );
}

void PeerConnection::sendInterested()
{
    const char message[] = {0, 0, 0, 1, 2};
    write(message, sizeof(message));

    // After telling the peer that we're interested, we expect to get
    // unchoked within a certain timeframe; otherwise we'll drop the
    // connection.
    //if (pendingRequestTimer)
    //    killTimer(pendingRequestTimer);

    //pendingRequestTimer = startTimer(PendingRequestTimeout);
    SetIsAmInterested( true );
}

void PeerConnection::sendNotInterested()
{
    const char message[] = {0, 0, 0, 1, 3};
    write(message, sizeof(message));

    SetIsAmInterested( false );
}

void PeerConnection::sendHave(quint32 pieceIndex)
{
    //if (!sentHandShake)
    //    sendHandShake();

    const char messageMask[] = {0, 0, 0, 5, 4};
    QByteArray message( messageMask );
    QDataStream( &message, QIODevice::WriteOnly ) << quint32( pieceIndex );
    write( message );
}

void PeerConnection::responseHandler()
{
    qDebug() << Q_FUNC_INFO;
    if ( !m_handshakeSended ){          // Инициатор -- пир

        prepareHandshakeMsg();
    }
    else if ( !m_handshakeHandled ){    // Инициатор -- я
        if ( handshakeHandler() )
            emit handshakeIsDone();
        else
            emit handshakeFailed();
    }
}

bool PeerConnection::handshakeHandler()
{
    qDebug() << "processHandshake()";

    quint8 pstrLen = -1;
    auto msg = readAll();
    QDataStream stream( &msg, QIODevice::ReadOnly );
    stream >> quint8( pstrLen );
    QByteArray pstr          = msg.mid(1, pstrLen);
    QByteArray reservedZeros = msg.mid(pstrLen + 1, 8);
    QByteArray infoHash      = msg.mid( pstrLen + 9, 20 );
    QByteArray peerId        = msg.mid( pstrLen + 29, 20 );

    m_handshakeHandled = true;
    if ( pstrLen == 19 && pstr == "BitTorrent protocol" ){
        if ( infoHash == m_Torrent->GetTorrentFileInfo()->GetInfoHashSHA1() ){
            qDebug() << Q_FUNC_INFO << " Handshake is complited ";
            return true;
        }
        else{
            qWarning() << Q_FUNC_INFO << " INCORRECT INFO_HASH ";
            return false;
        }
    }
    qWarning() << Q_FUNC_INFO << " INCORRECT PROTOCOL";
    return false;
}

PeerConnection::PacketType PeerConnection::incomingPacetType()
{
    if ( m_incomingBuffer.size() == 4 && m_incomingBuffer.startsWith("\0\0\0\0") ){
        return PacketKeepAlive;
    }
    if ( m_incomingBuffer.size() <= 4 ){
        return PacketInvalid;
    }

    quint8 id = (quint8)PacketInvalid;

    QDataStream in( &m_incomingBuffer, QIODevice::ReadOnly );
    in.device()->seek(5);
    in >> quint8( id );

    return (PacketType)id;
}

