cmake_minimum_required(VERSION 3.5)

project(LisaBackup VERSION 0.1 LANGUAGES CXX)

include(GenerateExportHeader)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# to make this a lib
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_CXX_VISIBILITY_PRESET hidden)
#set(CMAKE_VISIBILITY_INLINES_HIDDEN TRUE)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS DBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)

find_package(QT NAMES Qt5 Qt6 CONFIG REQUIRED COMPONENTS Core Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test)

add_subdirectory(App)

#add_executable(LisaBackup
#    ${PROJECT_SOURCES}
#)


#set_target_properties(LisaBackup PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
#    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
#    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
#    MACOSX_BUNDLE TRUE
#    WIN32_EXECUTABLE TRUE
#)

#install(TARGETS LisaBackup
#    BUNDLE DESTINATION .
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

#if(QT_VERSION_MAJOR EQUAL 6)
#    qt_finalize_executable(LisaBackup)
#endif()


add_subdirectory(Tests)
