#ifndef TRIGGERMONITOR_H
#define TRIGGERMONITOR_H

#include <QDialog>
#include "../appcontext.h"
#include "../common.h"

class DbusUtils;

namespace Ui {
class TaskManager;
}
class TreeViewTasks;


class TaskManager : public QDialog
{
    Q_OBJECT

public:
    explicit TaskManager(AppContext* appContext, QWidget *parent = nullptr);
    ~TaskManager();

    virtual void showEvent(QShowEvent *e);

private:
    Ui::TaskManager *ui;
    DbusUtils* dbusUtils;
    TreeViewTasks* taskview;

    void removeCurrentTask();

private slots:
    void handleMounted(const QString& label, const QString& uuid);
    void refreshView();
    void currentTaskChanged(QString taskid);

    void on_pushButtonEditTask_clicked();
    void OnPushButtonRun();

public slots:
    void showPreselectedTask(QString taskid);

signals:
    void shown();
    void taskSelectedForEdit(const QString taskid);
    void runTask(const QString taskname, Common::TaskRunnerReason reason, bool show);


};

#endif // TRIGGERMONITOR_H
