#ifndef PEERCONNECTION_H
#define PEERCONNECTION_H

#include <QObject>

class PeerConnection : public QTCPSocket
{
public:
    PeerConnection();
    ~PeerConnection();
};

#endif // PEERCONNECTION_H
