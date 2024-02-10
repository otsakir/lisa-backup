#ifndef TREEVIEWTASKS_H
#define TREEVIEWTASKS_H

#include <QStandardItem>
#include <QTreeView>
#include "../common.h"

class TaskLoader;
class TaskRunnerManager;

class TreeViewTasks : public QTreeView
{
    Q_OBJECT
public:
    TreeViewTasks(TaskLoader* taskLoader, TaskRunnerManager* taskRunnerHelper, QWidget* parent);
    int taskCount();
    void showDetails(bool show);
    const QString currentTaskId();
    void refresh(const QString& reselectTask);
    void boldSingleRow(int row);
    void boldSingleRow(const QString& taskname);
    ~TreeViewTasks();

public slots:
    void removeCurrent();
    void onTaskRunnerEvent(const QString taskname, Common::TaskRunnerEventType eventType);

signals:
    void currentTaskIs(QString taskName, const QModelIndex& modelIndex);  // taskName is empty string if there is no current at all. This also first at the start.
    void taskDoubleClicked(const QString taskname);
    void taskRemoved(const QString taskname);

protected:
    void showEvent(QShowEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void onCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void populateTasks();
    int rowByTaskname(const QString taskname);

    QStandardItemModel* model; // TODO - check this is released from RAM
    TaskLoader* taskLoader;
    TaskRunnerManager* taskRunnerHelper;
    bool detailsShown;
    QFont boldFont;
};

#endif // TREEVIEWTASKS_H
