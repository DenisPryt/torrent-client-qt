#include "mywindow.h"
#include <QMessageBox>

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
        PeerConnection *peer = new PeerConnection( m_client );

        connect( peer, &PeerConnection::handshakeIsDone, [this](){
            QMessageBox::information(this, "Correct handshake", "Correct handshake" );
        } );
        connect( peer, &PeerConnection::handshakeIsDone, peer, &PeerConnection::sendInterested );

        connect( peer, &PeerConnection::handshakeFailed, [this](){
            QMessageBox::critical(this, "Invalid handshake", "Invalid handshake" );
        } );

        long long piecesSize = m_torrent->GetTorrentFileInfo()->GetPieces().size();
        qDebug() << "piecesSize = " << piecesSize;
        peer->connectToPeer( m_peers[ ind ], m_torrent->GetTorrentFileInfo()->GetInfoHashSHA1(),
                             piecesSize );
        peer->prepareHandshakeMsg();
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

