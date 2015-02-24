#ifndef TORRENT_H
#define TORRENT_H

#include <QObject>

class DownloadingInfo{
public:
    void clear();
    DownloadingInfo(){ clear(); }

    qint64      Downloaded;
    qint64      Left;
    qint64      Uploaded;

    qint32      NumWant;
    qint32      Key;
};


class Torrent : public QObject
{
    Q_OBJECT
public:
    explicit Torrent(QObject *parent = 0);
    ~Torrent();

signals:

public slots:
};

#endif // TORRENT_H
