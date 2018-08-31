QT       += core widgets sql


!contains(QMAKE_TARGET.arch, x86_64) {
    TARGET = xntsv32
} else {
    TARGET = xntsv64
}


TEMPLATE = app


SOURCES += \ 
    prototype.cpp \
    mainwindow.cpp \
    main.cpp \
    htmltable.cpp \
    history.cpp \
    about.cpp \
    dialogmemorymap.cpp

HEADERS  += \ 
    prototype.h \
    mainwindow.h \
    htmltable.h \
    history.h \
    about.h \
    _version.h \
    dialogmemorymap.h

FORMS    += \
    prototype.ui \
    mainwindow.ui \
    about.ui \
    dialogmemorymap.ui


RC_FILE = icon.rc

RESOURCES += \
    resource.qrc

include(../build.pri)
