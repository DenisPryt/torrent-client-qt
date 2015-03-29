#include "peerconnection.h"
#include "torrentclient.h"
#include "torrent.h"
#include "torrentfileinfo.h"

#include <QByteArray>
#include <QDataStream>
#include <QTimerEvent>
#include <QTimer>

static const QByteArray HANDSHAKE_PSTR    = "BitTorrent protocol";
static const int        HANDSHAKE_PSTRLEN = HANDSHAKE_PSTR.size();
static const quint32    STANDART_HANDSHAKE_TIMEOUT = 1000 * 5;      // 5 sec
static const quint32    STANDART_KEEPALIVE_TIMEOUT = 1000 * 60 * 3; // 3 min
static const uint       STANDART_SPEEDTEST_TIMEOUT = 1000 * 3;      // 3 sec
static const uint       STANDART_BYTES_SPEED_SIZE  = 8;
static const uint       STANDART_PIECE_SIZE        = 16384;         // 2^14

PeerConnection::PeerConnection( TorrentClient *torrentClient, QObject *parent )
    : QTcpSocket( parent )
    , m_TorrentClient( torrentClient )
    , m_timerHandhsake( nullptr )
    , m_timerKeepAlive( nullptr )
    , m_timerMyKeepAlive( nullptr )
    , m_timerSpeedTest( nullptr )
{
    Q_ASSERT( torrentClient != nullptr );

    clear();

    initTimer( m_timerHandhsake, STANDART_HANDSHAKE_TIMEOUT, [this](){
        m_timerHandhsake->stop();
        //qWarning() << "HANDSHAKE TIMEOUT";
        disconnectFromHost();
        emit handshakeFailed();
    } );

    initTimer( m_timerKeepAlive, STANDART_KEEPALIVE_TIMEOUT, [this](){
        m_timerKeepAlive->stop();
        qWarning() << "KEEP ALIVE TIMEOUT";
        disconnectFromHost();
    } );

    initTimer( m_timerMyKeepAlive, STANDART_KEEPALIVE_TIMEOUT, [this](){
        qDebug() << "My keep alive timeout";
        sendKeepAlive();
    } );

    initTimer( m_timerSpeedTest, STANDART_SPEEDTEST_TIMEOUT, [this](){
        qDebug() << "Speed test timeout";
        timerSpeedTestHandler();
    } );

    connect( this, &QAbstractSocket::readyRead, this, &PeerConnection::responseHandler );

    connect( this, &PeerConnection::handshakeIsDone, m_timerHandhsake, &QTimer::stop );
/*
    connect( this, &PeerConnection::stateChanged, [this]( QAbstractSocket::SocketState state ){
        qDebug() << state;
    } );
*/
    connect( this, &PeerConnection::connected, [this](){
        qDebug() << write( m_outgoingBuffer );
        m_handshakeSended = true;
    } );
}

void PeerConnection::clear()
{
    m_InfoHash.clear();
    m_PeerInfo.clear();
    m_NumCompletedPieces = 0;

    m_IsAmChoking = true;
    m_IsAmInterested = false;
    m_IsPeerChoking = true;
    m_IsPeerInterested = false;

    m_IsSeed = false;
    m_LastVisited = 0;
    m_ConnectStart = 0;
    m_ConnectTime = 0;
    m_HandshakeTimeout = STANDART_HANDSHAKE_TIMEOUT;
    m_KeepAliveTimeout = STANDART_KEEPALIVE_TIMEOUT;

    m_handshakeHandled = false;
    m_handshakeSended = false;

    clearTimer( m_timerHandhsake,   STANDART_HANDSHAKE_TIMEOUT );
    clearTimer( m_timerKeepAlive,   STANDART_KEEPALIVE_TIMEOUT );
    clearTimer( m_timerMyKeepAlive, STANDART_KEEPALIVE_TIMEOUT );
    clearTimer( m_timerSpeedTest,   STANDART_SPEEDTEST_TIMEOUT );

    m_peerRequestedBlocks.clear();
    m_amRequestedBlocks.clear();
    m_downloadedBlocks.clear();
    m_peerPieces.clear();
    m_downloadedPieces.clear();

    m_bytesSpeedSize = STANDART_BYTES_SPEED_SIZE;
    m_bytesDownloadedList.clear();
    m_bytesUploadedList.clear();
    m_bytesDownloaded = 0;
    m_bytesUploaded = 0;

    m_PieceSize = STANDART_PIECE_SIZE;
}

