#ifndef TASKRUNNERDIALOG_H
#define TASKRUNNERDIALOG_H

#include <QDialog>
#include <QTemporaryFile>
#include <QProcess>
#include "../common.h"


class QToolButtonAnimated;
namespace Ui {
class TaskRunnerDialog;
}

class TaskRunnerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaskRunnerDialog(QWidget *parent = nullptr);
    ~TaskRunnerDialog();

    bool setTask(const QString taskname);
    void run(Common::TaskRunnerReason reason);
    void waitTillDone();
    void saveScriptAs(const QString filepath);
    const QString getTaskName() const;
    void stopRunningTask();
    const QProcess& getScriptProcess();

    void hideEvent(QHideEvent* event) override;

    static const QString processStateToString(QProcess::ProcessState state);

signals:
    void taskDialogNotNeeded(const QString taskname); // The task dialog can be discared. This is thrown when the user closes the task dialog and task is not running.
    void taskStateChanged(const QString taskname, QProcess::ProcessState state); // reflects state of underlying process performing backup task

private slots:
    void OnProcessStarted();
    void OnProcessFinished(int exitcode, QProcess::ExitStatus exitstatus);
    void OnErrorOccurred(QProcess::ProcessError error);
    void OnProcessStatusChanged(QProcess::ProcessState state);
    void logStandardOut();   // reads scriptProcess standard out and puts it to log
    void logStandardError(); // reads scriptProcess standard error and puts it to log
    void OnExportScript();
    void markError();

private:
    Ui::TaskRunnerDialog *ui;
    QProcess scriptProcess;

    QString taskName;
    QVector<QString> commands;
    QTemporaryFile backupScript{"tempBackupScript-XXXXXX.sh"};
    QColor cachedLoggingColor; // default color to log stdout in the UI
    Common::TaskRunnerReason reasonStarted;
    bool errorsOccured;

    QToolButtonAnimated* animatedButtonRun;

    void logAppendText(const QString text, bool bold = false);
    void logAppendError(const QString text, bool bold = false);
};



#endif // TASKRUNNERDIALOG_H
