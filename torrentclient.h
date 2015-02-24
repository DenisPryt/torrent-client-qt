#ifndef TORRENTCLIENT_H
#define TORRENTCLIENT_H

#include "macro.h"
#include <QObject>
#include <QHostAddress>
#include "clienidgenerators.h"

class TorrentClient : public QObject
{
    Q_OBJECT
public:
    explicit TorrentClient(QObject *parent = 0);
    ~TorrentClient();
    void clear();

    PROP_GET( QHostAddress, IpAddress )
    PROP_GET( quint16,      Port )
    PROP_GET( QByteArray,   ClientId )        // 20-байтовая строка, которая используется как уникальный идентификатор клиента, сгенерированный им же при запуске.
    PROP_SIMPLE( IdStyles::Styles, IdStyle )

public:

public slots:
    void generateClientId( IdStyles::Styles style = IdStyles::Shadow );
    void initialClient();

signals:
    void IdStyleChanged( const IdStyles::Styles &newStyle );
};

#endif // TORRENTCLIENT_H
