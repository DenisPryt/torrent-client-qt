#include "peerconnection.h"
#include "torrentclient.h"

PeerConnection::PeerConnection(const QByteArray &infoHash, TorrentClient *torrentClient )
    : QTcpSocket( torrentClient )
    , m_InfoHash( infoHash )
{
}

PeerConnection::~PeerConnection()
{

}

void PeerConnection::makeHandshake()
{
    const quint8 HANDSHAKE_PSTRLEN = 19;
    const char   HANDSHAKE_PSTR[] = "BitTorrent protocol";

    //write( &HANDSHAKE_PSTRLEN, 1 );
    //write( HANDSHAKE_PSTR, strlen(HANDSHAKE_PSTR) );
    //write( QByteArray(8, '\0') );       // Reserved
    //write( m_InfoHash );
    //write( m_TorrentClient->GetClientId() );
}

