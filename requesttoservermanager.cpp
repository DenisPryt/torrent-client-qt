#include "requesttoservermanager.h"
#include "torrentclient.h"
#include <QUrlQuery>
#include <QByteArray>
#include <QUrl>
#include <QDebug>
#include <QHostInfo>
#include <QTimer>

RequestToServerManager::RequestToServerManager(TorrentClient *torClient, QObject *parent)
    : QObject(parent)
    , m_torrentClient(torClient)
{
    m_connectionId = 0;
    m_requestsAmount = 0;
    m_lastAction = ActionError;

    connect( &m_udpSocket, &QUdpSocket::stateChanged, [this](QAbstractSocket::SocketState state){
        qDebug() << "SOCKET STATE IS CHANGED : " << state;
    } );
    connect( &m_udpSocket, &QUdpSocket::disconnected, [](){
        qDebug() << "SOCKET IS DISCONNECTED!";
    } );
}

RequestToServerManager::~RequestToServerManager()
{
}

void RequestToServerManager::GetPeers(QSharedPointer< TorrentFileInfo > torFileInfo, QSharedPointer< DownloadingInfo > downlInfo)
{
    if ( torFileInfo.isNull() ){
        qWarning() << Q_FUNC_INFO << "torFileInfo is null";
        return;
    }
    if ( downlInfo.isNull() ){
        qWarning() << Q_FUNC_INFO << "downlInfo is null";
        return;
    }

    m_torrentFleInfo = torFileInfo;
    m_downloadingInfo = downlInfo;


    //connect( &m_udpSocket, &QAbstractSocket::connected, this, &RequestToServerManager::sendLastActionRequest );
    connect( &m_udpSocket, &QIODevice::readyRead, this, &RequestToServerManager::responseHandler );
    m_udpSocket.connectToHost( torFileInfo->GetTrackerUrl().host(), torFileInfo->GetTrackerUrl().port(),
                               QIODevice::ReadWrite, QUdpSocket::IPv4Protocol );
    m_udpSocket.waitForConnected();

    int requestsAmount = 0;
    do{
        ++requestsAmount;
        connectRequest();
        if ( requestsAmount > 4 ){
            qCritical() << "SERVER DO NOT RESPONSE";
        }
    }while( !m_udpSocket.waitForReadyRead(5000) );


    requestsAmount = 0;
    do{
        ++requestsAmount;
        announceRequest();
        if ( requestsAmount > 4 ){
            qCritical() << "SERVER DO NOT RESPONSE";
        }
    }while( !m_udpSocket.waitForReadyRead(5000) );

    /*
    auto infoHash = QUrl( torrentInfoPtr->GetInfoHashSHA1() ).toEncoded(QUrl::FullyEncoded);
    qDebug() << "info hash : " <<  infoHash;

    //long noPeerId = 0;          // Говорит о том, что трекер может пренебречь полем 'peer id' в хэш-таблице 'peers'. Этот параметр игнорируется, если включен компактный режим.
    //long key = 0;               // (опциональный) Дополнительная идентификация, которая не доступна остальным пользователям. Предназначена для того, чтобы клиент мог подтвердить свою подлинность при смене IP-адреса.

    QUrlQuery query( torrentInfoPtr->GetTrackerUrl() );
    // 20-байтовый SHA1-хеш от значения ключа 'info' файла мета-данных
    query.addQueryItem( "info_hash", infoHash );
    // 20-байтовая строка, которая используется как уникальный идентификатор клиента, сгенерированный им же при запуске.
    query.addQueryItem( "peer_id", m_torrentClient->GetClientId() );
    // Номер порта, который прослушивает клиент. Зарезервированы для BitTorrent — 6881-6889
    query.addQueryItem( "port", QByteArray::number( m_torrentClient->GetPort() ) );
    // "1" сигнализирует, что клиет может принимать компактные ответы.
    query.addQueryItem( "compact", "1" );
    // Суммарное количество отданных данных (после того, как клиент послал событие 'started' трекеру) записанное десятичным числом.
    query.addQueryItem( "uploaded", QByteArray::number(0) );
    // Суммарное количество скачанных данных (после того, как клиент послал событие 'started' трекеру) записанное десятичным числом.
    query.addQueryItem( "downloaded", QByteArray::number(0) );
    // Число байт десятичным числом, которое клиент ещё должен скачать.
    query.addQueryItem( "left", QByteArray::number( torrentInfoPtr->GetTotalFilesSize() ) );
    // 'started', 'stopped', 'completed'
    query.addQueryItem( "event", "started" );
    // (опциональный) Если предыдущий ответ содержал значение 'tracker id', это значение нужно вписать сюда.
    if ( !m_trackerId.isEmpty() ){
        query.addQueryItem( "trackerid", m_trackerId );
    }
    // (опциональный) Количество пиров, которое клиент хочет получить от трекера. Значение может быть нулём. Если параметр не задан, по-умолчанию, обычно отдаётся 50 пиров.
    //query.addQueryItem( "numwant", QByteArray::number(30) );

    // (опциональный) Реальный IP-адрес клиентской машины, формат адреса — четыре байта (десятичными числами) разделённых точками
    if ( !m_torrentClient->GetIpAddress().isNull() ){
        query.addQueryItem( "ip", m_torrentClient->GetIpAddress().toString() );
    }

    QUrl requstUrl = torrentInfoPtr->GetTrackerUrl();
    requstUrl.setQuery( query );
    qDebug() << requstUrl;
*/
}

