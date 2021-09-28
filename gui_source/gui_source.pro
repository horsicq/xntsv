QT       += core widgets sql

TARGET = xntsv

TEMPLATE = app

SOURCES += \
    guimainwindow.cpp \
    main_gui.cpp \
    prototype.cpp \
    htmltable.cpp \
    history.cpp \
    about.cpp \
    dialogmemorymap.cpp

HEADERS  += \
    ../global.h \
    guimainwindow.h \
    prototype.h \
    htmltable.h \
    history.h \
    about.h \
    _version.h \
    dialogmemorymap.h

FORMS    += \
    guimainwindow.ui \
    prototype.ui \
    about.ui \
    dialogmemorymap.ui


RC_FILE = icon.rc

RESOURCES += \
    resource.qrc

include(../build.pri)

!contains(XCONFIG, xprocesswidget) {
    XCONFIG += xprocesswidget
    include(../../_mylibs/XProcessWidget/xprocesswidget.pri)
}
