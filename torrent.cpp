#include "torrent.h"

//// class DownloadingInfo
void TorrentDownloadInfo::clear()
{
    Downloaded = 0;
    Left = 0;
    Uploaded = 0;
    Key = 0;
}

//// class Torrent
Torrent::Torrent(QSharedPointer<TorrentFileInfo> torFileInf, QObject *parent /*= 0*/)
    : QObject(parent)
    , m_TorrentDownloadInfo( new TorrentDownloadInfo )
{
    Q_ASSERT( !torFileInf.isNull() );
    m_TorrentFileInfo = torFileInf;
}

Torrent::~Torrent()
{

}