//// Requests

void RequestToServerManager::connectRequest()
{
    m_lastAction = ActionConnect;
    m_transactionId = QDateTime::currentDateTime().toTime_t();
    qDebug() << "CONNECT REQUEST :" << qint64(0x41727101980) << qint32( m_lastAction ) << qint32( m_transactionId );

    QByteArray messege;
    QDataStream( &messege, QIODevice::WriteOnly ) << qint64( 0x41727101980 ) << qint32( m_lastAction )
                                                  << qint32( m_transactionId );
    qDebug() << "WRITTEN BYTES : " << m_udpSocket.write( messege );
}

void RequestToServerManager::announceRequest()
{
    m_lastAction = ActionAnnounce;
    m_transactionId = QDateTime::currentDateTime().toTime_t();

    auto &infoHash = m_torrentFleInfo->GetInfoHashSHA1();
    if ( infoHash.size() != 20 ){
        qWarning() << Q_FUNC_INFO << "INVALIS INFOHASH";
    }

    auto &clientId = m_torrentClient->GetClientId();
    if ( clientId.size() != 20 ){
        qWarning() << Q_FUNC_INFO << "INVALIS CLIENT ID";
    }

    qint32 event = 2;
    qint32 key = 0;
    auto &downlInf = *m_downloadingInfo;
    QByteArray messege;
    QDataStream out( &messege, QIODevice::WriteOnly );
    out << qint64( m_connectionId ) << qint32( m_lastAction ) << qint32( m_transactionId );
    QByteArray messegeEnd;
    QDataStream out2( &messegeEnd, QIODevice::WriteOnly );
    out2<< qint64( downlInf.Downloaded ) << qint64( downlInf.Left )
        << qint64( downlInf.Uploaded ) << qint32( event ) << qint32(m_torrentClient->GetIpAddress().toIPv4Address())
        << qint32( key ) << qint32( downlInf.NumWant ) << qint16( m_torrentClient->GetPort() );

    qDebug() << "ANNOUNCE REQUEST IS :" << qint64( m_connectionId ) << qint32( m_lastAction )
             << qint32( m_transactionId ) << infoHash << clientId << qint64( downlInf.Downloaded )
             << qint64( downlInf.Left ) << qint64( downlInf.Uploaded )
             << qint32( event ) << qint32( 0 )
             << qint32( key ) << qint32( downlInf.NumWant ) << qint16( m_torrentClient->GetPort() );

    messege.append( infoHash ).append( clientId ).append(messegeEnd);
    qDebug() << "ANNOUNCE REQUEST SIZE : " << m_udpSocket.write( messege );
}

