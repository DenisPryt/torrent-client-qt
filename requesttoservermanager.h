#ifndef REQUESTTOSERVERMANAGER_H
#define REQUESTTOSERVERMANAGER_H

#include <QObject>
#include <QSharedPointer>
#include <QByteArray>
#include <QUdpSocket>
#include <QMutex>
#include <QMutexLocker>

#include "torrentfileinfo.h"
#include "torrent.h"

class QNetworkReply;
class QNetworkAccessManager;
class TorrentClient;

class RequestToServerManager : public QObject
{
    Q_OBJECT
public:
    explicit RequestToServerManager(TorrentClient *torClient, QObject *parent = 0);
    ~RequestToServerManager();

    void GetPeers(QSharedPointer< TorrentFileInfo > torFileInfo, QSharedPointer< DownloadingInfo > downlInfo);

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
    QSharedPointer< TorrentFileInfo > m_torrentFleInfo;
    QSharedPointer< DownloadingInfo > m_downloadingInfo;

    qint32                  m_transactionId;
    qint64                  m_connectionId;

    QByteArray              m_response;
    TorrentClient          *m_torrentClient;

    uint                    m_requestsAmount;        // После каждого успешного действия сбрасывается
};

#endif // REQUESTTOSERVERMANAGER_H
