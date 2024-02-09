#include "dialogs/mainwindow.h"


#include <QApplication>
#include <QCommandLineParser>
#include <QObject>
#include <QDebug>
#include <QSettings>
#include <QDesktopWidget>
#include <QLoggingCategory>
#include "task.h"
#include "terminal.h"
#include "scripting.h"
#include "settings.h"
#include "conf.h"
#include <QMessageBox>
#include "utils.h"
#include "appcontext.h"
#include "dialogs/components/taskmanager.h"
#include "taskrunnermanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("otsakir");
    QApplication::setApplicationName("Lisa Backup");

    QLoggingCategory::setFilterRules(QStringLiteral("default.debug=true\ndefault.info=true"));
    registerQtMetatypes();

    qInfo() << "Starting Lisa Backup " << LBACKUP_VERSION << "...";



    AppContext appContext;



    QSettings settings;
    settings.setValue("initialized", false);  // Remove settings file. Uncomment this to start afresh

    if ( ! settings.value("initialized", false).toBool())
    {
        qInfo() << "Blank settings found. They will get initialized.";
        settings.setValue(Settings::TaskrunnerKey, static_cast<int>(Settings::Taskrunner::Gui));
        settings.setValue("taskrunner/ShowConfirmation", 2); // i.e. true
        settings.setValue("initialized",true);
        settings.setValue(Settings::LoglevelKey, static_cast<int>(Settings::Loglevel::Errors));
        settings.setValue(Settings::DataDirectoryKey, Lb::dataDirectory());
    }
    settings.setValue("ApplicationFilePath", QApplication::applicationFilePath());
    qInfo() << "Settings file path:" << settings.fileName();

    //parse command line
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption runOption(QStringList() << "r" << "run", "Run the specified task");
    parser.addOption(runOption);
    QCommandLineOption taskOption(QStringList() << "t" << "task", "Choose a task", "task name");
    parser.addOption(taskOption);

    parser.process(a);

    QString taskName;
    if (parser.isSet(taskOption))
        taskName = parser.value(taskOption);

    TaskLoader taskLoader(Lb::dataDirectory());

    if (parser.isSet(runOption))    // "CLI" mode
    {
        if (parser.isSet(taskOption))
        {
            BackupModel persisted;
            if (taskLoader.loadTask(taskName,persisted))
            {
                if (settings.value(Settings::TaskrunnerConfirmKey).toInt() == 2)
                {
                    QMessageBox messageBox(QMessageBox::Information, "Lisa Backup", QString("Backup task '%1' triggered. Shall i proceed ?").arg(taskName), QMessageBox::Yes | QMessageBox::Cancel, nullptr,Qt::Dialog);
                    int ret = messageBox.exec();
                    if (ret == QMessageBox::Yes) {
                        qInfo().noquote() << QString("running backup task '%1'...").arg(taskName);
                        QVector<QString> commands;
                        Scripting::buildBackupCommands(persisted, commands);
                        for (QString& command: commands)
                        {
                            qInfo().noquote() << command;
                            QString out;
                            Terminal::runShellCommand(command, &out);
                            qInfo().noquote() << out;
                        }
                    }
                }
            } else
            {
                qCritical().noquote() << QString("error loading task '%1'...").arg(taskName);
            }
            exit(0);
        } else
        {
            qCritical().noquote() << QString("no backup task specified. Use '-t'.");
            exit(0);
        }
    }

    // "GUI" mode
    QIcon::setThemeName("Papirus");
    TaskRunnerManager taskRunnerManager;
    appContext.taskRunnerManager = &taskRunnerManager;
    MainWindow w(taskName, &appContext);

    //qDebug() << "delete on close: " << triggerMonitorWindow.testAttribute(Qt::WA_DeleteOnClose);

    // center within desktop
    QDesktopWidget *desktop = QApplication::desktop();
    QPoint windowOrigin((desktop->width()-w.width())/2, (desktop->height()-w.height())/2);
    w.move(windowOrigin);

    w.show();
    //triggerMonitorWindow.show();
    return a.exec();
}
