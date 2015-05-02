#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTime>

#include "mainwindow.h"
#include "torrentmodel.h"
#include "ratecontroller.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
#if 0
    MainWindow wnd;
    wnd.show();
#else
    qmlRegisterType< TorrentModel >( "Torrent.TorrentModel", 1, 0, "TorrentModel" );
    qmlRegisterType< TorrentClient >( "Torrent.TorrentClient", 1, 0, "TorrentClient" );
    RateController::instance()->setDownloadLimit(1024 * 1024 * 10);
    RateController::instance()->setUploadLimit( 1024 * 1024 * 10 );
    TorrentModel torrentModel;

    QQmlEngine engine;
    QQmlComponent component( &engine );
    QQuickWindow::setDefaultAlphaBuffer(true);

    engine.rootContext()->setContextProperty("torrentModel", &torrentModel);

    component.loadUrl(QUrl("qrc:/main.qml"));
    if ( component.isReady() )
        component.create();
    else
        qWarning() << component.errorString();
#endif
    return app.exec();
}
