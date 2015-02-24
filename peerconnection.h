#ifndef PEERCONNECTION_H
#define PEERCONNECTION_H

#include <QObject>
#include <QTcpSocket>

#include "macro.h"

class TorrentClient;

class PeerConnection : public QTcpSocket
{
public:
    PeerConnection(const QByteArray &infoHash, TorrentClient *torrentClient);
    ~PeerConnection();

    //// Read only
    PROP_GET( QByteArray, InfoHash )
    PROP_GET( TorrentClient *, TorrentClient )

public slots:


private slots:
    // handshake: <pstrlen><pstr><reserved><info_hash><peer_id>
    void makeHandshake();

signals:
    void peerChokingChanged( bool newVal );
    void peerInterestedChanged( bool newVal );
    void amChokingChanged( bool newVal );
    void amInterestedChanged( bool newVal );
};

#endif // PEERCONNECTION_H
