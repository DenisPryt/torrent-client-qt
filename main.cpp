//#include <QApplication>
//#include <QQmlApplicationEngine>
//#include <QQmlComponent>
//#include <QQuickWindow>
#include <QDebug>

#include "torrentfileinfo.h"

int main(int argc, char *argv[])
{
    auto res = TorrentFileInfo::parse( "D:\\test.torrent" );
    auto res2 = TorrentFileInfo::parse( "D:\\test2.torrent" );
    return 0;
    /*
    QApplication app(argc, argv);

    QQmlEngine engine;
    QQmlComponent component(&engine);
    QQuickWindow::setDefaultAlphaBuffer(true);
    component.loadUrl(QUrl("qrc:/main.qml"));
    if ( component.isReady() )
        component.create();
    else
        qWarning() << component.errorString();

    return app.exec();*/
}