void RequestToServerManager::scrapeRequest()
{
    m_lastAction = ActionScrape;

    QByteArray messege;
    QDataStream out( &messege, QIODevice::WriteOnly );

    qDebug() << "SCRAPE REQUEST SIZE : " << m_udpSocket.write( messege );
}

void RequestToServerManager::sendLastActionRequest()
{
    switch ( m_lastAction ) {
    case ActionConnect :
        connectRequest();
        break;

    case ActionAnnounce :
        announceRequest();
        break;

    case ActionScrape :
        scrapeRequest();
        break;

    case ActionError :
        qCritical() << Q_FUNC_INFO << "ERROR ACTION";
        break;

    default:
        qCritical() << Q_FUNC_INFO << "UNKNOWN ACTION";
        break;
    }
}

//// Handlers

void RequestToServerManager::connectResponseHandler()
{
    if ( m_udpSocket.bytesAvailable() < 16 ){
        qWarning() << Q_FUNC_INFO << "INCORRECT RESPONSE SIZE";
    }
    qint32 action = -1;
    qint32 transactionId = -1;
    qint64 connectionId = -1;

    auto buff = m_udpSocket.readAll();
    QDataStream( &buff, QIODevice::ReadOnly ) >> action >> transactionId >> connectionId;
    qDebug() << "CONNECT RESPONSE : " << action << transactionId << connectionId;

    if ( action != (qint32)m_lastAction ){
        qCritical() << Q_FUNC_INFO << "WRONG ACTION";
        return;
    }

    if ( m_transactionId != transactionId ){
        qCritical() << Q_FUNC_INFO << "TRANSACTION ID IS NOT EQUAL";
        return;
    }

    m_lastAction = ActionAnnounce;              // Не обязательно
    m_connectionId = connectionId;
}

void RequestToServerManager::announceResponseHandler()
{
    if ( m_udpSocket.bytesAvailable() < 20 ){
        qWarning() << Q_FUNC_INFO << "INCORRECT RESPONSE SIZE";
    }
    qint32 action = -1;
    qint32 transactionId = -1;
    qint32 interval = -1;
    qint32 leechers = -1;
    qint32 seeders = -1;
    quint32 ipAddress = -1;
    quint16 tcpPort = -1;

    auto buff = m_udpSocket.readAll();
    QDataStream in( &buff, QIODevice::ReadOnly );
    in >> action >> transactionId >> interval >> leechers >> seeders;
    qDebug() << "ANNOUNCE RESPONSE : " << action << transactionId << interval << leechers << seeders;

    if ( action != (qint32)m_lastAction ){
        qCritical() << Q_FUNC_INFO << "WRONG ACTION";
        return;
    }

    if ( m_transactionId != transactionId ){
        qCritical() << Q_FUNC_INFO << "TRANSACTION ID IS NOT EQUAL";
        return;
    }

    int i = 0;
    while ( !in.atEnd() ){
        in >> ipAddress >> tcpPort;
        qDebug() << QHostAddress(ipAddress).toString() << tcpPort << ++i;
    }

}

void RequestToServerManager::scrapeResponseHandler()
{
    qDebug() << "SCRAPE RESPONSE HANDLER";
}

void RequestToServerManager::responseHandler()
{
    qDebug() << "RESPONSE HANDLER";
    switch ( m_lastAction ) {
    case ActionConnect :
        connectResponseHandler();
        m_requestsAmount = 0;
        break;

    case ActionAnnounce :
        announceResponseHandler();
        m_requestsAmount = 0;
        break;

    case ActionScrape :
        scrapeResponseHandler();
        m_requestsAmount = 0;
        break;

    case ActionError :
        qCritical() << Q_FUNC_INFO << "ERROR ACTION";
        break;

    default:
        qCritical() << Q_FUNC_INFO << "UNKNOWN ACTION";
        break;
    }
}
