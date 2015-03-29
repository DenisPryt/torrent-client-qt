#include "downloader.h"

#include "peerconnection.h"
#include "torrentclient.h"

Downloader::Downloader(TorrentClient *torrentClient, const TorrentFileInfo &info, QObject *parent)
    : QObject(parent)
    , m_torrentClient( torrentClient )
    , m_torrentFileInfo( info )
{
    m_downloadedPieceIndex = 0;
    m_downloadedBytes = 0;
    m_fileManager = new FileManager( this );
    m_fileManager->setDestinationFolder("D:/");
    m_fileManager->setMetaInfo( info );
    m_fileManager->start();
}

Downloader::~Downloader()
{
}

void Downloader::addPeer(const PeerInfo &peerInfo)
{
    auto peer = new PeerConnection( m_torrentClient, this );
    connect( peer, &PeerConnection::handshakeFailed, this, &Downloader::removePeer );
    connect( peer, &PeerConnection::handshakeIsDone, peer, &PeerConnection::sendInterested );
    connect( peer, &PeerConnection::IsPeerChokingChanged, this, &Downloader::downloadNextBlock );
    connect( peer, &PeerConnection::BlockDownloaded, this, &Downloader::writeIncomingBlock );
    peer->connectToPeer( peerInfo, m_torrentFileInfo.GetInfoHashSHA1(), m_torrentFileInfo.GetPieces().size() );
    m_peers.insert( peerInfo, peer );
}

void Downloader::removePeer()
{
    auto senderPeer = qobject_cast< PeerConnection * >( sender() );
    if ( senderPeer == nullptr ){
        qWarning() << Q_FUNC_INFO << "senderPeer == nullptr";
        return;
    }
    auto peerInfo = senderPeer->GetPeerInfo();
    auto failedPeer = m_peers.take( peerInfo );
    connect( failedPeer, &PeerConnection::disconnected, failedPeer, &PeerConnection::deleteLater );
}

void Downloader::writeIncomingBlock( quint32 index, quint32 begin, const QByteArray &block )
{
    m_fileManager->write( index, begin, block );
}

void Downloader::downloadNextBlock(bool isPeerChoking)
{
    if ( isPeerChoking ){
        return;
    }
    auto senderPeer = qobject_cast< PeerConnection * >( sender() );
    if ( senderPeer == nullptr ){
        qWarning() << Q_FUNC_INFO << "senderPeer == nullptr";
        return;
    }
    if ( !m_peers.contains(senderPeer->GetPeerInfo()) ){
        qDebug() << Q_FUNC_INFO << "!m_peers.contains";
        return;
    }
    m_peers[ senderPeer->GetPeerInfo() ]->sendRequest( m_downloadedPieceIndex, m_downloadedBytes, 16000);
}
