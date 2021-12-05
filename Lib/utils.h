#ifndef UTILS_H
#define UTILS_H

#include <QString>

namespace Lb {

QString dataDirectory();
QString configDirectory();
QString scriptsDirectory();
QString homeDirectory();

QString backupDataFilePath(const QString& backupName);
QString backupScriptFilePath(const QString& backupName);

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

QString randomString(unsigned int size);
}

#endif // UTILS_H
