#include "dialogs/mainwindow.h"


#include <QApplication>
#include <QObject>
#include <QDebug>
#include <QSettings>
#include <QDesktopWidget>
#include <QLoggingCategory>
#include "task.h"
#include "settings.h"
#include "conf.h"
#include <QMessageBox>
#include "utils.h"
#include "appcontext.h"
#include "taskrunnermanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("otsakir");
    QApplication::setApplicationName("Lisa Backup");

    // set log level
    //QLoggingCategory::setFilterRules(QStringLiteral("default.debug=true\ndefault.info=true"));
    QLoggingCategory::setFilterRules(QStringLiteral("default.info=true\ndefault.debug=true"));
    registerQtMetatypes();

    qInfo() << "Starting Lisa Backup " << LBACKUP_VERSION << "...";
    qDebug() << "Tasks in " << Lb::dataDirectory();
    qDebug() << "Application scripts in " << Lb::appScriptsDir();
    qDebug() << "Application directory is " << QCoreApplication::applicationDirPath();

    Lb::setupDirs(); // created directory structure if not there

    // default settings
    QSettings settings;
    settings.setValue("initialized", false);  // Remove settings file. Uncomment this to start afresh
    if ( ! settings.value("initialized", false).toBool())
    {
        qInfo() << "Blank settings found. They will get initialized.";
        settings.setValue(Settings::TaskrunnerKey, static_cast<int>(Settings::Taskrunner::Gui));
        settings.setValue("taskrunner/ShowConfirmation", 2); // i.e. true
        settings.setValue("initialized",true);
        settings.setValue(Settings::LoglevelKey, static_cast<int>(Settings::Loglevel::Errors));
        settings.setValue(Settings::Keys::DataDirectory, Lb::dataDirectory());
        settings.setValue(Settings::Keys::KeepRunningInTray, 0);
    }
    settings.setValue("ApplicationFilePath", QApplication::applicationFilePath());
    qInfo() << "Settings file path:" << settings.fileName();

    TaskLoader taskLoader(Lb::dataDirectory());

    // "GUI" mode
    QIcon::setThemeName("Papirus");

    AppContext appContext;
    TaskRunnerManager taskRunnerManager;
    appContext.taskRunnerManager = &taskRunnerManager;
    MainWindow w(QString(), &appContext);

    // center within desktop
    QDesktopWidget *desktop = QApplication::desktop();
    QPoint windowOrigin((desktop->width()-w.width())/2, (desktop->height()-w.height())/2);
    w.move(windowOrigin);

    // allow minimize-to-tray
    QApplication::setQuitOnLastWindowClosed(false);

    w.show();
    return a.exec();
}
