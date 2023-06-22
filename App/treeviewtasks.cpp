#include "treeviewtasks.h"

#include <QDir>
#include <QStandardItem>
#include <QShowEvent>
#include <QMessageBox>
#include "core.h"
#include "qdebug.h"
#include "task.h"
#include "utils.h"
#include "scripting.h"
#include "triggering.h"

TreeViewTasks::TreeViewTasks(QWidget* parent) : QTreeView(parent)
{
    QStandardItemModel* model = new QStandardItemModel(0,1,this);
    setModel(model);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    QDir taskDir(Lb::dataDirectory());
    QStringList filters;
    filters << "*.task";
    taskDir.setNameFilters(filters);
    taskDir.setFilter(QDir::Files);
    taskDir.setSorting(QDir::Time);


    connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, &TreeViewTasks::onCurrentChanged);

    setIndentation(0);
    setHeaderHidden(true);

    QStringList entries = taskDir.entryList();
    BackupModel backupModel;
    model->clear();
    //model->setHorizontalHeaderItem(0, new QStandardItem("Name"));
    model->setHorizontalHeaderItem(0, new QStandardItem("Id"));
    for (int i = 0; i < entries.size(); i++) {
        //qInfo() << "entry " << entries.at(i);
        QString taskId = entries.at(i); // returns "{id}.task"

        taskId = taskId.replace(QRegularExpression("\\.task$"),"");
        if (Tasks::loadTask(taskId, backupModel)) {
            QList<QStandardItem*> rowItems;
            rowItems << /*new QStandardItem(backupModel.backupDetails.friendlyName) << */ new QStandardItem(taskId);
            model->appendRow(rowItems);
        } else {
            // TODO - log error
        }
    }
}

int TreeViewTasks::taskCount()
{
    return model()->rowCount();
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
        if (model()->rowCount() == 0)
            emit currentTaskIs(QString(), QModelIndex());
    } else
        return;
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
