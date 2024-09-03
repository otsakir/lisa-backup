set(PROJECT_SOURCES
        main.cpp
        dbusutils.cpp dbusutils.h
        core.cpp core.h
        logging.cpp logging.h
        scripting.cpp scripting.h
        task.cpp task.h
        triggering.cpp triggering.h
        utils.cpp utils.h
        conf.h
        settings.h        
        common.h
        custom-icons.qrc
        appcontext.h appcontext.cpp
        taskrunnermanager.h taskrunnermanager.cpp
        globals.h
)

add_compile_definitions(RUN_MODE="${RUN_MODE}")

add_subdirectory(dialogs)

add_library(LisaBackuplib SHARED
    ${PROJECT_SOURCES}
)
generate_export_header(LisaBackuplib)
#target_link_libraries(QWAMlib PUBLIC Qt5::Widgets)
target_link_libraries(LisaBackuplib PUBLIC Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::DBus Qt${QT_VERSION_MAJOR}::Gui dialogs)


#message("CMAKE_CURRENT_BINARY_DIR:" ${CMAKE_CURRENT_BINARY_DIR})
#message("CMAKE_CURRENT_SOURCE_DIR:" ${CMAKE_CURRENT_SOURCE_DIR})

file(INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/app/backup-one.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/scripts USE_SOURCE_PERMISSIONS) # this will copy only if not exists
file(INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/app/run-LisaBackup.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR} USE_SOURCE_PERMISSIONS)

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../scripts/app/backup-one.sh ${CMAKE_CURRENT_BINARY_DIR}/scripts/backup-one.sh COPYONLY)
#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/app/backup-one.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

add_executable(LisaBackup
    main.cpp
)

target_link_libraries(LisaBackup PRIVATE Qt5::Widgets LisaBackuplib)


