#include <QFile>

#include "core.h"
#include "utils.h"
#include "terminal.h"
#include <QSettings>

namespace Systemd
{

int installHook(const BackupDetails& backup)
{
    QSettings settings;
    QString appPath = settings.value("ApplicationFilePath").toString();

    QString backupScriptPath = Lb::backupScriptFilePath(backup.tmp.taskId);
    QString commandLine = QString("%1/%2 install -s %3 -u \"%4\" \"\\\"%5\\\" -r %3\"").arg(Lb::appScriptsDir(),"install-systemd-hook.sh",backup.tmp.taskId, backup.systemdMountUnit, appPath);
    return Terminal::runCommandInTerminal(commandLine);
}

int removeHook(QString taskId)
{
    QString commandLine = QString("%1/%2 remove -s %3").arg(Lb::appScriptsDir(),"install-systemd-hook.sh",taskId);
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
