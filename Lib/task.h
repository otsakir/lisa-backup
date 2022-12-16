#ifndef TASK_H
#define TASK_H


namespace Tasks
{

bool loadTask(const QString taskId, BackupModel& persisted);

void saveTask(const QString taskId, const BackupModel& persisted);

/**
 * @brief deleteTask
 * @param taskId
 * @return true for success
 */
bool deleteTask(const QString taskId);

} // namespace Tasks

#endif // TASK_H
