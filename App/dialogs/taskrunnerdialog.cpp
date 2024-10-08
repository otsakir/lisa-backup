#include "taskrunnerdialog.h"
#include "ui_taskrunnerdialog.h"

#include <QMetaEnum>
#include <QSettings>
#include <QDebug>
#include <QFileDialog>
#include <QSequentialAnimationGroup>
#include <QDateTime>
#include <QCloseEvent>
#include "../scripting.h"
#include "../task.h"
#include "../settings.h"
#include "components/qtoolbuttonanimated.h"

#include "../utils.h"



// helps convert ProcessState enums to string
QMetaEnum processStateMetaEnum = QMetaEnum::fromType<QProcess::ProcessState>();

TaskRunnerDialog::TaskRunnerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TaskRunnerDialog),
    reasonStarted(Common::TaskRunnerReason::Manual)
{
    ui->setupUi(this);

    ui->toolButtonStop->setIcon(QIcon(":/custom-icons/stop.svg"));
    ui->toolButtonExportToScript->setIcon(QIcon(":/custom-icons/script.svg"));
    animatedButtonRun = new QToolButtonAnimated(this);
    animatedButtonRun->setIcon(QIcon(":/custom-icons/play.svg"));

    static_cast<QHBoxLayout*>(ui->horizontalLayoutFooterDynamicButtons->layout())->insertWidget(0, animatedButtonRun);

    connect(ui->toolButtonStop, &QToolButton::clicked, this, &TaskRunnerDialog::stopRunningTask);
    connect(animatedButtonRun, &QToolButton::clicked, [this] () {
        this->run(Common::TaskRunnerReason::Manual);
    });
    connect(ui->toolButtonExportToScript, &QToolButton::clicked, this, &TaskRunnerDialog::OnExportScript);
    connect(&scriptProcess, &QProcess::started, this, &TaskRunnerDialog::OnProcessStarted);
    connect(&scriptProcess, &QProcess::errorOccurred, this, &TaskRunnerDialog::OnErrorOccurred);
    connect(&scriptProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,  &TaskRunnerDialog::OnProcessFinished);
    connect(&scriptProcess, &QProcess::stateChanged, this, &TaskRunnerDialog::OnProcessStatusChanged);
    connect(&scriptProcess, &QProcess::readyReadStandardOutput, this, &TaskRunnerDialog::logStandardOut);
    connect(&scriptProcess, &QProcess::readyReadStandardError, this, &TaskRunnerDialog::logStandardError);
    connect(&scriptProcess, &QProcess::readyReadStandardError, this, &TaskRunnerDialog::markError);

    cachedLoggingColor = ui->textEditLog->textColor();
}

TaskRunnerDialog::~TaskRunnerDialog()
{
    delete ui;
    qDebug() << "In TaskRunnerDialog desctructor for " << taskName;
}


bool TaskRunnerDialog::setTask(const QString taskname)
{
    this->taskName = taskname;
    setWindowTitle(QString("Task Runner - %1").arg(taskname));

    const QString datadir = Lb::dataDirectory();
    assert(!datadir.isEmpty());
    TaskLoader loader(datadir);

    BackupModel backupModel;
    if (!loader.loadTask(taskName, backupModel))
    {
        qCritical().noquote() << QString("error loading task '%1'...").arg(taskName);
        return false;
    }
    commands.clear();
    Scripting::buildBackupCommands(backupModel, commands);

    // actually create temporary file and store commands into it
    if (backupScript.open())
    {
        qDebug() << "Created temporary script file " << backupScript.fileName();
        QTextStream stream(&backupScript);
        for (const QString& command: commands)
        {
            stream << command << Qt::endl;
        }
        //stream << "sleep 3" << Qt::endl; // pause for a while to simulate long running backup tasks

        backupScript.close();
    }

    return true;
}


void TaskRunnerDialog::run(Common::TaskRunnerReason reason)
{
    QSettings settings;
    Settings::Loglevel loglevel = GET_INT_SETTING(Settings::Loglevel);

    QStringList arguments;
    arguments << backupScript.fileName();
    assert(backupScript.exists());
    reasonStarted = reason;
    logOnlyErrors = (loglevel == Settings::Loglevel::Errors);

    errorsOccured = false;   // being optimistic
    scriptProcess.start("/bin/bash", arguments);
    scriptProcess.waitForStarted();
}

void TaskRunnerDialog::waitTillDone()
{
    if (scriptProcess.state() == QProcess::Running)
        scriptProcess.waitForFinished();
}

