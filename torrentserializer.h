#ifndef TORRENTSERIALIZER_H
#define TORRENTSERIALIZER_H

#include <QObject>

class TorrentModel;
class QSettings;

class TorrentSerializer : public QObject
{
    Q_OBJECT
public:
    explicit TorrentSerializer( TorrentModel *model, QObject *parent = 0);
    ~TorrentSerializer();

    TorrentModel *model() const;

signals:

public slots:
    void save();
    void load();

private:
    TorrentModel            *m_model;
    QSettings               *m_settings;
};

#endif // TORRENTSERIALIZER_H
