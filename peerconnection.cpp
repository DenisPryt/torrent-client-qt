#include "peerconnection.h"
#include "torrentclient.h"
#include "torrent.h"
#include "torrentfileinfo.h"

#include <QByteArray>
#include <QDataStream>

PeerConnection::PeerConnection(const PeerInfo &peerInfo, Torrent *parentTorrent, TorrentClient *torrentClient )
    : QTcpSocket( parentTorrent )
    , m_TorrentClient( torrentClient )
    , m_Torrent( parentTorrent )
{
    Q_ASSERT( parentTorrent != nullptr );
    Q_ASSERT( torrentClient != nullptr );

    clear();
    m_PeerInfo = peerInfo;
}

void PeerConnection::clear()
{
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
}

PeerConnection::~PeerConnection()
{

}

void PeerConnection::makeHandshake()
{
    connectToHost( m_PeerInfo.GetAddress(), m_PeerInfo.GetPort(), QIODevice::ReadWrite );
    if ( !waitForConnected( 5000 ) ){
        qWarning() << Q_FUNC_INFO << "CANT CONNECT TO PEER";
        return;
    }

    const QByteArray HANDSHAKE_PSTR = "BitTorrent protocol";
    const int HANDSHAKE_PSTRLEN = HANDSHAKE_PSTR.size();

    auto &infoHash  = m_Torrent->GetTorrentFileInfo()->GetInfoHashSHA1();
    auto &peerId    = m_TorrentClient->GetClientId();

    QByteArray msg;
    QDataStream msgStream( &msg, QIODevice::WriteOnly );

    msgStream << quint8( HANDSHAKE_PSTRLEN );
    msg.append( HANDSHAKE_PSTR );

    QByteArray msg2;
    QDataStream msgStream2( &msg2, QIODevice::WriteOnly );
    msgStream2 << qint64(0);

    msg.append( msg2 );
    msg.append( infoHash );
    msg.append( peerId );
    qDebug() << write( msg );
}

