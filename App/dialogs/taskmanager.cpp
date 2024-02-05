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
    QDialog(parent),
    ui(new Ui::TaskManager),
    dbusUtils(new DbusUtils())
{
    ui->setupUi(this);
    ui->pushButtonDeleteTask->setIcon(QIcon(":/custom-icons/trash.svg"));

    dbusUtils->registerOnMount();
    connect(dbusUtils, &DbusUtils::labeledDeviceMounted, this, &TaskManager::handleMounted);
    connect(this, &TaskManager::shown, this, &TaskManager::refreshView);

    taskview = new TreeViewTasks(appContext->getTaskLoader(), appContext->taskRunnerManager, this);
    taskview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    taskview->showDetails(true);
    static_cast<QVBoxLayout*>(ui->verticalLayoutTasksContainer->layout())->insertWidget(0, taskview);

    connect(taskview, &TreeViewTasks::currentTaskIs,this, &TaskManager::currentTaskChanged);
    connect(ui->pushButtonDeleteTask, &QPushButton::clicked, this, &TaskManager::removeCurrentTask);
    connect(ui->pushButtonRun, &QPushButton::clicked, this, &TaskManager::OnPushButtonRun);

    Triggering::printTriggers();
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

void TaskManager::refreshView()
{
    qDebug() << "will now refresh view";
}


void TaskManager::on_pushButtonEditTask_clicked()
{
    QString taskid = taskview->currentTaskId();
    if (!taskid.isEmpty())
    {
        emit taskSelectedForEdit(taskid);
        hide();
    }
}

void TaskManager::OnPushButtonRun()
{
    QString taskid = taskview->currentTaskId();
    if (!taskid.isEmpty())
    {
        emit runTask(taskid, Common::TaskRunnerReason::Manual, false);
        //hide();
    }

}

void TaskManager::currentTaskChanged(QString taskid)
{
    ui->pushButtonEditTask->setEnabled(!taskid.isEmpty());
    qInfo() << "current task changed";
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


