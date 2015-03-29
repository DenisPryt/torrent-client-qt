#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QHash>

#include "peerinfo.h"
#include "torrentfileinfo.h"
#include "filemanager.h"

class PeerConnection;
class TorrentClient;

class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(TorrentClient *torrentClient, const TorrentFileInfo &info, QObject *parent = 0);
    ~Downloader();

    void addPeer( const PeerInfo &peerInfo );

signals:

public slots:
    void removePeer();
    void writeIncomingBlock(quint32 index, quint32 begin, const QByteArray &block);
    void downloadNextBlock( bool isPeerChoking );

private:
    QHash< PeerInfo, PeerConnection *>  m_peers;
    TorrentClient                      *m_torrentClient;
    TorrentFileInfo                     m_torrentFileInfo;
    FileManager                        *m_fileManager;
    quint32                             m_downloadedPieceIndex;
    quint32                             m_downloadedBytes;
};

#endif // DOWNLOADER_H
