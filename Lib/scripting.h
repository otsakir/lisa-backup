#ifndef SCRIPTING_H
#define SCRIPTING_H

#include <core.h>

namespace Scripting
{

bool buildBackupScript(QString taskId, const BackupModel& persisted);

bool removeBackupScript(QString taskId);

bool buildBackupCommands(const BackupModel& appstate, QVector<QString>& commands);

}

#endif // SCRIPTING_H
