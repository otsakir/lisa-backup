#include "treeviewtasks.h"

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
    detailsShown(true)
{
    model = new QStandardItemModel(0,4,this);
    setModel(model);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    QFont boldFont(this->font());
    boldFont.setBold(true);
    this->boldFont = boldFont;

    connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, &TreeViewTasks::onCurrentChanged);
    connect(taskRunnerHelper, &TaskRunnerManager::taskRunnerEvent, this, &TreeViewTasks::onTaskRunnerEvent);

    setIndentation(0);
    setHeaderHidden( !detailsShown);
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
    model->clear();
    if (detailsShown)
    {
        model->setColumnCount(3);
        model->setHorizontalHeaderItem(0, new QStandardItem("Task"));
        model->setHorizontalHeaderItem(1, new QStandardItem("Destination"));
        model->setHorizontalHeaderItem(2, new QStandardItem("Status"));
        model->setHorizontalHeaderItem(3, new QStandardItem("Triggering device"));
    } else
    {
        model->setColumnCount(1);
    }
    setModel(model);

    refresh("");
}

// returns -1 if task is not found
int TreeViewTasks::rowByTaskname(const QString taskname)
{
    for (int irow = 0; irow < model->rowCount(); irow++)
    {
        QString anyTaskname = model->item(irow, 0)->text();
        if (taskname == anyTaskname)
            return irow;
    }
    return -1;

}

void TreeViewTasks::boldSingleRow(int row)
{
    for (int i = 0; i < model->rowCount(); i++)
    {
        QModelIndex index = model->index(i, 0);
        if (i == row)
        {
            model->setData(index, this->boldFont, Qt::FontRole);
            this->highlightedTask = model->data(index).toString();
        }
        else
            model->setData(index, font(), Qt::FontRole);
    }
}

void TreeViewTasks::boldSingleRow(const QString &taskname)
{
    boldSingleRow(rowByTaskname(taskname));
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
        return currentIndex().siblingAtColumn(0).data().toString();
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
    model->removeRows(0, model->rowCount());

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
                         << new QStandardItem(runningState)
                         << new QStandardItem(triggerEntry.uuid);

            }
            else
                rowItems << /*new QStandardItem(backupModel.backupDetails.friendlyName) << */ new QStandardItem(taskId);
            model->appendRow(rowItems);
            if (taskId == highlightedTask && model->rowCount() > 0)
            {
                boldSingleRow(model->rowCount()-1);
            }
        } else {
            // TODO - log error
        }
        counter ++;
    }

    if (reselectPosition != -1)
        this->setCurrentIndex(model->index(reselectPosition,0));
}

int TreeViewTasks::taskCount()
{
    return model->rowCount();
}


void TreeViewTasks::showDetails(bool show)
{
    if (show != detailsShown)
    {
        detailsShown = show;
        setHeaderHidden( !detailsShown);
        populateTasks();
    }
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
        model->removeRow(current.row());
        if (taskId == highlightedTask)
            highlightedTask = "";
        refresh("");
        emit taskRemoved(taskId);
    } else
        return;
}

void TreeViewTasks::onTaskRunnerEvent(const QString taskname, Common::TaskRunnerEventType eventType)
{
    TaskRunnerDialog* taskRunner = taskRunnerHelper->getRunner(taskname);
    if (taskRunner == nullptr)
        return;

    QProcess::ProcessState processState = taskRunner->getScriptProcess().state();
    int row = rowByTaskname(taskname);
    if (row != -1)
    {
        QString processStateString = TaskRunnerDialog::processStateToString(processState);
        model->item(row, 2) -> setText(processStateString);
    }
}

void TreeViewTasks::showEvent(QShowEvent *event)
{
    if (model->rowCount() > 0)
    {
        const QModelIndex& firstIndex = model->index(0,0);
        setCurrentIndex(firstIndex);
        assert(firstIndex == currentIndex());
    }

    event->accept();
}

void TreeViewTasks::mouseDoubleClickEvent(QMouseEvent *event)
{
    QModelIndex modelIndex = indexAt(event->localPos().toPoint());
    int row = modelIndex.row();
    if (row > -1)
    {
        boldSingleRow(row);
        emit taskHighlighted(modelIndex.siblingAtColumn(0).data().toString());
    }
}



void TreeViewTasks::onCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (current.isValid())
    {
        emit taskGotCurrent(current.siblingAtColumn(0).data().toString());
    } else
    {
        emit taskGotCurrent(""); // invalid
    }

}
