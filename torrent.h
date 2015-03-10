#ifndef TORRENT_H
#define TORRENT_H

#include <QObject>
#include <QSharedPointer>

#include "macro.h"

class TorrentDownloadInfo{
public:
    void clear();
    TorrentDownloadInfo(){ clear(); }

    qint64      Downloaded;
    qint64      Left;
    qint64      Uploaded;

    qint32      Key;
};

class TorrentFileInfo;

class Torrent : public QObject
{
    Q_OBJECT
public:
    explicit Torrent(QSharedPointer< TorrentFileInfo > torFileInf, QObject *parent = 0);
    ~Torrent();

    PROP_GET( QSharedPointer< TorrentFileInfo >,        TorrentFileInfo )
    PROP_GET( QSharedPointer< TorrentDownloadInfo >,    TorrentDownloadInfo )

signals:

public slots:

};

#endif // TORRENT_H
