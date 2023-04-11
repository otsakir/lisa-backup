
#include "utils.h"
#include "terminal.h"

#include <QStandardPaths>
#include <QDir>
#include <QRandomGenerator>
#include <QStandardItem>
#include <QRegularExpression>


#include <QDebug>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>


// stands for Lisa-Backup
namespace Lb {

// CONVENTION: directory paths don't include trailing slash

// /usr/share/lbackup - not needed currently
/*
QString appDir() {
    return "/usr/share/lbackup";
}
*/

// usr/share/lbackup/
QString appScriptsDir() {
#ifdef QT_DEBUG
    return "/opt/lbackup";
#else
    return "/usr/share/lbackup";
#endif
}

// /home/{username}/.lbackup
QString dataDirectory() {
    return QString("%1/.lbackup").arg(homeDirectory());
}

// holds backup task scripts - /home/{username}/.lbackup/scripts
QString scriptsDirectory() {
    return QString("%1/scripts").arg(dataDirectory());
}

QString configDirectory() {
    QStringList pathlist = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    return pathlist.first();
}

QString homeDirectory() {
    QStringList pathlist = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    return pathlist.first();
}

QString systemdDirectory() {
    return "/etc/systemd/system"; // TODO make this parametric from installation
}

const QVector<QString>& excludedDevicePathPrefix()
{
    static QVector<QString> prefixes = {"/snap","/run"};

    return prefixes;
}

// generates the full path to the backup data file based on the 'backup name'
QString taskFilePathFromName(const QString& backupName) {
    return dataDirectory() + "/" + backupName + ".task";
}

QString taskNameFromPath(const QString& taskFilePath) {
    QRegularExpression re(".*/([^/]+)\\.task$");
    QRegularExpressionMatch match = re.match(taskFilePath);
    if (match.hasMatch()) {
        //qInfo() << "matched" << match.captured(1);
        return match.captured(1);
    }

    return QString(); // nothing matched
}

// from '/home/alice/Documents' returns 'Documents'
QString lastDirInPath(const QString& path)
{
    QRegularExpression re(".*/([^/]+)$");
    QRegularExpressionMatch match = re.match(path);
    if (match.hasMatch()) {
        return match.captured(1);
    }

    return QString(); // nothing matched
}

QString backupScriptFilePath(const QString& backupName) {
    return scriptsDirectory() + "/backup-" + backupName + ".sh";
}

QString windowTitle(const QString& taskFriendlyName) {
    return QString("Lisa Backup - %1").arg(taskFriendlyName);
}

void setupDirs() {
    QDir dataDir(scriptsDirectory());
    if (!dataDir.exists())
        dataDir.mkpath(scriptsDirectory());
    assert( dataDir.exists() );
}


//
// ### Linux specific ###
//

struct UserInfo {
    int userid;
    int groupid;
    QString username;
    QString group;
};

void fillUserInfo(UserInfo& userInfo) {
    // get userid
    uid_t uid = getuid();

    char buf[1033];
    struct passwd pwd;
    struct passwd* ppwd = &pwd;
    getpwuid_r(uid, &pwd, buf, 1033, &ppwd);

    struct group* grp = getgrgid(pwd.pw_gid);

    userInfo.userid = pwd.pw_uid;
    userInfo.username = pwd.pw_name;
    userInfo.groupid = pwd.pw_gid;
    userInfo.group = grp->gr_name;
}

QString username() {
    UserInfo userInfo;
    fillUserInfo(userInfo);
    return userInfo.username;
}

QString userGroup() {
    UserInfo userInfo;
    fillUserInfo(userInfo);
    return userInfo.group;
}

QString randomString(unsigned int size) {
#define MAX_RANDOM_STRING 64
    assert (size <= MAX_RANDOM_STRING);

    char buffer[MAX_RANDOM_STRING+1]; // account for null terminating character
    const char oneofthese[] = "abcdefghizklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

    unsigned int i = 0;
    for (i = 0; i < size; i++ ) {
        buffer[i] = oneofthese[QRandomGenerator::global()->bounded((int) sizeof(oneofthese)-1)];
    }
    buffer[i] = 0;
    return QString(buffer);
}

// give it a path where a device is mounted and it will try to return the respective systemd unit it
bool systemdUnitForMountPath(QString path, QString& systemdUnit) {
    QString mountUnitsString = Terminal::runShellCommand("systemctl list-units --type=mount | grep 'loaded active mounted' | sed -e 's/^\\s*//' -e 's/\\.mount\\s\\s*loaded active mounted\\s/.mount||/'");
    if (!mountUnitsString.isEmpty()) {
        QStringList lines = mountUnitsString.split("\n", QString::SkipEmptyParts);
        //qInfo() << "lines: " << lines << "\n";

        for (int i = 0; i<lines.size(); i++) {
            QString line = lines.at(i);
            QStringList unitinfo = line.split("||", QString::SkipEmptyParts);
            //qInfo() << unitinfo;

            if ( unitinfo.size() == 2) {
                if (unitinfo[1].trimmed() == path) {
                    //qInfo() << "found match: " << unitinfo[1] << unitinfo[0];
                    //result = unitinfo[0];
                    systemdUnit = unitinfo[0];
                    return true;
                }
            }
        }
    }
    return false;
}


/**
 * Path validator
 *
 * validates a path one piece at a time. Updates validPath with
 * the largest valid path found.
 *
 * Example
 *
 *     bestValidDirectoryMatch("/home/valid/path/with some trailing/crap /in/it",validPath)
 *
 * Here, validPath will be filled with "/home/valid/path/".
 *
 * The trailing slash in the returned value is important. It's present for directories
 * and missing for (valid) files.
 */
void bestValidDirectoryMatch(const QString& rawpath, QString& validPath) {
    QDir startDir(rawpath);
    QString path = startDir.absolutePath(); // remove costructs like "..", "." etc.

    QStringList pathParts = path.split("/");
    validPath.clear();
    //QString validPath = "";
    validPath.reserve(256);
    foreach (const QString& part, pathParts) {
        qInfo() << "part: " << part;
        if (!part.isEmpty()) {
            QString testpath = validPath + part;
            QFileInfo checkDir(testpath);
            if (checkDir.exists()) {
                    validPath.append(part);
                    if (checkDir.isDir())
                        validPath.append("/");
            }

        } else
            validPath.append("/");
    }
}



} // Lb namespace
