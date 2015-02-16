#ifndef TORRENTFILEINFO_H
#define TORRENTFILEINFO_H

#include "macro.h"
#include "torrentfileparser.h"

#include <QObject>
#include <QUrl>
#include <QList>
#include <QHash>
#include <QDateTime>
#include <QString>
#include <QByteArray>

struct FileInfo{
    FileInfo(){
        FileSize = 0;
    }
    //name ( для многофайловых торрентфайлов path )
    QString             FilePath;
    //length размер файла в байтах
    quint64             FileSize;
    //md5sum 32-символьная шестнадцатеричная строка соответствующая MD5-сумме файла
    QByteArray          MD5Sum;
};

class TorrentFileInfo : public QObject
{
    Q_OBJECT
public:
    TorrentFileInfo( QObject *parent = nullptr ) : QObject(parent){  }
    ~TorrentFileInfo(){}

//// Simple Properties
    PROP( QList< QUrl >,    TrackerUrlList )    //announce + announce-list
    PROP( QDateTime,        CreationDate )      //creation date Дата создания торрента
    PROP( QString,          CreatedBy )         //created by    Имя и версия программы, которая использовалась для создания torrent-файла
    PROP( QString,          Comment )           //comment       Текстовый комментарий в свободной форме от автора
    PROP( quint64,          PieceLength )       //piece length  Размер каждого куска в байтах
    PROP( QList< QByteArray >, Pieces )         //pieces Строка, составленная объединением 20-байтовых значений SHA1-хэшей каждого куска (один кусок — один хэш)
    PROP( bool,             isPeersFromTrackersOnly )//private

public:
    const QUrl              GetTrackerUrl() const{ return m_TrackerUrlList[0]; }
    const bool              IsSingleFile() const { return m_FilesInfo.size() == 1; }
    const int               FilesCount() const   { return m_FilesInfo.size(); }
    FileInfo&               GetFileInfo( uint pos );
    const FileInfo&         GetFileInfo( uint pos ) const;

//// Static
public:
    static QSharedPointer<TorrentFileInfo> parse(const QString &path);

private:

    //name Имя корневой директории, которую содержит торрент. Носит рекомендательный характер.
    //Для однофайловых не используется
    QString                 m_RootDirName;
    QList< FileInfo >       m_FilesInfo;
};

#endif // TORRENTFILEINFO_H
