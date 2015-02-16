TEMPLATE = app

CONFIG += c++11

QT += qml quick widgets network

SOURCES += main.cpp \
    torrentfileinfo.cpp \
    torrentfileparser.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    torrentfileinfo.h \
    torrentfileparser.h \
    macro.h
