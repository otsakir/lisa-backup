#ifndef UTILS_H
#define UTILS_H

#include <QProcess>
#include <QString>
#include "core.h"

namespace Lb {

QString appScriptsDir();    // application installation directory (typically /usr/share/lbackup). Contains install-systemd-hook.sh or bundle-git-repo.sh
QString dataDirectory();    // for .task  files and respective backup scripts (.sh)
QString configDirectory();
QString scriptsDirectory();
QString homeDirectory();
QString systemdDirectory(); // systemd service file directory (typically /etc/systemd/system/)
const QVector<QString>& excludedDevicePathPrefix(); // a set prefices to exclude systemd devices. It is checked against the device path.

QString taskFilePathFromName(const QString& backupName);
QString taskNameFromPath(const QString& taskFilePath);
QString backupScriptFilePath(const QString& backupName);

QString windowTitle(const QString& taskFriendlyName);

/*!
 * \brief setupDirs conditionally creates application directory structure
 */
void setupDirs();

/*!
 * \brief Returns the username of the user running the current process
 *
 * \note There should always be a username returned.
 * \return the username as a Qstring
 */
QString username();
QString userGroup();

QString runShellCommand(QString commandString);
void runScriptInWindow(QString scriptPath);

QString randomString(unsigned int size);

bool systemdUnitForMountPath(QString path, QString& systemdUnit);
void bestValidDirectoryMatch(const QString& rawpath, QString& validPath);
void startProcess(QProcess& process, const QString& program, const QStringList& arguments);

}

#endif // UTILS_H
