#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTime>
#include <QTimer>

#include "mainwindow.h"
#include "torrentmodel.h"
#include "ratecontroller.h"
#include "torrentserializer.h"

static QObject *RateControllerProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED( engine )
    Q_UNUSED( scriptEngine )
    return RateController::instance();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    app.setOrganizationName("Pruchkovski");
    //app.setOrganizationDomain("somecompany.com");
    app.setApplicationName("TorrentClientQt");

#if 0
    MainWindow wnd;
    wnd.show();
#else
    qmlRegisterType< TorrentModel >( "Torrent.TorrentModel", 1, 0, "TorrentModel" );
    qmlRegisterType< TorrentClient >( "Torrent.TorrentClient", 1, 0, "TorrentClient" );
    qmlRegisterSingletonType< RateController >("Torrent.RateController", 1, 0, "RateController", RateControllerProvider);
    RateController::instance()->setDownloadLimit( 1024 * 1024 * 10 );
    RateController::instance()->setUploadLimit  ( 1024 * 1024 * 10 );
    TorrentModel torrentModel;
    TorrentSerializer serializer( &torrentModel );
    serializer.load();
    QTimer serializeTimer;
    serializeTimer.setInterval( 1000 * 10 );
    QObject::connect( &torrentModel, &TorrentModel::countChanged, &serializer, &TorrentSerializer::save );
    QObject::connect( &serializeTimer, &QTimer::timeout, &serializer, &TorrentSerializer::save );


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
