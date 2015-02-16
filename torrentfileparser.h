#ifndef TORRENTFILEPARSER_H
#define TORRENTFILEPARSER_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QVariant>
#include <QHash>
#include <QSharedPointer>

typedef QHash<QByteArray,QVariant> BencodeHash;
Q_DECLARE_METATYPE(BencodeHash)

QT_FORWARD_DECLARE_CLASS( TorrentFileInfo )

class TorrentFileParser
{
public:
    TorrentFileParser(){}
    ~TorrentFileParser(){}

    QSharedPointer< TorrentFileInfo > parse(const QByteArray &data );

private:
    bool parseString( QByteArray &byteString );
    bool parseNumber( qint64 &number );
    bool parseList( QList< QVariant > &list );
    bool parseHash( QHash< QByteArray, QVariant > &hash );

    bool parseStringLength( int &length );

//// Data
private:
    QByteArray          m_bencodeData;
    qint64              m_nextPos;

    qint64              m_infoStartPos;
    qint64              m_lengthOfInfo;

    QString             m_lastErrorString;
};

#endif // TORRENTFILEPARSER_H
