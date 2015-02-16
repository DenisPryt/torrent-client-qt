#include "torrentfileparser.h"
#include "torrentfileinfo.h"
#include <QDebug>
#include <QVariant>

void TorrentFileParser::clear()
{
    m_lastErrorString.clear();
    m_bencodeData.clear();
    m_nextPos =
    m_infoStartPos =
    m_lengthOfInfo = 0;
}

TorrentFileParser::TorrentFileParser()
{
    clear();
}

QSharedPointer< BencodeHash > TorrentFileParser::parse(const QByteArray &data)
{
    m_bencodeData = data;
    QSharedPointer< BencodeHash > infoHash( new BencodeHash );
    if ( !parseHash( *infoHash ) ){
        qWarning() << "Error";
    }
    return infoHash;
}

bool TorrentFileParser::parseStringLength(int &len)
{
    if ( !currentCharacter().isDigit() ){
        return false;
    }
    QString stringLength;
    while ( !endOfData() ){
        if ( currentCharacter().isDigit() ){
            stringLength.append( currentCharacter() );
            ++m_nextPos;
        }
        else if ( currentCharacter() == ':' && !stringLength.isEmpty() ){
            bool bOk = false;
            len = stringLength.toInt( &bOk );
            ++m_nextPos;
            return bOk;
        }
        else{
            m_lastErrorString = QString("Error at %1 position").arg(m_nextPos);
            return false;
        }
    }

    return false;
}

bool TorrentFileParser::parseString(QByteArray &byteString)
{
    int strLen = -1;
    if ( !parseStringLength( strLen ) )
        return false;

    if ( m_nextPos + strLen > m_bencodeData.size() ){
        qWarning() << "Incorrect string length TorrentFileParser::parseString ";
        m_lastErrorString = QStringLiteral( ".torrent file is not valid. Incorrect string length." );
        return false;
    }

    byteString = m_bencodeData.mid(m_nextPos, strLen);
    m_nextPos += strLen;
    return true;
}

bool TorrentFileParser::parseNumber(qint64 &number)
{
    number = -1;
    QString numberString;

    if ( currentCharacter() != 'i' ){
        return false;
    }

    ++m_nextPos;
    int sign = 1;
    if ( nextCharacter() == '-' ){
        sign = -1;
        ++m_nextPos;
    }
    else if ( nextCharacter() == '+' ){
        ++m_nextPos;
    }

    do{
        if ( currentCharacter() == 'e' ){
            if ( numberString.isEmpty() ){
                m_lastErrorString = QStringLiteral( "No number between tag \'i\' and \'e\''" );
                return false;
            }
            ++m_nextPos;
            bool bOk = false;
            number = sign * numberString.toLongLong( &bOk );
            return bOk;
        }
        else if ( currentCharacter().isDigit() ){
            numberString.append( currentCharacter() );
            ++m_nextPos;
        }
        else{
            m_lastErrorString = QStringLiteral( "Invalid character after tag \'i\'" );
            return false;
        }
    }while( !endOfData() );

    m_lastErrorString = QStringLiteral( "Not fount tag \'e\' after tag \'i\'" );
}

bool TorrentFileParser::parseList(QList< QVariant > &resList)
{
    if ( currentCharacter() != 'l' ){
        return false;
    }

    resList.clear();
    ++m_nextPos;
    do{
        if ( currentCharacter() == 'e' ){
            ++m_nextPos;
            return true;
        }

        qint64 number;
        QByteArray string;
        QList< QVariant > list;
        QHash< QByteArray, QVariant > hash;

        if      ( parseNumber(number) )
            resList << number;
        else if ( parseString(string) )
            resList << string;
        else if ( parseList(list) )
            resList << list;
        else if ( parseHash(hash) )
            resList << QVariant::fromValue<QHash<QByteArray, QVariant> >( hash );
        else{
            m_lastErrorString = QString("Error at %1 position").arg(m_nextPos);
        }

    }while( !endOfData() );

    m_lastErrorString = QStringLiteral( "Tag \'e\' is not found" );
    return false;
}

bool TorrentFileParser::parseHash(QHash< QByteArray, QVariant > &resHash)
{
    if ( currentCharacter() != 'd' ){
        return false;
    }

    ++m_nextPos;
    resHash.clear();
    do {
        if ( m_bencodeData.at(m_nextPos) == 'e' ) {
            ++m_nextPos;
            return true;
        }

        QByteArray key;
        if ( !parseString( key ) )
            break;

        if ( key == "info" )
            m_infoStartPos = m_nextPos;

        qint64 number;
        QByteArray string;
        QList< QVariant > list;
        QHash< QByteArray, QVariant > hash;

        if      ( parseNumber(number) )
            resHash.insert(key, number);
        else if ( parseString(string) )
            resHash.insert(key, string);
        else if ( parseList(list) )
            resHash.insert(key, list);
        else if ( parseHash(hash) )
            resHash.insert( key, QVariant::fromValue<QHash<QByteArray, QVariant> >(hash) );
        else {
            m_lastErrorString = QString("Error at %1 position").arg(m_nextPos);
            return false;
        }

        if (key == "info"){
            m_lengthOfInfo = m_nextPos - m_infoStartPos;
        }

    } while ( !endOfData() );

    m_lastErrorString = QStringLiteral( "Tag \'e\' is not found" );
    return false;
}

QChar TorrentFileParser::currentCharacter() const
{
    if ( m_nextPos < 0 || m_nextPos >= m_bencodeData.size() ){
        qWarning() << "Index out of range TorrentFileParser::currentCharacter";
        return QChar();
    }
    return m_bencodeData.at( m_nextPos );
}

QChar TorrentFileParser::nextCharacter() const
{
    if ( m_nextPos < 0 || m_nextPos + 1 >= m_bencodeData.size() ){
        qWarning() << "Index out of range TorrentFileParser::nextCharacter";
        return QChar();
    }
    return m_bencodeData.at( m_nextPos + 1 );
}

bool TorrentFileParser::endOfData() const
{
    return m_nextPos >= m_bencodeData.size() ;
}

