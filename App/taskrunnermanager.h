#ifndef TASKRUNNERMANAGER_H
#define TASKRUNNERMANAGER_H

#include <QObject>
#include <QMap>
#include <QProcess>

#include "common.h"

class TaskRunnerDialog;

class TaskRunnerManager :  public QObject
{
    Q_OBJECT

    typedef QMap<QString, TaskRunnerDialog*> TaskRunners;

public:

    TaskRunnerDialog* newRunner(const QString taskname, bool show=true);
    TaskRunnerDialog* getRunner(const QString taskname);

signals:
    void taskRunnerEvent(const QString taskname, Common::TaskRunnerEventType eventType);

public slots:
    void runTask(const QString taskname, Common::TaskRunnerReason reason, bool show=true);

private slots:
    void OnChildTaskStateChanged(const QString taskname, QProcess::ProcessState state); // state of the process of one of the child tasks has changed

private:
    TaskRunners taskRunners;

    void removeRunner(const QString taskname);

    friend class TaskRunnerDialog;
};

#endif // TASKRUNNERMANAGER_H
