set(PROJECT_SOURCES
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        aboutdialog.cpp
        main.cpp
        mainwindow.cpp
        multipledirdialog.cpp
        mylistview.cpp
        newbackuptaskdialog.cpp
        settingsdialog.cpp
        treeviewtasks.cpp
        dbusutils.cpp
        aboutdialog.h
        mainwindow.h
        multipledirdialog.h
        mylistview.h
        newbackuptaskdialog.h
        settingsdialog.h
        treeviewtasks.h
        dbusutils.h
        core.cpp
        logging.cpp
        scripting.cpp
        task.cpp
        terminal.cpp
        triggering.cpp
        utils.cpp
        conf.h
        core.h
        logging.h
        scripting.h
        settings.h
        task.h
        terminal.h
        triggering.h
        utils.h
        aboutdialog.ui  mainwindow.ui  multipledirdialog.ui  newbackuptaskdialog.ui  settingsdialog.ui
        custom-icons.qrc

)

add_library(LisaBackuplib SHARED
    ${PROJECT_SOURCES}
)
generate_export_header(LisaBackuplib)
#target_link_libraries(QWAMlib PUBLIC Qt5::Widgets)
target_link_libraries(LisaBackuplib PUBLIC Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::DBus Qt${QT_VERSION_MAJOR}::Gui )


add_executable(LisaBackup
    main.cpp
)

target_link_libraries(LisaBackup PRIVATE Qt5::Widgets LisaBackuplib)


