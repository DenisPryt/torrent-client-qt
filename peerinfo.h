#ifndef PEERINFO_H
#define PEERINFO_H

#include <QObject>

class PeerInfo : public QObject
{
    Q_OBJECT
public:
    explicit PeerInfo(QObject *parent = 0);
    ~PeerInfo();

signals:

public slots:
};

#endif // PEERINFO_H
