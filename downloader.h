#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QHash>

#include "peerinfo.h"
#include "torrentfileinfo.h"
#include "filemanager.h"

class PeerConnection;
class TorrentClient;
class RequestToServerManager;
class PeersManager;

class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(const TorrentFileInfo &info, QObject *parent = 0);
    ~Downloader();

    enum States{
        StatePeersProcessing,
        StateFileVerification,
        StateDownloading,
        StateInvalid
    };

signals:
    void progressChanged(double percent);

public slots:
    void startTorrent();
    void fetchComplited();
    void writeIncomingBlock(quint32 index, quint32 begin, const QByteArray &block);

private slots:
    void setState( States state );
    void startDownload();

private:
    States                              m_state;

    QSet< PeerInfo >                    m_peers;
    QHash< PeerInfo, PeerConnection *>  m_peerConnections;
    QHash< quint32, QBitArray >         m_downloadingPiece2ComplitedBlocks;
    QHash< quint32, quint32 >           m_downloadingPiece2NextBlock;
    QBitArray                           m_complitedPieces;

    FileManager                        *m_fileManager;
    PeersManager                       *m_peersManager;
    TorrentFileInfo                     m_torrentFileInfo;
    quint32                             m_nextPiece2Download;

    quint32                             m_successConnection;

private:
    quint32 getBlockLength(quint32 numOfBlock) const;
    quint32 getBlocksCount() const;
    quint32 getBlockNum(quint32 begin);
    quint32 getBlockBegin(quint32 blockNum);

//// Static consts
private:
    static const uint MaxPiecesDownloading;
    static const uint MaxBlockSize4Request;
};

#endif // DOWNLOADER_H
