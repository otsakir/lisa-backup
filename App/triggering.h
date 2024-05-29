#ifndef TRIGGERING_H
#define TRIGGERING_H

#include <QString>
#include "core.h"


class Triggering
{
private:
    Triggering();
public:

    static void enableMountTrigger(const QString& taskId, const MountedDevice& triggerEntry);
    static void disableMountTrigger(const QString& taskId);
    static void tasksForUuid(const QString& uuidPredicate, QStringList& tasks); // returns the backup task(s) that are linked with this uuid. In case no tasks are linked, 'tasks' list is left empty.
    static bool triggerExists(const QString& taskId);
    static bool triggerEntryForTask(const QString& taskid, MountedDevice& triggerEntry);
    static void printTriggers();
    static void filterTriggerEntries(const QList<MountedDevice>& availableTriggerEntries, QList<MountedDevice>& filteredTriggerEntries);

};

#endif // TRIGGERING_H