void PeerConnection::initTimer(QTimer *&timer, quint32 timeout, std::function< void(void) > timeoutHandler )
{
    if ( timer != nullptr ){
        timer->stop();
        delete timer;
    }

    timer = new QTimer( this );
    timer->setInterval( timeout );
    timer->setSingleShot( false );
    connect( timer, &QTimer::timeout, timeoutHandler );
}

void PeerConnection::clearTimer(QTimer *timer, quint32 timeout)
{
    if ( timer == nullptr )
        return;

    timer->stop();
    timer->setInterval( timeout );
}

void PeerConnection::SetTimeout(quint32 &timeoutVar, QTimer *timerVar, quint32 newTimeout)
{
    timeoutVar = newTimeout;
    bool wasIsActive = timerVar->isActive();
    timerVar->stop();
    timerVar->setInterval( timeoutVar );
    if ( wasIsActive )
        timerVar->start();
}

void PeerConnection::timerSpeedTestHandler()
{
    timerSpeedTestHandler( m_bytesDownloaded, m_bytesDownloadedList );
    timerSpeedTestHandler( m_bytesUploaded, m_bytesUploadedList );
}

void PeerConnection::timerSpeedTestHandler(quint64 &bytes, QLinkedList<quint64> &list)
{
    list.append( bytes );
    if ( list.size() > m_bytesSpeedSize ){
        list.pop_front();
    }
}

quint64 PeerConnection::CalculateSpeed(const QLinkedList<quint64> &list) const
{
    quint64 sum = 0;
    for ( auto bytes : list )
        sum += bytes;

    return sum / (list.size() * STANDART_SPEEDTEST_TIMEOUT);
}

PeerConnection::~PeerConnection()
{
    m_timerHandhsake->stop();
    m_timerKeepAlive->stop();
    m_timerSpeedTest->stop();
}

void PeerConnection::DownloadBlock(quint32 index, quint32 size)
{
    for (quint32 currentOffset = 0; currentOffset < size; currentOffset += m_PieceSize ){
        sendRequest( index, currentOffset, m_PieceSize );
    }
}

void PeerConnection::connectToPeer(const PeerInfo &peerInfo, const QByteArray &infoHash, quint32 pieceCount)
{
    qDebug() << Q_FUNC_INFO << peerInfo.GetAddress() << peerInfo.GetPort();
    m_peerPieces.resize( pieceCount );
    m_PeerInfo = peerInfo;
    m_InfoHash = infoHash;
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
    //const auto &infoHash  = m_Torrent->GetTorrentFileInfo()->GetInfoHashSHA1();
    const auto &myId    = m_TorrentClient->GetClientId();

    m_outgoingBuffer.clear();
    QDataStream msgStream( &m_outgoingBuffer, QIODevice::WriteOnly );

    msgStream << quint8( HANDSHAKE_PSTRLEN );
    m_outgoingBuffer.append( HANDSHAKE_PSTR );

    QByteArray msg2;
    QDataStream msgStream2( &msg2, QIODevice::WriteOnly );
    msgStream2 << qint64(0);

    m_outgoingBuffer.append( msg2 );
    m_outgoingBuffer.append( m_InfoHash );
    m_outgoingBuffer.append( myId );
}

void PeerConnection::sendChoke()
{
    const char message[] = {0, 0, 0, 1, 0};
    write(message, sizeof(message));

    m_amRequestedBlocks.clear();

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
    qDebug() << Q_FUNC_INFO;
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
    const char messageMask[] = {0, 0, 0, 5, 4};
    QByteArray message( messageMask );
    QDataStream( &message, QIODevice::WriteOnly ) << quint32( pieceIndex );
    write( message );
}

void PeerConnection::sendRequest(quint32 index, quint32 begin, quint32 length)
{
    qDebug() << Q_FUNC_INFO;
    m_amRequestedBlocks.insert( TorrentBlockDescriptor(index, begin, length) );
    QByteArray msg;
    QDataStream in( &msg, QIODevice::WriteOnly );
    in << qint32( 13 ) << qint8( PacketRequest ) << index << begin << length;
    qDebug() << write( msg );
}

