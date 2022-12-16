#include <QFile>

#include "core.h"
#include "utils.h"
#include "terminal.h"

namespace Systemd
{

int installHook(const BackupDetails& backup)
{
    QString backupScriptPath = Lb::backupScriptFilePath(backup.tmp.name);
    QString commandLine = QString("%1/%2 install -s %3 -u \"%4\" \"%5\"").arg(Lb::appScriptsDir(),"install-systemd-hook.sh",backup.tmp.name, backup.systemdMountUnit, backupScriptPath);
    return Terminal::runCommandInTerminal(commandLine);
}

int removeHook(const BackupDetails &backup)
{
    QString commandLine = QString("%1/%2 remove -s %3").arg(Lb::appScriptsDir(),"install-systemd-hook.sh",backup.tmp.name);
    return Terminal::runCommandInTerminal(commandLine);
    //startProcess(process, "xterm", {"-e", QString("%1/%2").arg(appScriptsDir(),"install-systemd-hook.sh"),"remove","-s", backup.tmp.name});

}

/**
 * @brief Checks if systemd service is inplace
 * @param backupName The name of the backup as entered by the user
 * @return true if systemd service if present, false otherwise
 */
bool hookPresent(const QString& backupName)
{
    QString path = Lb::systemdDirectory();
    path.reserve(128);
    path.append("/").append("lbackup-").append(backupName).append(".service");

    QFile serviceFile(path);
    return serviceFile.exists();
}

} // Systemd namespace
