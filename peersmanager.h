#ifndef PEERSMANAGER_H
#define PEERSMANAGER_H

#include <QObject>
#include <QList>

#include "peerinfo.h"

class RequestToServerManager;
class PeerConnection;
class QTimer;

class PeersManager : public QObject
{
    Q_OBJECT
public:
    explicit PeersManager(RequestToServerManager *requestManager, const QByteArray &infoHash,
                          uint piecesCount, QObject *parent = 0);
    ~PeersManager();

    const QHash<PeerInfo, PeerConnection *> &GetConnections() const;
    void SetPeersCount( int peersCount );

signals:
    void FetchComplited();

public slots:
    void StartFetchPeers();

private slots:
    void abortAllUnknown();
    void removePeer();
    void handshakedPeer();
    void newSuccessConnection( bool isPeerChoking );

private:
    RequestToServerManager             *m_requestManager;

    QHash< PeerInfo, PeerConnection * > m_complitedConnections;
    QHash< PeerInfo, PeerConnection * > m_handshakedConnections;
    QHash< PeerInfo, PeerConnection * > m_unknownConnections;
    QSet< PeerInfo >                    m_allPeers;
    uint                                m_needPeersCount;

    uint                                m_piecesCount;
    QByteArray                          m_infoHash;
    bool                                m_fetchComplited;

    QTimer                             *m_timerFetch;
    void                                timerFetchHandler();
    uint                                m_timerFetchTimeoutCount;

private:
    bool        isNeedMorePeers() const;
    void        setFetchComplited();
};

#endif // PEERSMANAGER_H
