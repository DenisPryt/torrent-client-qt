#ifndef REQUESTTOSERVERMANAGER_H
#define REQUESTTOSERVERMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <QByteArray>
#include <QUdpSocket>
#include <QSet>

#include "torrentfileinfo.h"
#include "torrent.h"
#include "peerinfo.h"

class TorrentClient;

class RequestToServerManager : public QObject
{
    Q_OBJECT
public:
    explicit RequestToServerManager(TorrentClient *torClient, QObject *parent = 0);
    ~RequestToServerManager();

    QSet< PeerInfo > GetPeers(QSharedPointer< Torrent > torrent, qint32 peersCount = -1 );

signals:

private slots:

//// Requests :
    void connectRequest();
    void announceRequest();
    void scrapeRequest();

    void sendLastActionRequest();

//// Handlers :
    void connectResponseHandler();
    void announceResponseHandler();
    void scrapeResponseHandler();
    void responseHandler();

private:
    enum ActionTypes{
        ActionConnect   = 0,
        ActionAnnounce  = 1,
        ActionScrape    = 2,
        ActionError     = 3
    };
    ActionTypes             m_lastAction;

    QUdpSocket              m_udpSocket;
    QSharedPointer< Torrent > m_torrent;

    qint32                  m_transactionId;
    qint64                  m_connectionId;

    QByteArray              m_response;
    TorrentClient          *m_torrentClient;

    qint32                  m_wantPeersCount;
    uint                    m_requestsAmount;        // После каждого успешного действия сбрасывается

    QSet< PeerInfo >        m_fetchedPeers;
};

#endif // REQUESTTOSERVERMANAGER_H
