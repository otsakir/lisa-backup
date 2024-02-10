#ifndef TRIGGERMONITOR_H
#define TRIGGERMONITOR_H

#include <QGroupBox>
#include <QWidget>
#include "../../appcontext.h"
#include "../../common.h"

class DbusUtils;

namespace Ui {
class TaskManager;
}
class TreeViewTasks;


class TaskManager : public QGroupBox
{
    Q_OBJECT

public:
    explicit TaskManager(AppContext* appContext, QWidget *parent = nullptr);
    int taskCount();
    ~TaskManager();

    virtual void showEvent(QShowEvent *e);

private:
    Ui::TaskManager *ui;
    DbusUtils* dbusUtils;
    TreeViewTasks* taskview;

    void removeCurrentTask();

private slots:
    void handleMounted(const QString& label, const QString& uuid);
    void currentTaskChanged(QString taskid);
    void selectTask(QString taskid);

    void on_pushButtonEditTask_clicked();
    void OnPushButtonRun();

public slots:
    void editTaskClicked();
    void showPreselectedTask(QString taskid);
    void refreshView(const QString taskid);
    void setBoldListEntry(const QString taskid); // make a single entry of the list bold

signals:
    void shown();
    void taskSelectedForEdit(const QString taskid); // user wants to edit task 'taskid'
    void runTask(const QString taskname, Common::TaskRunnerReason reason, bool show);
    void newTask();


};

#endif // TRIGGERMONITOR_H
