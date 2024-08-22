#include "dialogs/mainwindow.h"


#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QSettings>
#include <QDesktopWidget>
#include <QLoggingCategory>
#include <QCommandLineParser>
#include "task.h"
#include "settings.h"
#include "conf.h"
#include <QMessageBox>
#include "utils.h"
#include "appcontext.h"
#include "taskrunnermanager.h"

namespace Lb {
    namespace Globals {
        QString tasksDirectory; // override tasks directory from "~/.lbackup" to something else
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName("otsakir");
    QApplication::setApplicationName("Lisa Backup");


    // set log level
    //QLoggingCategory::setFilterRules(QStringLiteral("default.debug=true\ndefault.info=true"));
    QLoggingCategory::setFilterRules(QStringLiteral("default.info=true\ndefault.debug=true"));
    registerQtMetatypes();

    qInfo() << "Starting Lisa Backup " << LBACKUP_VERSION << "...";
    qDebug() << "Application scripts in " << Lb::appScriptsDir();
    qDebug() << "Application directory is " << QCoreApplication::applicationDirPath();

    Lb::setupDirs(); // created directory structure if not there

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption startMinimizedOption(QStringList() << "m", "Start minimized to system tray");
    parser.addOption(startMinimizedOption);
    QCommandLineOption taskDirectoryOverrideOption(QStringList() << "d", "Override tasks directory absolute path", "path");
    parser.addOption(taskDirectoryOverrideOption);
    parser.process(app);

    // default settings
    QSettings settings;
    //settings.setValue("initialized", false);  // Remove settings file. Uncomment this to start afresh
    if ( ! settings.value("initialized", false).toBool())
    {
        qInfo() << "Blank settings found. They will get initialized.";
        settings.setValue(Settings::TaskrunnerKey, static_cast<int>(Settings::Taskrunner::Gui));
        settings.setValue(Settings::Keys::TaskrunnerConfirm, 2); // i.e. true
        settings.setValue(Settings::Keys::TaskrunnerShowDialog, 2); // i.e. true
        settings.setValue("initialized",true);
        settings.setValue(Settings::LoglevelKey, static_cast<int>(Settings::Loglevel::Errors));
        settings.setValue(Settings::Keys::KeepRunningInTray, 0);
    }
    // explicitly set KeepRunningInTray setting if started minimized
    if (parser.isSet(startMinimizedOption))
    {
        settings.setValue(Settings::Keys::KeepRunningInTray, 1);
    }
    if (parser.isSet(taskDirectoryOverrideOption))
    {
        Lb::Globals::tasksDirectory = parser.value(taskDirectoryOverrideOption);
    }

    qDebug() << "Tasks in " << Lb::dataDirectory();
    settings.setValue("ApplicationFilePath", QApplication::applicationFilePath());
    qInfo() << "Settings file path:" << settings.fileName();

    TaskLoader taskLoader(Lb::dataDirectory());

    // "GUI" mode
    QIcon::setThemeName("Papirus");

    AppContext appContext;
    TaskRunnerManager taskRunnerManager;
    appContext.taskRunnerManager = &taskRunnerManager;
    Common::GlobalSignals globalSignals;
    appContext.globalSignals = &globalSignals;
    MainWindow w(parser.isSet(startMinimizedOption), QString(), &appContext);

    // center within desktop
    QDesktopWidget *desktop = QApplication::desktop();
    QPoint windowOrigin((desktop->width()-w.width())/2, (desktop->height()-w.height())/2);
    w.move(windowOrigin);

    // allow minimize-to-tray
    QApplication::setQuitOnLastWindowClosed(false);

    if (!parser.isSet(startMinimizedOption))
    {
        w.show();
    }

    return app.exec();
}
