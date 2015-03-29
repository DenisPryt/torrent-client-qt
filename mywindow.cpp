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

    QFile file( "D:\\test.torrent" );
    if ( !file.open( QFile::ReadOnly ) ){
        return ;
    }

    m_client = new TorrentClient( this );
    m_torrent.reset( new Torrent( TorrentFileInfo::parse( file.readAll() ) ) );

    m_downloader = new Downloader( m_client, *m_torrent->GetTorrentFileInfo().data(), this );

    RequestToServerManager requestManager( m_client, this );
    m_peers = requestManager.GetPeers( m_torrent ).toList();
    for ( auto peer : m_peers ){
        m_downloader->addPeer( peer );
    }

}

MyWindow::~MyWindow()
{

}

