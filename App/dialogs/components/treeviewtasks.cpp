#include "treeviewtasks.h"

#include <QDir>
#include <QStandardItem>
#include <QShowEvent>
#include <QMessageBox>
#include "../../core.h"
#include "qdebug.h"
#include "../../task.h"
#include "../../triggering.h"
#include "../dialogs/taskrunnerdialog.h"
#include "../taskrunnermanager.h"

TreeViewTasks::TreeViewTasks(TaskLoader* taskLoader, TaskRunnerManager *taskRunnerHelper, QWidget* parent)
    : QTreeView(parent),
    taskLoader(taskLoader),
    taskRunnerHelper(taskRunnerHelper),
    detailsShown(false)
{
    QStandardItemModel* model = new QStandardItemModel(0,4,this);
    setModel(model);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, &TreeViewTasks::onCurrentChanged);
    connect(taskRunnerHelper, &TaskRunnerManager::taskRunnerEvent, this, &TreeViewTasks::onTaskRunnerEvent);

    setIndentation(0);
    setHeaderHidden(true);

    populateTasks();
}

TreeViewTasks::~TreeViewTasks()
{
    qDebug() << "destroying TreeViewTasks";
}


/**
 * @brief TreeViewTasks::populateTasks
 *
 * Reloads task entries from filesystem
 *
 */
void TreeViewTasks::populateTasks()
{
    QStandardItemModel* dataModel = static_cast<QStandardItemModel*>(model());
    dataModel->clear();
    if (detailsShown)
    {
        dataModel->setColumnCount(3);
        dataModel->setHorizontalHeaderItem(0, new QStandardItem("Task"));
        dataModel->setHorizontalHeaderItem(1, new QStandardItem("Destination"));
        dataModel->setHorizontalHeaderItem(2, new QStandardItem("Triggering device"));
        dataModel->setHorizontalHeaderItem(3, new QStandardItem("Status"));
    } else
    {
        dataModel->setColumnCount(1);
    }
    setModel(dataModel);

    refresh("");
}

// returns -1 if task is not found
int TreeViewTasks::rowByTaskname(const QString taskname)
{
    QStandardItemModel* dataModel = static_cast<QStandardItemModel*>(model());

    for (int irow = 0; irow < dataModel->rowCount(); irow++)
    {
        QString anyTaskname = dataModel->item(irow, 0)->text();
        qInfo() << "iterating over task" << anyTaskname;
        if (taskname == anyTaskname)
            return irow;
    }
    return -1;

}

/**
 * @brief currentTaskId
 *
 * Wrapper that gets the task name if available or "" if not.
 *
 */
const QString TreeViewTasks::currentTaskId()
{
    if (currentIndex().isValid())
    {
        return currentIndex().data().toString();
    } else
        return "";
}

/**
 * @brief Reload tasks
 *
 * Reload tasks from filesystem. Try to re-select the task given after reloading.
 *
 */
void TreeViewTasks::refresh(const QString& reselectTask)
{
    QStandardItemModel* dataModel = static_cast<QStandardItemModel*>(model());
    dataModel->removeRows(0, model()->rowCount());

    int counter = 0;
    int reselectPosition = -1;
    const QList<QString>& tasks = taskLoader->getTaskInfo();
    BackupModel backupModel;
    for (const QString& taskId: tasks)
    {
        if (taskLoader->loadTask(taskId, backupModel)) {
            QList<QStandardItem*> rowItems;
            if (detailsShown)
            {
                MountedDevice triggerEntry;
                Triggering::triggerEntryForTask(taskId, triggerEntry);
                if (reselectTask == taskId)
                    reselectPosition = counter;

                // get the running status for this task if any
                TaskRunnerDialog* taskRunner = taskRunnerHelper->getRunner(taskId);
                QString runningState;
                if (taskRunner)
                {
                    QProcess::ProcessState state = taskRunner->getScriptProcess().state();
                    switch (state)
                    {
                        case QProcess::ProcessState::Running:
                            runningState = "Running";
                        break;
                        case QProcess::ProcessState::NotRunning:
                            runningState = "Not Running";
                        break;
                    }
                }
                rowItems << new QStandardItem(taskId)
                         << new QStandardItem(backupModel.backupDetails.destinationPath)
                         << new QStandardItem(triggerEntry.uuid)
                         << new QStandardItem(runningState);
            }
            else
                rowItems << /*new QStandardItem(backupModel.backupDetails.friendlyName) << */ new QStandardItem(taskId);
            dataModel->appendRow(rowItems);
        } else {
            // TODO - log error
        }
        counter ++;
    }

    if (reselectPosition != -1)
        this->setCurrentIndex(dataModel->index(reselectPosition,0));
}

int TreeViewTasks::taskCount()
{
    return model()->rowCount();
}


void TreeViewTasks::showDetails(bool show)
{
    detailsShown = show;
    setHeaderHidden( !detailsShown);
    populateTasks();
}


void TreeViewTasks::removeCurrent()
{
    QModelIndex current = currentIndex();
    assert(current.isValid());

    QString taskId = current.data().toString();
    assert( !taskId.isEmpty() );

    // ask for user confirmation
    int ret = QMessageBox::warning(this, QString("Remove task '%1'").arg(taskId),"You're about to remove a backup task.\n Are you sure ?",QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (ret == QMessageBox::Yes)
    {
        Triggering::disableMountTrigger(taskId);
        Tasks::deleteTask(taskId); // TODO - error handling ? where ?
        // and the entry from the table/tree
        model()->removeRow(current.row());
        refresh("");
    } else
        return;
}

void TreeViewTasks::onTaskRunnerEvent(const QString taskname, Common::TaskRunnerEventType eventType)
{
    TaskRunnerDialog* taskRunner = taskRunnerHelper->getRunner(taskname);
    if (taskRunner == nullptr)
        return;

    QProcess::ProcessState processState = taskRunner->getScriptProcess().state();
    QStandardItemModel* dataModel = static_cast<QStandardItemModel*>(model());
    int row = rowByTaskname(taskname);
    if (row != -1)
    {
        QString processStateString = TaskRunnerDialog::processStateToString(processState);
        dataModel->item(row, 3) -> setText(processStateString);
    }
}

void TreeViewTasks::showEvent(QShowEvent *event)
{
    QAbstractItemModel* model = this->model();
    if (model->rowCount() > 0)
    {
        const QModelIndex& firstIndex = model->index(0,0);
        setCurrentIndex(firstIndex);
        assert(firstIndex == currentIndex());
    }

    event->accept();
}

void TreeViewTasks::onCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (current.isValid())
    {
        emit currentTaskIs(current.data().toString(), current);
    }
}
