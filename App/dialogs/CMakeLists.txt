

add_library(dialogs
    taskrunnerdialog.cpp taskrunnerdialog.h taskrunnerdialog.ui
    taskmanager.cpp taskmanager.h taskmanager.ui
    newbackuptaskdialog.cpp newbackuptaskdialog.h newbackuptaskdialog.ui
    settingsdialog.cpp settingsdialog.h settingsdialog.ui
    aboutdialog.cpp aboutdialog.h aboutdialog.ui
    mainwindow.cpp mainwindow.h mainwindow.ui
    components/multipledirdialog.cpp components/multipledirdialog.h components/multipledirdialog.ui
    components/triggeringcombobox.cpp components/triggeringcombobox.h
    components/treeviewtasks.cpp components/treeviewtasks.h
    components/qtoolbuttonanimated.h components/qtoolbuttonanimated.cpp
)


target_link_libraries(dialogs PUBLIC Qt5::Widgets Qt5::DBus Qt5::Gui Qt5::DBus)



