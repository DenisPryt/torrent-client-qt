#include "downloader.h"

#include "peerconnection.h"
#include "torrentclient.h"
#include "requesttoservermanager.h"
#include "peersmanager.h"

#include <QMessageBox>

const uint Downloader::MaxPiecesDownloading = 5;
const uint Downloader::MaxBlockSize4Request = 8000;

Downloader::Downloader(const TorrentFileInfo &info, QObject *parent)
    : QObject(parent)
    , m_torrentFileInfo( info )
{
    m_nextPiece2Download = 0;
    m_successConnection = 0;

    auto requestManager = new RequestToServerManager( new Torrent(info) );
    m_peersManager = new PeersManager( requestManager, m_torrentFileInfo.GetInfoHashSHA1(),
                                       m_torrentFileInfo.GetPieces().size(), this );
    connect( m_peersManager, &PeersManager::FetchComplited, this, &Downloader::fetchComplited );

    m_fileManager = new FileManager( this );
    connect( m_fileManager, &FileManager::verificationProgress, this, &Downloader::progressChanged );
    connect( m_fileManager, &FileManager::verificationDone, this, &Downloader::startDownload );
    connect( m_fileManager, &FileManager::pieceVerified, [this]( int index, bool isVer ){
        if ( isVer ){
            emit progressChanged( index * 100 / m_torrentFileInfo.GetPieces().size() );
        }
    } );

    m_fileManager->setDestinationFolder("D:/");
    m_fileManager->setMetaInfo( info );
    m_fileManager->start();
}

Downloader::~Downloader()
{
}

void Downloader::startTorrent()
{
    m_fileManager->startDataVerification();
}

void Downloader::startDownload()
{
    m_peersManager->SetPeersCount( 5 );
    m_peersManager->StartFetchPeers();
}

void Downloader::fetchComplited()
{
    const auto& peers = m_peersManager->GetConnections();
    qDebug() << Q_FUNC_INFO << QThread::currentThreadId() << peers.size();

    m_downloadingPiece2ComplitedBlocks[ 0 ].resize( getBlocksCount() );
    for ( auto peer : peers ){
        connect( peer, &PeerConnection::BlockDownloaded, this, &Downloader::writeIncomingBlock );
        //peer->SetPiecesCount( m_torrentFileInfo.GetPieces().size() );
        peer->DownloadBlock(0, 0, getBlockLength( 0 ) );
    }
}

quint32 Downloader::getBlockLength( quint32 numOfBlock ) const
{
    if ( numOfBlock == getBlocksCount() - 1 ){
        quint32 lastBlockLen = m_torrentFileInfo.GetPieceLength() % MaxBlockSize4Request;
        if ( lastBlockLen > 0 )
            return lastBlockLen;
    }
    return MaxBlockSize4Request;
}

quint32 Downloader::getBlocksCount() const
{
    quint32 blocksCount = m_torrentFileInfo.GetPieceLength() / MaxBlockSize4Request;
    if ( (m_torrentFileInfo.GetPieceLength() % MaxBlockSize4Request) > 0 ){
        ++blocksCount;
    }
    return blocksCount;
}

quint32 Downloader::getBlockNum(quint32 begin)
{
    return begin / MaxBlockSize4Request;
}

quint32 Downloader::getBlockBegin(quint32 blockNum)
{
    return blockNum * MaxBlockSize4Request;
}

void Downloader::writeIncomingBlock( quint32 index, quint32 begin, const QByteArray &block )
{
    qDebug() << "INCOMING BLOCK" << index << begin << block.size();
    m_fileManager->write( index, begin, block );

    auto senderPeer = qobject_cast< PeerConnection * >( sender() );
    if ( senderPeer == nullptr ){
        qCritical() << Q_FUNC_INFO << "senderPeer == nullptr";
        return;
    }

    const auto& peers = m_peersManager->GetConnections();
    auto peerConnection = peers.find( senderPeer->GetPeerInfo() );
    if ( peerConnection == peers.end() ){
        qCritical() << Q_FUNC_INFO << "peerConnection == m_peers.end()";
        return;
    }

    quint32 blockBit = getBlockNum( begin );
    if ( blockBit >= getBlocksCount() - 1 ){
        m_fileManager->verifyPiece( index );

        // new piece
        m_downloadingPiece2ComplitedBlocks[ index + 1 ].resize( getBlocksCount() );
        m_downloadingPiece2NextBlock[ index + 1 ];
        qDebug() << "NEW INDEX!!!\nBLOCK NUM " << 0;
        (*peerConnection)->DownloadBlock( index + 1, 0, getBlockLength( 0 ) );

        return;
        // end new piece
    }
    auto &complBlocks = m_downloadingPiece2ComplitedBlocks[ index ];
    if ( complBlocks.testBit( blockBit ) )
        return;

    complBlocks.setBit( blockBit );

    quint32 nextBlock = ++m_downloadingPiece2NextBlock[ index ];
    qDebug() << "BLOCK NUM " << nextBlock;
    (*peerConnection)->DownloadBlock( index, getBlockBegin(nextBlock), getBlockLength( nextBlock ) );
}

void Downloader::setState(Downloader::States state)
{
    m_state = state;
}
