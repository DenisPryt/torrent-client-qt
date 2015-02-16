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

class TorrentFileParser
{
public:
    TorrentFileParser();
    ~TorrentFileParser(){}

    QSharedPointer< BencodeHash > parse(const QByteArray &data );
    void clear();

private:
    bool    parseString( QByteArray &byteString );
    bool    parseNumber( qint64 &number );
    bool    parseList(QList< QVariant > &resList );
    bool    parseHash(QHash< QByteArray, QVariant > &resHash );

    bool    parseStringLength( int &length );
    QChar   currentCharacter() const;
    QChar   nextCharacter() const;
    bool    endOfData() const;

//// Data
private:
    QByteArray          m_bencodeData;
    qint64              m_nextPos;

    qint64              m_infoStartPos;
    qint64              m_lengthOfInfo;

    QString             m_lastErrorString;
};

#endif // TORRENTFILEPARSER_H