void PeerConnection::sendKeepAlive()
{
    const char message[] = {0, 0, 0, 0};
    write( message, sizeof(message) );
}

void PeerConnection::bitFildHandler()
{
    qDebug() << Q_FUNC_INFO;
    qlonglong len = incomingLen();
    if ( m_incomingBuffer.size() - 4 > len ){
        qWarning() << Q_FUNC_INFO << "m_incomingBuffer.size() > len";
        emit invalidPeer();
    }

    if ( m_incomingBuffer.size() - 4 < len ){
        qDebug() << "m_incomingBuffer.size() - 4 < len";
        return;
    }

    bool isInvalidPeer = false;
    for ( qlonglong i = 5; i < len; ++i ){
        for ( int bit = 0; bit < 8; ++bit ){
            if ( m_incomingBuffer.at(i) & (1 << (7 - bit)) ){
                int bitIndex = int( ((i - 1) * 8) + bit );
                if ( bitIndex < 0 || bitIndex >= m_peerPieces.size() ){
                    emit invalidPeer();
                    isInvalidPeer = true;
                    break;
                }
                m_peerPieces.setBit( bitIndex );
            }

            if ( isInvalidPeer ){
                break;
            }
        }
    }
}

void PeerConnection::RequestHandler()
{
    qDebug() << Q_FUNC_INFO;
    if ( m_incomingBuffer.size() < 17 ){
        qWarning() << Q_FUNC_INFO << "m_incomingBuffer.size() < 17";
        return;
    }

    if ( incomingLen() != 13 ){
        qWarning() << Q_FUNC_INFO << "incomingLen() != 13";
        emit invalidPeer();
        return;
    }

    QDataStream out( &m_incomingBuffer, QIODevice::ReadOnly );
    out.device()->seek( 5 );
    quint32 index = -1, begin = -1, length = -1;
    out >> quint32( index ) >> quint32( begin ) >> quint32( length );
    m_peerRequestedBlocks.insert( TorrentBlockDescriptor(index, begin, length) );
    emit PeerRequest( index, begin, length );
}

void PeerConnection::PieceHandler()
{
    qDebug() << Q_FUNC_INFO;

    if ( m_incomingBuffer.size() < 13 ){
        qDebug() << Q_FUNC_INFO << "m_incomingBuffer.size() < 13";
        return;
    }

    QDataStream out( &m_incomingBuffer, QIODevice::ReadOnly );
    out.device()->seek( 5 );
    quint32 index = 0, begin = 0;
    out >> index >> begin;

    m_bytesDownloaded += m_incomingBuffer.size() - 13;
    // 13 = 4 + 1 + 4 + 4 : <len(4)><id(1)><index(4)><begin(4)><block(X)>
    TorrentBlockDescriptor blockDesc(index, begin, m_incomingBuffer.size() - 13);
    m_amRequestedBlocks.remove( blockDesc );
    m_downloadedBlocks.insert( blockDesc, m_incomingBuffer.mid(13) );

    qDebug() << m_incomingBuffer.size() - 13;

    emit BlockDownloaded( index, begin, m_incomingBuffer.mid(13) );
/*

    // Kill the pending block timer.
    if (pendingRequestTimer) {
        killTimer(pendingRequestTimer);
        pendingRequestTimer = 0;
    }*/
}

void PeerConnection::CancelHandler()
{
    qDebug() << Q_FUNC_INFO;
    QDataStream out( &m_incomingBuffer, QIODevice::ReadOnly );
    out.device()->seek( 5 );
    quint32 index = 0, begin = 0, length = 0;
    out >> index >> begin >> length;

    m_peerRequestedBlocks.remove( TorrentBlockDescriptor(index, begin, length) );
}

