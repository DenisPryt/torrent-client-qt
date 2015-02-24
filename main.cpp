#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickWindow>
#include <QDebug>
#include <QFile>

#include "torrentfileinfo.h"
#include "requesttoservermanager.h"
#include "torrentclient.h"
#include "torrent.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    QFile file( "D:\\test.torrent" );
    if ( !file.open( QFile::ReadOnly ) ){
        return 0;
    }

    RequestToServerManager requestManager( new TorrentClient(&app) );
    requestManager.GetPeers( TorrentFileInfo::parse( file.readAll() ), QSharedPointer<DownloadingInfo>(new DownloadingInfo) );

//    QQmlEngine engine;
//    QQmlComponent component(&engine);
//    QQuickWindow::setDefaultAlphaBuffer(true);
//    component.loadUrl(QUrl("qrc:/main.qml"));
//    if ( component.isReady() )
//        component.create();
//    else
//        qWarning() << component.errorString();

    return app.exec();
}
