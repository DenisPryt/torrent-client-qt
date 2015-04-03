#include "torrentclient.h"
#include "clienidgenerators.h"
#include <QDebug>

Q_GLOBAL_STATIC(TorrentClient, torrentClient)

TorrentClient *TorrentClient::instance()
{
    return torrentClient();
}

TorrentClient::TorrentClient()
{
    clear();
    SetIdStyle( IdStyles::Azareus );
    m_Port = 6888;
}

void TorrentClient::clear()
{
    m_IdStyle = IdStyles::Unknown;
    m_ClientId.clear();
    m_Port = 0;
    m_IpAddress.clear();
}

QHostAddress TorrentClient::GetIpAddress() const
{
    QReadLocker locker( &m_lock );
    return m_IpAddress;
}

quint16 TorrentClient::GetPort() const
{
    QReadLocker locker( &m_lock );
    return m_Port;
}

QByteArray TorrentClient::GetClientId() const
{
    QReadLocker locker( &m_lock );
    return m_ClientId;
}

IdStyles::Styles TorrentClient::GetIdStyle() const
{
    QReadLocker locker( &m_lock );
    return m_IdStyle;
}

void TorrentClient::SetIdStyle(IdStyles::Styles style)
{
    QWriteLocker locker( &m_lock );
    m_IdStyle = style;
    generateClientId( m_IdStyle );
}

void TorrentClient::generateClientId(IdStyles::Styles style)
{
    if ( !m_ClientId.isNull() ){
        qWarning() << Q_FUNC_INFO << "reinitialize m_ClientId";
    }
    auto generator = ClientIdGeneratorsFactory::instance()->getGenerator(style);
    m_ClientId = generator->generate();
}
