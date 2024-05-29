#include "taskmanager.h"
#include "ui_taskmanager.h"

#include "../dbusutils.h"
#include "../triggering.h"
#include "components/treeviewtasks.h"
#include "../task.h"
#include "../settings.h"

#include <QMessageBox>
#include <QTimer>
#include <QSettings>
#include <QDebug>


TaskManager::TaskManager(AppContext* appContext, QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::TaskManager),
    dbusUtils(new DbusUtils())
{
    ui->setupUi(this);
    ui->pushButtonDeleteTask->setIcon(QIcon(":/custom-icons/trash.svg"));
    ui->toolButtonRun->setIcon(QIcon(":/custom-icons/play.svg"));
    ui->toolButtonEditTask->setIcon(QIcon(":/custom-icons/edit.svg"));


    dbusUtils->registerOnMount();
    connect(dbusUtils, &DbusUtils::labeledDeviceMounted, this, &TaskManager::handleMounted);

    taskview = new TreeViewTasks(appContext->getTaskLoader(), appContext->taskRunnerManager, this);
    taskview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    taskview->showDetails(true);
    static_cast<QVBoxLayout*>(ui->verticalLayoutTasksContainer->layout())->insertWidget(0, taskview);

    connect(ui->pushButtonDeleteTask, &QPushButton::clicked, this, &TaskManager::removeCurrentTask);
    connect(ui->toolButtonRun, &QPushButton::clicked, this, &TaskManager::OnPushButtonRun);
    connect(ui->toolButtonNewTask, &QToolButton::clicked, this, &TaskManager::newTask);
    connect(taskview, &TreeViewTasks::taskHighlighted, this, &TaskManager::editTask);
    connect(ui->toolButtonEditTask, &QToolButton::clicked, this, &TaskManager::editTaskClicked);
    connect(taskview, &TreeViewTasks::taskRemoved, this, &TaskManager::taskRemoved);
    connect(taskview, &TreeViewTasks::taskGotCurrent, this, &TaskManager::setButtonState);
    connect(appContext->globalSignals, &Common::GlobalSignals::taskModified, this, &TaskManager::reloadTask);

    Triggering::printTriggers();
}

int TaskManager::taskCount()
{
    return taskview->taskCount();
}

TaskManager::~TaskManager()
{
    delete taskview;
    dbusUtils->unregisterOnMount();
    delete dbusUtils;
    delete ui;
}

// executes when an external storage device is mounted
void TaskManager::handleMounted(const QString& label, const QString& uuid)
{
    QSettings settings;
    QStringList tasks;
    Triggering::tasksForUuid(uuid, tasks);
    foreach( QString task, tasks) {
        if (settings.value(Settings::Keys::TaskrunnerConfirm).toInt() == 2)
        {
            QMessageBox messageBox(QMessageBox::Information, "Lisa Backup", QString("Backup task '%1' triggered. Shall i proceed ?").arg(task), QMessageBox::Yes | QMessageBox::Cancel, nullptr,Qt::Dialog);
            int ret = messageBox.exec();
            if (ret == QMessageBox::Yes) {
                qDebug() << "running backup task" << task << "for uuid" << uuid << ".";
                // give some time for the mount point to get available...
                emit runTask(task, Common::Triggered, true);
            }
        } else
        {
            QTimer::singleShot(1000, this, [task, this]() { // TODO - set timeout to a value from settings
                emit runTask(task, Common::Triggered, true);
            });
        }
    }
}

void TaskManager::refreshView(const QString taskid)
{
    this->taskview->refresh(taskid);
}

void TaskManager::taskIsNowEdited(const QString taskid)
{
    setBoldListEntry(taskid);
    setButtonState(taskid);
}

void TaskManager::setBoldListEntry(const QString taskid)
{
    boldTask = taskid;
    taskview->boldSingleRow(taskid);
}

void TaskManager::reloadTask(const QString taskid)
{
    this->taskview->refreshOne(taskid);
}


void TaskManager::OnPushButtonRun()
{
    QSettings settings;

    QString taskid = taskview->currentTaskId();
    if (!taskid.isEmpty())
    {
        bool show = (settings.value(Settings::Keys::TaskrunnerShowDialog).toInt() == 2);
        emit runTask(taskid, Common::TaskRunnerReason::Manual, show);
    }

}

void TaskManager::editTaskClicked()
{
    const QString& taskid = taskview->currentTaskId();
    assert(!taskid.isEmpty());
    taskview->boldSingleRow(taskid);
    emit editTask(taskid);
}

void TaskManager::setButtonState(QString taskid)
{
    ui->toolButtonEditTask->setEnabled(!taskid.isEmpty() && boldTask != taskid);
    ui->pushButtonDeleteTask->setEnabled(!taskid.isEmpty());
    ui->toolButtonRun->setEnabled(!taskid.isEmpty());
}

void TaskManager::selectTask(QString taskid)
{
    this->taskview->refresh(taskid);
}

/**
 * @brief showPreselectedTask slot
 *
 * Show window having this task preselected in the list
 */
void TaskManager::showPreselectedTask(QString taskid)
{
    this->show();
    this->taskview->refresh(taskid);
}

void TaskManager::removeCurrentTask()
{
    assert(!taskview->currentTaskId().isEmpty());

    taskview->removeCurrent();
}





