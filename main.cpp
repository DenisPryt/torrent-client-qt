#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickWindow>

#include "mywindow.h"



int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    MyWindow wnd;
    wnd.show();

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
