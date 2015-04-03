#ifndef TORRENTCLIENT_H
#define TORRENTCLIENT_H

#include "macro.h"
#include "clienidgenerators.h"

#include <QObject>
#include <QHostAddress>
#include <QReadWriteLock>

class TorrentClient
{
public:
    inline TorrentClient();
    static TorrentClient *instance();
    void clear();

    QHostAddress GetIpAddress() const;
    quint16      GetPort() const;
    QByteArray   GetClientId() const;        // 20-байтовая строка, которая используется как уникальный идентификатор клиента, сгенерированный им же при запуске.
    IdStyles::Styles GetIdStyle() const;

    void SetIdStyle( IdStyles::Styles style );

    void generateClientId( IdStyles::Styles style = IdStyles::Shadow );

private:
    QHostAddress     m_IpAddress;
    quint16          m_Port;
    QByteArray       m_ClientId;
    IdStyles::Styles m_IdStyle;

    mutable QReadWriteLock m_lock;
};

#endif // TORRENTCLIENT_H
