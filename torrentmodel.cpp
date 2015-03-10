#include "torrentmodel.h"
#include "torrentfileinfo.h"
#include <QDebug>


int TorrentModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED( parent );
    return m_torrentsList.size();
}

QVariant TorrentModel::data(const QModelIndex &index, int role) const
{
    if ( !index.isValid() || role < 0 || role >= torInvalidRole )
        return QVariant();

    if ( index.row() < 0 || index.row() >= m_torrentsList.size() ){
        qWarning() << Q_FUNC_INFO << "INVALID ROW";
        return QVariant();
    }

    auto &torrent = *m_torrentsList[ index.row() ];
    switch (role) {
    case torName:
        return torrent.GetTorrentFileInfo()->GetFileInfo().FilePath;

    case torDownloaded:
        return torrent.GetTorrentDownloadInfo()->Downloaded;

    case torLeft:
        return torrent.GetTorrentDownloadInfo()->Left;
    }
    return QVariant();
}

QVariant TorrentModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    if ( orientation == Qt::Horizontal ){
        const auto &roles = roleNames();
        if ( roles.size() <= role ){
            qWarning() << Q_FUNC_INFO << "INVALID ROLE";
        }
        return roles[ role ];
    }

    return QVariant();
}

QHash<int, QByteArray> TorrentModel::roleNames() const
{
    auto roles = QAbstractListModel::roleNames();
    roles[ torName ]        = "torName";
    roles[ torDownloaded ]  = "torDownloaded";
    roles[ torLeft ]        = "torLeft";
    return roles;
}

bool TorrentModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if ( row < 0 || row > m_torrentsList.size() ){
        qWarning() << Q_FUNC_INFO << "OUT OF RANGE";
        return false;
    }
    Torrent *emptyTorrent = new Torrent( QSharedPointer< TorrentFileInfo >(), this );
    beginInsertRows(parent, row, row + count - 1);
    for ( int i = 0; i < count; ++i ){
        m_torrentsList.insert( row, emptyTorrent );
    }
    endInsertRows();
    return true;
}

bool TorrentModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if ( row + count - 1 > m_torrentsList.size() || row < 0 ){
        qWarning() << Q_FUNC_INFO << "OUT OF RANGE";
        return false;
    }
    beginRemoveRows(parent, row, row + count - 1);
    for ( int i = 0; i < count; ++i ){
        m_torrentsList.removeAt( row );
    }
    endRemoveRows();
    return true;
}
