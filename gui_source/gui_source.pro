QT       += core widgets

TARGET = xntsv

TEMPLATE = app

SOURCES += \
    dialogabout.cpp \
    dialogoptions.cpp \
    guimainwindow.cpp \
    main_gui.cpp

HEADERS  += \
    ../global.h \
    dialogabout.h \
    dialogoptions.h \
    guimainwindow.h

FORMS    += \
    dialogabout.ui \
    dialogoptions.ui \
    guimainwindow.ui


win32 {
    RC_ICONS = ../icons/main.ico
}

RESOURCES += \
    rsrc.qrc

include(../build.pri)

!contains(XCONFIG, xprocesswidget) {
    XCONFIG += xprocesswidget
    include(../../_mylibs/XProcessWidget/xprocesswidget.pri)
}
