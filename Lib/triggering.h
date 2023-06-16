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
        qDebug() << "trigger map (in enable): " << taskidToUuid;
        taskidToUuid.value(taskId, QVariant(uuidPredicate));
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

};

#endif // TRIGGERING_H
