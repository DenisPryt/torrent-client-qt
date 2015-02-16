#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickWindow>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlEngine engine;
    QQmlComponent component(&engine);
    QQuickWindow::setDefaultAlphaBuffer(true);
    component.loadUrl(QUrl("qrc:/main.qml"));
    if ( component.isReady() )
        component.create();
    else
        qWarning() << component.errorString();

    return app.exec();
}
