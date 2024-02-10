#include "taskmanager.h"
#include "qevent.h"
#include "ui_taskmanager.h"

#include "../dbusutils.h"
#include "../triggering.h"
#include "components/treeviewtasks.h"
#include "../task.h"
#include "../settings.h"

#include <QMessageBox>
#include <QTimer>


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
    connect(this, &TaskManager::shown, this, [this] () {
        this->refreshView("");
    });

    taskview = new TreeViewTasks(appContext->getTaskLoader(), appContext->taskRunnerManager, this);
    taskview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    taskview->showDetails(true);
    static_cast<QVBoxLayout*>(ui->verticalLayoutTasksContainer->layout())->insertWidget(0, taskview);

    connect(taskview, &TreeViewTasks::currentTaskIs,this, &TaskManager::currentTaskChanged);
    connect(ui->pushButtonDeleteTask, &QPushButton::clicked, this, &TaskManager::removeCurrentTask);
    connect(ui->toolButtonRun, &QPushButton::clicked, this, &TaskManager::OnPushButtonRun);
    connect(ui->toolButtonNewTask, &QToolButton::clicked, this, &TaskManager::newTask);
    connect(taskview, &TreeViewTasks::taskDoubleClicked, this, &TaskManager::taskSelectedForEdit);
    connect(ui->toolButtonEditTask, &QToolButton::clicked, this, &TaskManager::editTaskClicked);

//    TaskRunnerManager* taskRunnerHelper = appContext->taskRunnerManager;
//    taskRunnerHelper->runTask(taskName, Common::TaskRunnerReason::Manual);


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

void TaskManager::showEvent(QShowEvent *e)
{
    emit shown();
}


// executes when an external storage device is mounted
void TaskManager::handleMounted(const QString& label, const QString& uuid)
{
    QSettings settings;
    QStringList tasks;
    Triggering::tasksForUuid(uuid, tasks);
    foreach( QString task, tasks) {
        if (settings.value(Settings::TaskrunnerConfirmKey).toInt() == 2)
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

void TaskManager::setBoldListEntry(const QString taskid)
{

}


void TaskManager::on_pushButtonEditTask_clicked()
{
    QString taskid = taskview->currentTaskId();
    if (!taskid.isEmpty())
    {
        emit taskSelectedForEdit(taskid);
    }
}

void TaskManager::OnPushButtonRun()
{
    QString taskid = taskview->currentTaskId();
    if (!taskid.isEmpty())
    {
        emit runTask(taskid, Common::TaskRunnerReason::Manual, true);
        //hide();
    }

}

void TaskManager::editTaskClicked()
{
    const QString& taskid = taskview->currentTaskId();
    assert(!taskid.isEmpty());
    taskview->boldSingleRow(taskid);
    emit taskSelectedForEdit(taskid);
}

void TaskManager::currentTaskChanged(QString taskid)
{
    ui->toolButtonEditTask->setEnabled(!taskid.isEmpty());
    qInfo() << "current task changed";
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




