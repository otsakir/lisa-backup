#ifndef TRIGGERING_H
#define TRIGGERING_H

#include "qdebug.h"
#include <QString>
#include <QVariant>
#include <QSettings>

class Triggering
{
private:
    Triggering();
public:

    static void enableMountTrigger(const QString& taskId, const QString& uuidPredicate)
    {
        QSettings settings;
        QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap(); // taskid->fs-uuid
        taskidToUuid[taskId] = uuidPredicate;
        qDebug() << "trigger map (in enable): " << taskidToUuid;
        settings.setValue("triggersAsUuids", QVariant(taskidToUuid));
    }


    static void disableMountTrigger(const QString& taskId)
    {
        QSettings settings;
        QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap(); // taskid->fs-uuid
        qDebug() << "trigger map (in disable): " << taskidToUuid;
        taskidToUuid.remove(taskId);
        settings.setValue("triggersAsUuids", QVariant(taskidToUuid));
    }

    // returns the backup task that is linked with this uuid or
    static const QString  triggerTaskForUuid(const QString& uuidPredicate)
    {
        QSettings settings;
        QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap(); // taskid->fs-uuid
        QMap<QString, QVariant>::const_iterator iterator = taskidToUuid.constBegin();
        while (iterator != taskidToUuid.constEnd())
        {
            QString key = iterator.key();
            if (iterator.value().toString() == uuidPredicate)
            {
                return key;
            }
            iterator ++;
        }
        return QString(); // "null" string
    }

    static bool triggerExists(const QString& taskId)
    {
        QSettings settings;
        QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap(); // taskid->fs-uuid
        return taskidToUuid.contains(taskId);
    }

    // if there is a task->uuid mapping for the specific task it returns the uuid. Otherwise, empty string.
    static const QString triggerUuidForTask(const QString& taskid)
    {
        QSettings settings;
        QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap();
        return taskidToUuid.value(taskid, QString()).toString();
    }

    static void printTriggers()
    {
        QSettings settings;
        QMap<QString, QVariant> taskidToUuid = settings.value("triggersAsUuids").toMap(); // taskid->fs-uuid
        qDebug() << taskidToUuid;
    }

};

#endif // TRIGGERING_H
