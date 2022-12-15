#ifndef TASK_H
#define TASK_H



bool loadTask(const QString taskId, BackupModel& persisted);

void saveTask(const QString taskId, const BackupModel& persisted);

void deleteTask();


#endif // TASK_H
