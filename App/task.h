#ifndef TASK_H
#define TASK_H

#include <QWidget>
#include "core.h"

class TaskLoader
{
public:
    TaskLoader(QString path);

    void refresh(); // reload task list
    const QList<QString>& getTaskInfo();
    bool loadTask(const QString taskId, BackupModel& persisted);

private:
    QString taskPath;
    QList<QString> taskInfo;
};

namespace Tasks
{

//bool loadTask(const QString taskId, BackupModel& persisted);

void saveTask(const QString taskId, const BackupModel& persisted);

/**
 * @brief deleteTask
 * @param taskId
 * @return true for success
 */
bool deleteTask(const QString taskId);

} // namespace Tasks

#endif // TASK_H
