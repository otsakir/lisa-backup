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
    multipledirdialog.cpp \
    mylistview.cpp \
    newbackuptaskdialog.cpp \
    systemdunitdialog.cpp \
    treeviewtasks.cpp

HEADERS += \
    aboutdialog.h \
    mainwindow.h \
    multipledirdialog.h \
    mylistview.h \
    newbackuptaskdialog.h \
    systemdunitdialog.h \
    treeviewtasks.h

FORMS += \
    aboutdialog.ui \
    mainwindow.ui \
    multipledirdialog.ui \
    newbackuptaskdialog.ui \
    systemdunitdialog.ui


unix:!macx: LIBS += -L$$OUT_PWD/../Lib/ -lLib

INCLUDEPATH += $$PWD/../Lib
DEPENDPATH += $$PWD/../Lib

unix:!macx: PRE_TARGETDEPS += $$OUT_PWD/../Lib/libLib.a

RESOURCES += \
    custom-icons.qrc \
    fontawesome.qrc

DISTFILES += \
    qtlogging.ini