void PeerConnection::responseHandler()
{
    qDebug() << Q_FUNC_INFO;
    if ( !m_handshakeSended ){          // Инициатор -- пир
        handshakeHandler();
        prepareHandshakeMsg();
        return;
    }
    else if ( !m_handshakeHandled ){    // Инициатор -- я
        if ( handshakeHandler() )
            emit handshakeIsDone();
        else
            emit handshakeFailed();

        return;
    }
    m_incomingBuffer.clear();
    do{
        m_incomingBuffer += readAll();
        switch ( incomingPacetType() ) {
        case PacketKeepAlive:
            //m_timerKeepAlive->start();
            break;
        case PacketChoke:
            qDebug() << "PacketChoke";
            SetIsPeerChoking( true );
            break;
        case PacketUnchoke:
            qDebug() << "PacketUnchoke";
            SetIsPeerChoking( false );
            break;
        case PacketInterested:
            qDebug() << "PacketInterested";
            SetIsPeerInterested( true );
            break;
        case PacketNotInterested:
            qDebug() << "PacketNotInterested";
            SetIsPeerInterested( false );
            break;
        case PacketHave:
            qDebug() << "PacketHave";
            m_peerPieces.setBit( quint8(m_incomingBuffer[5]) );
            break;
        case PacketBitField:
            qDebug() << "PacketBitField";
            bitFildHandler();
            break;
        case PacketRequest:
            qDebug() << "PacketRequest";
            RequestHandler();
            break;
        case PacketPiece: {
            qDebug() << "PacketPiece";
            PieceHandler();
            break;
        }
        case PacketCancel: {
            qDebug() << "PacketCancel";
            CancelHandler();
            break;
        }
        default:
            qWarning() << Q_FUNC_INFO << "UNKNOWN PACKET TYPE" << "PACKET = " << m_incomingBuffer;
            break;
        }
    }while( bytesAvailable() > 0 );
}

bool PeerConnection::handshakeHandler()
{
    qDebug() << "processHandshake()";

    quint8 pstrLen = -1;
    m_incomingBuffer = readAll();
    QDataStream stream( &m_incomingBuffer, QIODevice::ReadOnly );
    stream >> quint8( pstrLen );
    QByteArray pstr          = m_incomingBuffer.mid( 1, pstrLen );
    QByteArray reservedZeros = m_incomingBuffer.mid( pstrLen + 1, 8 );
    QByteArray infoHash      = m_incomingBuffer.mid( pstrLen + 9, 20 );
    QByteArray peerId        = m_incomingBuffer.mid( pstrLen + 29, 20 );

    m_handshakeHandled = true;
    if ( pstrLen != HANDSHAKE_PSTRLEN || pstr != HANDSHAKE_PSTR ){
        qWarning() << Q_FUNC_INFO << " INCORRECT PROTOCOL";
        return false;
    }

    if( m_InfoHash.isEmpty() ){     // Инициатор рукопожатия -- пир
        m_InfoHash = infoHash;
    }

    if ( m_PeerInfo.GetId().isEmpty() ){
        m_PeerInfo.SetId( peerId );
    }
    else if ( m_PeerInfo.GetId() != peerId ){
        qWarning() << Q_FUNC_INFO << "INCORRECT PEER ID";
    }

    if ( infoHash == m_InfoHash ){
        qDebug() << Q_FUNC_INFO << " Handshake is complited ";
        return true;
    }
    else{
        qWarning() << Q_FUNC_INFO << " INCORRECT INFO_HASH ";
        return false;
    }

}

PeerConnection::PacketType PeerConnection::incomingPacetType()
{
    if ( m_incomingBuffer.size() == 4 && m_incomingBuffer.startsWith("\0\0\0\0") ){
        return PacketKeepAlive;
    }
    if ( m_incomingBuffer.size() <= 4 ){
        return PacketInvalid;
    }

    qint8 id = (qint8)PacketInvalid;

    QDataStream in( &m_incomingBuffer, QIODevice::ReadOnly );
    in.device()->seek(4);
    in >> qint8( id );
    if ( id < 0 || id >= PacketInvalid ){
        qWarning() << Q_FUNC_INFO << "ID IS INVALID";
        return PacketInvalid;
    }

    return (PacketType)id;
}

quint32 PeerConnection::incomingLen()
{
    if ( m_incomingBuffer.size() < 4 )
        return 0;

    return (quint32(m_incomingBuffer[0]) << 24)
        | (quint32(m_incomingBuffer[1]) << 16)
        | (quint32(m_incomingBuffer[2]) << 8)
        | (quint32(m_incomingBuffer[3]));
}
