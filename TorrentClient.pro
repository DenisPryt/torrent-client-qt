TEMPLATE = app

CONFIG += c++11

QT += qml quick widgets network

SOURCES += main.cpp \
    addtorrentdialog.cpp \
    bencodeparser.cpp \
    connectionmanager.cpp \
    mainwindow.cpp \
    metainfo.cpp \
    peerwireclient.cpp \
    ratecontroller.cpp \
    filemanager.cpp \
    torrentclient.cpp \
    torrentserver.cpp \
    trackerclient.cpp \
    torrentmodel.cpp \
    torrentmodelitem.cpp

RESOURCES += qml.qrc \
	icons.qrc

FORMS += forms/addtorrentform.ui
	
# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += addtorrentdialog.h \
    bencodeparser.h \
    connectionmanager.h \
    mainwindow.h \
    metainfo.h \
    peerwireclient.h \
    ratecontroller.h \
    filemanager.h \
    torrentclient.h \
    torrentserver.h \
    trackerclient.h \
    torrentmodel.h \
    torrentmodelitem.h
