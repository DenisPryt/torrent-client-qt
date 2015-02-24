#ifndef TORRENT_H
#define TORRENT_H

#include <QObject>

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
