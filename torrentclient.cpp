#include "torrentclient.h"
#include "clienidgenerators.h"
#include <QDebug>

TorrentClient::TorrentClient(QObject *parent) : QObject(parent)
{
    clear();
    connect( this, &TorrentClient::IdStyleChanged, this, &TorrentClient::generateClientId );
    SetIdStyle( IdStyles::Azareus );
    m_Port = 6888;
}

TorrentClient::~TorrentClient()
{
}

void TorrentClient::clear()
{
    m_IdStyle = IdStyles::Unknown;
    m_ClientId.clear();
    m_Port = 0;
    m_IpAddress.clear();
}

void TorrentClient::generateClientId(IdStyles::Styles style)
{
    if ( !m_ClientId.isNull() ){
        qWarning() << Q_FUNC_INFO << "reinitialize m_ClientId";
    }
    auto generator = ClientIdGeneratorsFactory::instance()->getGenerator(style);
    m_ClientId = generator->generate();
}

void TorrentClient::initialClient()
{

}
