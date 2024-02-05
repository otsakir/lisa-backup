#ifndef TRIGGERING_H
#define TRIGGERING_H

#include "qdebug.h"
#include <QString>
#include <QVariant>
#include <QSettings>
#include "core.h"
#include "utils.h"

class Triggering
{
private:
    Triggering();
public:

    static void enableMountTrigger(const QString& taskId, const MountedDevice& triggerEntry)
    {
        QSettings settings;
        QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap(); // taskid->fs-uuid
        taskidToUuid[taskId].setValue(triggerEntry);
        settings.setValue("triggersAsUuids", QVariant(taskidToUuid));
    }


    static void disableMountTrigger(const QString& taskId)
    {
        QSettings settings;
        QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap(); // taskid->fs-uuid
        taskidToUuid.remove(taskId);
        settings.setValue("triggersAsUuids", QVariant(taskidToUuid));
    }

    // returns the backup task(s) that are linked with this uuid. In case no tasks are linked, 'tasks' list is left empty.
    static void tasksForUuid(const QString& uuidPredicate, QStringList& tasks)
    {
        QSettings settings;
        QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap(); // taskid->fs-uuid
        QMap<QString, QVariant>::const_iterator iterator = taskidToUuid.constBegin();
        tasks.clear();
        while (iterator != taskidToUuid.constEnd())
        {
            QString key = iterator.key();
            //QVariant value = iterator.value();
            if (iterator.value().value<MountedDevice>().uuid == uuidPredicate)
            {
                tasks.append(key);
            }
            iterator ++;
        }
    }

    static bool triggerExists(const QString& taskId)
    {
        QSettings settings;
        QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap(); // taskid->fs-uuid
        return taskidToUuid.contains(taskId);
    }

    static bool triggerEntryForTask(const QString& taskid, MountedDevice& triggerEntry) {
        QSettings settings;
        QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap();
        QVariant variant = taskidToUuid[taskid];
        if (!variant.isValid())
            return false;

        triggerEntry = variant.value<MountedDevice>();
        return true;
    }

    // if there is a task->uuid mapping for the specific task it returns the uuid. Otherwise, empty string.
    /*static const QString triggerUuidForTask(const QString& taskid)
    {
        MountedDevice triggerEntry;
        if (!triggerEntryForTask(taskid, triggerEntry))
            return QString();

        return triggerEntry.uuid;
    }*/

    static void printTriggers()
    {
        QSettings settings;
        QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap(); // taskid->fs-uuid
        qDebug() << taskidToUuid;
    }

    static void filterTriggerEntries(const QList<MountedDevice>& availableTriggerEntries, QList<MountedDevice>& filteredTriggerEntries)
    {
        filteredTriggerEntries.clear();
        bool skip;
        foreach( const MountedDevice& mountedDevice, availableTriggerEntries) {
            skip = false;
            foreach (const QString& prefix, Lb::excludedDevicePathPrefix())
            {
                if (mountedDevice.mountPoint.startsWith(prefix))
                {
                    skip = true;
                    break;
                }
            }
            if (skip)
                continue; // this is an excluded device
            if (mountedDevice.mountPoint.isEmpty())
                continue;

            filteredTriggerEntries.append(mountedDevice);
        }
    }

};

#endif // TRIGGERING_H
