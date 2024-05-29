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

private:
    Ui::TaskManager *ui;
    DbusUtils* dbusUtils;
    TreeViewTasks* taskview;
    QString boldTask;

    void removeCurrentTask();

private slots:
    void handleMounted(const QString& label, const QString& uuid);
    void setButtonState(QString taskid);
    void selectTask(QString taskid);
    void OnPushButtonRun();

public slots:
    void editTaskClicked();
    void showPreselectedTask(QString taskid);
    void refreshView(const QString taskid);
    void taskIsNowEdited(const QString taskid);
    void setBoldListEntry(const QString taskid); // make a single entry of the list bold
    void reloadTask(const QString taskid);


signals:
    void newTask();
    void editTask(const QString taskid); // user wants to edit task 'taskid'. In case there is no task at all, 'taskid' is the empty string
    void runTask(const QString taskname, Common::TaskRunnerReason reason, bool show);
    void taskRemoved(const QString taskid);

};

#endif // TRIGGERMONITOR_H
