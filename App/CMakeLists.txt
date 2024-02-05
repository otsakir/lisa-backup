set(PROJECT_SOURCES
        main.cpp
        dbusutils.cpp dbusutils.h
        core.cpp core.h
        logging.cpp logging.h
        scripting.cpp scripting.h
        task.cpp task.h
        terminal.cpp terminal.h
        triggering.cpp triggering.h
        utils.cpp utils.h
        conf.h
        settings.h        
        common.h
        custom-icons.qrc
        appcontext.h appcontext.cpp
        taskrunnermanager.h taskrunnermanager.cpp
)

add_subdirectory(dialogs)

add_library(LisaBackuplib SHARED
    ${PROJECT_SOURCES}
)
generate_export_header(LisaBackuplib)
#target_link_libraries(QWAMlib PUBLIC Qt5::Widgets)
target_link_libraries(LisaBackuplib PUBLIC Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::DBus Qt${QT_VERSION_MAJOR}::Gui dialogs)


add_executable(LisaBackup
    main.cpp
)

target_link_libraries(LisaBackup PRIVATE Qt5::Widgets LisaBackuplib)


