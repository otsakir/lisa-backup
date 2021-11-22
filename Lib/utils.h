#ifndef UTILS_H
#define UTILS_H

#include <QString>

namespace Lb {

QString dataDirectory();
QString configDirectory();
QString scriptsDirectory();

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

}

#endif // UTILS_H
