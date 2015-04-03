#include "mywindow.h"
#include <QMessageBox>
#include <QProgressBar>

MyWindow::MyWindow(QWidget *parent) : QWidget(parent)
{
    auto pb = new QProgressBar( this );
    pb->setMaximum( 100 );
    auto layout = new QVBoxLayout;
    layout->addWidget( pb );
    setLayout( layout );

    QFile file( "D:\\test.torrent" );
    if ( !file.open( QFile::ReadOnly ) ){
        return ;
    }

    m_downloader = new Downloader( TorrentFileInfo::parse( file.readAll() ), this );
    connect( m_downloader, &Downloader::progressChanged, pb, &QProgressBar::setValue );
    m_downloader->startTorrent();

}

MyWindow::~MyWindow()
{

}

