#ifndef TREEVIEWTASKS_H
#define TREEVIEWTASKS_H

#include <QTreeView>

class TreeViewTasks : public QTreeView
{
    Q_OBJECT
public:
    TreeViewTasks(QWidget* parent);

public slots:
    void removeCurrent();

signals:
    void currentTaskIs(QString taskName, const QModelIndex& modelIndex);  // taskName is empty string if there is no current at all. This also first at the start.
    //void taskDoubleClicked();

protected:
    void showEvent(QShowEvent *event) override;

private:
    void onCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
};

#endif // TREEVIEWTASKS_H
