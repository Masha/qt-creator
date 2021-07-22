TEMPLATE = lib

QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += MINIMAP_LIBRARY

QTC_PLUGIN_NAME = Minimap
QTC_PLUGIN_DEPENDS = coreplugin texteditor

include(../../qtcreatorplugin.pri)

SOURCES += \
    minimap.cpp \
    minimapstyle.cpp \
    minimapsettings.cpp

HEADERS += \
    minimap_global.h \
    minimapconstants.h \
    minimap.h \
    minimapstyle.h \
    minimapsettings.h

FORMS +=

RESOURCES +=

OTHER_FILES +=

DISTFILES += \
    AUTHORS \
    COPYING \
    README.md
