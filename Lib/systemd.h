#ifndef SYSTEMD_H
#define SYSTEMD_H

#include "core.h"

namespace Systemd
{

int installHook(const BackupDetails& backup);

int removeHook(QString taskId);

bool hookPresent(const QString& backupName);

} // Systemd namespace

#endif // SYSTEMD_H