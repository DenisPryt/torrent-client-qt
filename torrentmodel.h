#ifndef TORRENTMODEL_H
#define TORRENTMODEL_H

#include <QAbstractListModel>
#include <QHash>
#include <QByteArray>

#include "torrent.h"

class TorrentModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum TorrentRoles{
        torName = Qt::UserRole + 1,
        torDownloaded,
        torLeft,
        torInvalidRole
    };
    TorrentModel( QObject *parent = nullptr ) : QAbstractListModel(parent) {}
    virtual ~TorrentModel() {}

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    virtual QHash< int, QByteArray > roleNames() const override;

    virtual bool insertRows(int row, int count, const QModelIndex &parent) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent) override;

private:
    QList< Torrent* >           m_torrentsList;
};

#endif // TORRENTMODEL_H
