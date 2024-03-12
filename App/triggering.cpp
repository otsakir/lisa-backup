#include "triggering.h"

#include <QDebug>
#include <QVariant>
#include <QSettings>
#include "core.h"
#include "utils.h"


Triggering::Triggering()
{

}

void Triggering::enableMountTrigger(const QString& taskId, const MountedDevice& triggerEntry)
{
    QSettings settings;
    QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap(); // taskid->fs-uuid
    taskidToUuid[taskId].setValue(triggerEntry);
    settings.setValue("triggersAsUuids", QVariant(taskidToUuid));
}


void Triggering::disableMountTrigger(const QString& taskId)
{
    QSettings settings;
    QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap(); // taskid->fs-uuid
    taskidToUuid.remove(taskId);
    settings.setValue("triggersAsUuids", QVariant(taskidToUuid));
}

// returns the backup task(s) that are linked with this uuid. In case no tasks are linked, 'tasks' list is left empty.
void Triggering::tasksForUuid(const QString& uuidPredicate, QStringList& tasks)
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

bool Triggering::triggerExists(const QString& taskId)
{
    QSettings settings;
    QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap(); // taskid->fs-uuid
    return taskidToUuid.contains(taskId);
}

bool Triggering::triggerEntryForTask(const QString& taskid, MountedDevice& triggerEntry) {
    QSettings settings;
    QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap();
    QVariant variant = taskidToUuid[taskid];
    if (!variant.isValid())
        return false;

    triggerEntry = variant.value<MountedDevice>();
    return true;
}

void Triggering::printTriggers()
{
    QSettings settings;
    QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap(); // taskid->fs-uuid
    qDebug() << taskidToUuid;
}

void Triggering::filterTriggerEntries(const QList<MountedDevice>& availableTriggerEntries, QList<MountedDevice>& filteredTriggerEntries)
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
