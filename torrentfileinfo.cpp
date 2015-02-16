#include "torrentfileinfo.h"
#include <QSharedPointer>

FileInfo &TorrentFileInfo::GetFileInfo(uint pos)
{
    if ( pos >= m_FilesInfo.size() ){
        qWarning( "Index out of range TorrentFileInfo::GetFileInfo" );
        return m_FilesInfo[ 0 ];
    }
    return m_FilesInfo[ pos ];
}

const FileInfo &TorrentFileInfo::GetFileInfo(uint pos) const
{
    if ( pos >= m_FilesInfo.size() ){
        qWarning( "Index out of range TorrentFileInfo::GetFileInfo const" );
        return m_FilesInfo[ 0 ];
    }
    return m_FilesInfo[ pos ];
}


//// Static
TorrentFileInfo TorrentFileInfo::parse()
{
    //QSharedPointer< TorrentFileParser > parser( new TorrentFileParser );

}
