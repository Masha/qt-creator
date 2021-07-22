TEMPLATE = lib

QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += QTCREATORDISCORDRICHPRESENCE_LIBRARY

QTC_PLUGIN_NAME = drp
QTC_PLUGIN_DEPENDS = coreplugin projectexplorer

include(../../qtcreatorplugin.pri)

SOURCES += qtcreatordiscordrichpresenceplugin.cpp

HEADERS += qtcreatordiscordrichpresenceplugin.h \
        qtcreatordiscordrichpresence_global.h \
        qtcreatordiscordrichpresenceconstants.h

LIBS += -ldiscord-rpc
