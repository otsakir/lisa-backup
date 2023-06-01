#ifndef SCRIPTING_H
#define SCRIPTING_H

#include <core.h>

namespace Scripting
{

bool buildBackupCommands(const BackupModel& appstate, QVector<QString>& commands);

}

#endif // SCRIPTING_H
