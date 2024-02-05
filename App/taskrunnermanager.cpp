#include "taskrunnermanager.h"

#include "dialogs/taskrunnerdialog.h"

TaskRunnerDialog *TaskRunnerManager::newRunner(const QString taskname, bool show)
{
    assert(!taskRunners.contains(taskname));

    TaskRunnerDialog* taskRunner = new TaskRunnerDialog();
    if (!taskRunner->setTask(taskname))
    {
        delete taskRunner;
        return nullptr;
    }

    connect(taskRunner, &TaskRunnerDialog::taskDialogNotNeeded, this, &TaskRunnerManager::removeRunner);
    connect(taskRunner, &TaskRunnerDialog::taskStateChanged, this, &TaskRunnerManager::OnChildTaskStateChanged);

    taskRunners.insert(taskname, taskRunner);
    if (show)
        taskRunner->show();
    return taskRunner;
}

TaskRunnerDialog *TaskRunnerManager::getRunner(const QString taskname)
{
    return taskRunners.contains(taskname) ? taskRunners.value(taskname) : nullptr;
}

void TaskRunnerManager::OnChildTaskStateChanged(const QString taskname, QProcess::ProcessState state)
{
    emit taskRunnerEvent(taskname, Common::ProcessStateChanged);
}

void TaskRunnerManager::runTask(const QString taskname, Common::TaskRunnerReason reason, bool show)
{
    TaskRunnerDialog* taskRunner = getRunner(taskname);
    if (taskRunner == nullptr)
    {
        taskRunner = newRunner(taskname, show);
        taskRunner->run(reason);
    } else
    {
        if (taskRunner->getScriptProcess().state() == QProcess::NotRunning)
            taskRunner->run(reason);
        if (show)
        {
            taskRunner->show();
            taskRunner->activateWindow();
        }
    }
}

void TaskRunnerManager::removeRunner(const QString taskname)
{
    TaskRunnerDialog* taskRunner = getRunner(taskname);
    assert(taskRunner != nullptr);

    delete taskRunner;
    taskRunners.remove(taskname);
}
