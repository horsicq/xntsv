QT       += core widgets

TARGET = xntsv

TEMPLATE = app

macx {
    TARGET = XNTSV
}

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
    CONFIG -= embed_manifest_exe
    QMAKE_MANIFEST = windows.manifest.xml
    XCONFIG += use_xwiniodriver
}

RESOURCES += \
    rsrc.qrc

include(../build.pri)

!contains(XCONFIG, xprocesswidget) {
    XCONFIG += xprocesswidget
    include(../XProcessWidget/xprocesswidget.pri)
}

DISTFILES += \
    ../LICENSE \
    ../README.md \
    ../changelog.txt \
    ../release_version.txt
