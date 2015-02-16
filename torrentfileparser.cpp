#include "torrentfileparser.h"
#include "torrentfileinfo.h"
#include <QDebug>
#include <QVariant>

QSharedPointer<TorrentFileInfo> TorrentFileParser::parse(const QByteArray &data)
{
    m_bencodeData = data;
    QSharedPointer<TorrentFileInfo> info( new TorrentFileInfo );

    return info;
}

void TorrentFileParser::parseStringLength(QByteArray &byteString)
{
    const int dataSize = m_bencodeData.size();
    QString stringLength;
    while ( m_nextPos < dataSize ){
        QChar currentSumbol = m_bencodeData.at( m_nextPos );
        if ( currentSumbol.isDigit() ){
            stringLength << currentSumbol;
        }
        else if ( currentSumbol == ':' && !stringLength.isEmpty() ){
            int len = stringLength.toInt();
            byteString = m_bencodeData.mid(m_nextPos, len);
            m_nextPos += size;
        }
        ++m_nextPos;
    }
}

bool TorrentFileParser::parseString(QByteArray &byteString)
{
    int strLen = -1;
    if ( !parseStringLength( strLen ) )
        return false;

    if ( m_nextPos + size > m_bencodeData.size() ){
        qWarning() << "Incorrect string length TorrentFileParser::parseString ";
        m_lastErrorString = QStringLiteral( ".torrent file is not valid. Incorrect string length." );
        return false;
    }
    byteString = m_bencodeData.mid(m_nextPos, size);
    m_nextPos += size;
    return true;
}

bool TorrentFileParser::parseNumber(qint64 &number)
{

}

bool TorrentFileParser::parseList(QList<QVariant> &list)
{

}

bool TorrentFileParser::parseHash(QHash<QByteArray, QVariant> &hash)
{
    const int dataSize = m_bencodeData.size();
    if ( m_nextPos < 0 || m_nextPos >= m_bencodeData.size() ){
        qWarning() << "Index out of range TorrentFileParser::parseHash";
        return false;
    }
    if ( m_bencodeData.at(m_nextPos) != 'd' )
        return false;

    QHash<QByteArray, QVariant> tmp;
    ++m_nextPos;

    do {
        if (m_bencodeData.at(m_nextPos) == 'e') {
            ++m_nextPos;
            break;
        }

        QByteArray key;
        if ( !parseString(&key) )
            break;

        if ( key == "info" )
            m_infoStartPos = m_nextPos;

        qint64 number;
        QByteArray byteString;
        QList< QVariant > tmpList;
        QHash< QByteArray, QVariant > localHash;

        if ( parseNumber(&number) )
            tmp.insert(key, number);
        else if ( parseString(&byteString) )
            tmp.insert(key, byteString);
        else if ( parseList(&tmpList) )
            tmp.insert(key, tmpList);
        else if ( parseHash(&localHash) )
            tmp.insert( key, QVariant::fromValue<QHash<QByteArray, QVariant> >(localHash) );
        else {
            m_lastErrorString = QString("error at index %1").arg(m_nextPos);
            return false;
        }

        if (key == "info")
            m_lengthOfInfo = m_nextPos - m_infoStartPos;

    } while (m_nextPos < dataSize);

    hash = tmp;
    return true;
}

bool TorrentFileParser::parseStringLength(int &length)
{
    const int dataSize = m_bencodeData.size();
    QString stringLength;
    while ( m_nextPos < dataSize ){
        QChar currentSumbol = m_bencodeData.at( m_nextPos );
        if ( currentSumbol.isDigit() ){
            stringLength << currentSumbol;
        }
        else if ( currentSumbol == ':' && !stringLength.isEmpty() ){
            bool bOk = false;
            length = stringLength.toInt( &bOk );
            ++m_nextPos;
            return bOk;
        }
        else{
            return false;
        }
        ++m_nextPos;
    }
    return false;
}

