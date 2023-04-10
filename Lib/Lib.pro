CONFIG += qt

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    core.cpp \
    logging.cpp \
    scripting.cpp \
    systemd.cpp \
    task.cpp \
    terminal.cpp \
    utils.cpp

HEADERS += \
    core.h \
    logging.h \
    scripting.h \
    systemd.h \
    task.h \
    terminal.h \
    utils.h

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target