void TaskRunnerDialog::saveScriptAs(const QString filepath)
{
    //QString scriptDefaultDir("/tmp/lisa-backup-scripts/"); // TODO - set this in settings
    QString suggestedFilename = QString("backup-script-%1.sh").arg(taskName);
    QString fileName = QFileDialog::getSaveFileName(this, "Save backup script",suggestedFilename,tr("Bash scripts (*.sh *.bash)"));

    if (!fileName.isEmpty())
    {
        QFile::remove(fileName); // try removing if present
        if (backupScript.copy(fileName))
        {
            QFile copiedScript(fileName);
            copiedScript.setPermissions(QFileDevice::ExeOwner | QFileDevice::ReadOwner);
            qDebug() << "Saved backup script as " << fileName;
        }

    }
}

const QString TaskRunnerDialog::getTaskName() const
{
    return taskName;
}

void TaskRunnerDialog::stopRunningTask()
{
    logAppendError("Manually stopping backup task...", true);
    scriptProcess.terminate();
}

const QProcess &TaskRunnerDialog::getScriptProcess()
{
    return scriptProcess;
}

void TaskRunnerDialog::hideEvent(QHideEvent *event)
{
    event->accept();
    if ( !isVisible() && scriptProcess.state() == QProcess::NotRunning)
        emit taskDialogNotNeeded(taskName); // this goes to a queued connection to remove the dialog if done. Using direct connection fails.

}

const QString TaskRunnerDialog::processStateToString(QProcess::ProcessState state)
{
    return QString(processStateMetaEnum.valueToKey(state));
}


void TaskRunnerDialog::OnProcessStarted()
{
    switch (reasonStarted)
    {
        case Common::TaskRunnerReason::Manual:
            logAppendText(QString("Manually starting backup task %1 at %2").arg(taskName).arg(QDateTime::currentDateTime().toString()), true);
        break;
        case Common::TaskRunnerReason::Triggered:
            logAppendText(QString("Triggered backup task %1 at %2").arg(taskName).arg(QDateTime::currentDateTime().toString()), true);
        break;
    }
    this->animatedButtonRun->animation->start();
}

void TaskRunnerDialog::OnProcessFinished(int exitcode, QProcess::ExitStatus exitstatus)
{
    logAppendText(QString("Task %1 finished").arg(taskName).append(errorsOccured ? " with errors." : "."  ), true);
    this->animatedButtonRun->animation->stop();
    // by default, keep dialog open only in case of errors
    //if (!errorsOccured)
    //    this->close();
}

void TaskRunnerDialog::OnErrorOccurred(QProcess::ProcessError error)
{
    qInfo() << "OnErrorOccurred" << error;
}

void TaskRunnerDialog::OnProcessStatusChanged(QProcess::ProcessState state)
{
    ui->toolButtonStop->setEnabled(state == QProcess::Running);
    this->animatedButtonRun->setEnabled(state == QProcess::NotRunning);

    emit taskStateChanged(taskName, state);
}

void TaskRunnerDialog::logStandardOut()
{
    if (!logOnlyErrors)
    {
        QString out = QString::fromUtf8(scriptProcess.readAllStandardOutput());
        logAppendText(out);
    }
}

void TaskRunnerDialog::logStandardError()
{
    QString out = QString::fromUtf8(scriptProcess.readAllStandardError());
    logAppendError(out);
}

void TaskRunnerDialog::OnExportScript()
{
    saveScriptAs(QString());
}

void TaskRunnerDialog::markError()
{
    errorsOccured = true;
}

void TaskRunnerDialog::logAppendText(const QString text, bool bold)
{
    ui->textEditLog->setTextColor(cachedLoggingColor);
    if (bold)
    {
        int oldWeight = ui->textEditLog->fontWeight();
        ui->textEditLog->setFontWeight(QFont::Bold);
        ui->textEditLog->append(text);
        ui->textEditLog->setFontWeight(oldWeight);
    } else
        ui->textEditLog->append(text);

}

void TaskRunnerDialog::logAppendError(const QString text, bool bold)
{
    ui->textEditLog->setTextColor(QColor("red"));
    if (bold)
    {
        int oldWeight = ui->textEditLog->fontWeight();
        ui->textEditLog->setFontWeight(QFont::Bold);
        ui->textEditLog->append(text);
        ui->textEditLog->setFontWeight(oldWeight);
    } else
        ui->textEditLog->append(text);
}


