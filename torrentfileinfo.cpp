#include "torrentfileinfo.h"
#include <QSharedPointer>
#include <QFile>
#include <QDebug>
#include <QHash>
#include <QVariant>
#include <QByteArray>
#include <QDir>

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
QSharedPointer< TorrentFileInfo > TorrentFileInfo::parse( const QString &path )
{
    const int SHA1_HASH_LENGTH = 20;
    QFile file( path );
    if ( !file.open( QFile::ReadOnly ) ){
        qWarning() << "Can\'t open file " << path;
    }

    QByteArray data = file.readAll();
    TorrentFileParser parser;
    auto hashPtr = parser.parse( data );
    auto &hash = *hashPtr;

    auto getIfContains = []( BencodeHash &hash, const QByteArray &fild ) -> QVariant
    {
        if ( hash.contains( fild ) ){
            return hash[ fild ];
        }
        else{
            qDebug() << "Fild " << fild << " is missing";
            return QVariant();
        }
    };

    QSharedPointer< TorrentFileInfo > torrentFileInfo( new TorrentFileInfo );
    torrentFileInfo->m_Comment      = getIfContains( hash, "comment" ).toString();
    torrentFileInfo->m_CreatedBy    = getIfContains( hash, "created by" ).toString();
    torrentFileInfo->m_CreationDate = QDateTime::fromTime_t( getIfContains( hash, "creation date" ).toLongLong() );

    auto announce = getIfContains( hash, "announce" ).toString();
    if ( !announce.isEmpty() ){
        torrentFileInfo->m_TrackerUrlList.append( QUrl( announce ) );
    }

    auto annonceList = getIfContains( hash, "announce-list" ).toStringList();
    for ( auto curTracker : annonceList ){
        torrentFileInfo->m_TrackerUrlList.append( QUrl(curTracker) );
    }

    if ( hash.contains( "info" ) ){
        auto &info = hash[ "info" ].value< BencodeHash >();
        torrentFileInfo->m_PieceLength = getIfContains( info, "piece length" ).toLongLong();
        QByteArray piecesString = getIfContains( info, "pieces" ).toByteArray();
        qint64 pos = 0;
        while ( pos < piecesString.size() ){
            torrentFileInfo->m_Pieces.append( piecesString.mid(pos, SHA1_HASH_LENGTH) );
            pos += SHA1_HASH_LENGTH;
        }

        torrentFileInfo->m_isPeersFromTrackersOnly = info.contains("private") ?
                                                     ( hash["private"].toInt() == 1 ? true : false ) : false;

        if ( info.contains("files") ){      // MultiFile
            torrentFileInfo->m_RootDirName = getIfContains( info, "name" ).toString();
            QList< QVariant > files = getIfContains( info, "files" ).toList();
            for ( auto curFileVariant : files ){
                BencodeHash curFile = curFileVariant.value< BencodeHash >();

                FileInfo curFileInfo;
                QStringList directoryList = getIfContains( curFile, "path" ).toStringList();
                if ( directoryList.size() > 0 ){
                    curFileInfo.FilePath = directoryList.join('\\');
                }
                curFileInfo.FileSize = getIfContains( curFile, "length" ).toLongLong();
                curFileInfo.MD5Sum   = getIfContains( curFile, "md5sum" ).toByteArray();
                torrentFileInfo->m_FilesInfo.append( curFileInfo );
            }
        }
        else{                               // SingleFile
            FileInfo fileInfo;
                     fileInfo.FileSize = getIfContains( info, "length" ).toLongLong();
                     fileInfo.FilePath = getIfContains( info, "name" ).toString();
                     fileInfo.MD5Sum   = getIfContains( info, "md5sum" ).toByteArray();
            torrentFileInfo->m_FilesInfo.append( fileInfo );
        }
    }

    return torrentFileInfo;
}
