#include "mywindow.h"

MyWindow::MyWindow(QWidget *parent) : QWidget(parent)
{
    auto btn    = new QPushButton("HANDSHAKE", this);
    m_spinBox   = new QSpinBox(this);
    m_spinBox->setMinimum(0);
    auto layout = new QVBoxLayout;
    layout->addWidget( btn );
    layout->addWidget( m_spinBox );

    setLayout( layout );

    connect( btn, &QPushButton::released, [this](){
        long ind = m_spinBox->value();
        PeerConnection peer( m_peers[ ind ], m_torrent.data(), m_client );
        peer.makeHandshake();

        if ( peer.waitForReadyRead(5000) )
        {
            quint8 pstrLen = -1;
            auto msg = peer.readAll();
            QDataStream stream( &msg, QIODevice::ReadOnly );
            stream >> quint8( pstrLen );
            QByteArray pstr          = msg.mid(1, pstrLen);
            QByteArray reservedZeros = msg.mid(pstrLen + 1, 8);
            QByteArray infoHash      = msg.mid( pstrLen + 9, 20 );
            QByteArray peerId        = msg.mid( pstrLen + 29, 20 );

            if ( pstrLen == 19 && pstr == "BitTorrent protocol" ){
                if ( infoHash == m_torrent->GetTorrentFileInfo()->GetInfoHashSHA1() ){
                    qDebug() << Q_FUNC_INFO << " Handshake is complited ";
                }
                else{
                    qWarning() << Q_FUNC_INFO << " INCORRECT INFO_HASH ";
                }
            }
            else {
                qWarning() << Q_FUNC_INFO << " INCORRECT PROTOCOL";
            }
        }
    } );

    QFile file( "D:\\test.torrent" );
    if ( !file.open( QFile::ReadOnly ) ){
        return ;
    }

    m_client    = new TorrentClient( this );
    m_torrent.reset( new Torrent( TorrentFileInfo::parse( file.readAll() ) ) );

    RequestToServerManager requestManager( m_client, this );
    m_peers = requestManager.GetPeers( m_torrent ).toList();
    m_spinBox->setMaximum( m_peers.size() - 1 );

}

MyWindow::~MyWindow()
{

}

