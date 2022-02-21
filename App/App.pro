QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 debug

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    mylistview.cpp \
    newbackuptaskdialog.cpp \
    systemdunitdialog.cpp

HEADERS += \
    aboutdialog.h \
    mainwindow.h \
    mylistview.h \
    newbackuptaskdialog.h \
    systemdunitdialog.h

FORMS += \
    aboutdialog.ui \
    mainwindow.ui \
    newbackuptaskdialog.ui \
    systemdunitdialog.ui


unix:!macx: LIBS += -L$$OUT_PWD/../Lib/ -lLib

INCLUDEPATH += $$PWD/../Lib
DEPENDPATH += $$PWD/../Lib

unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../Lib/libLib.a

RESOURCES += \
    fontawesome.qrc

DISTFILES += \
    qtlogging.ini
