
#include "utils.h"
#include "conf.h"

#include <QStandardPaths>
#include <QDir>
#include <QRandomGenerator>
#include <QStandardItem>
#include <QRegularExpression>
#include <QCoreApplication>

#include <QDebug>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include "globals.h"
#include <QDebug>


// stands for Lisa-Backup
namespace Lb {

// CONVENTION: directory paths don't include trailing slash

// /usr/share/lbackup - not needed currently
/*
QString appDir() {
    return "/usr/share/lbackup";
}
*/


QString appVersion() {
    return LBACKUP_VERSION;
}

// usr/share/lbackup/
QString appScriptsDir() {
#ifdef QT_DEBUG
    // return "/opt/lbackup";
    return QString("%1/scripts").arg(QCoreApplication::applicationDirPath());
#else
    //return "/usr/share/lbackup";
    return QString("%1/scripts").arg(QCoreApplication::applicationDirPath());
#endif
}

QString lbBinaryPath()
{
    return QString("%1/LisaBackup").arg(QCoreApplication::applicationDirPath());
}

QString lbLauncherScriptBinaryPath()
{
    return QString("%1/run-LisaBackup.sh").arg(QCoreApplication::applicationDirPath());
}

QString autoStartDesktopFilePath()
{
    return QString("%1/.config/autostart/lisa-backup.desktop").arg(homeDirectory());
}

// /home/{username}/.lbackup
QString dataDirectory() {
    QString tasksDirectory = QString("%1/.lbackup").arg(homeDirectory());
    if ( !Lb::Globals::tasksDirectory.isEmpty() )
    {
        QDir dir(Lb::Globals::tasksDirectory);
        if (dir.exists())
            tasksDirectory = Lb::Globals::tasksDirectory;
        else
            qWarning() << "Can't use tasks directory override " << Lb::Globals::tasksDirectory << ". Switching to default: " << tasksDirectory;
    }
    return tasksDirectory;
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


QString windowTitle(const QString& taskFriendlyName) {
    return QString("%1 - %2").arg(Lb::Globals::applicationName).arg(taskFriendlyName);
}

void setupDirs() {
    QDir dataDir(dataDirectory());
    if (!dataDir.exists())
        dataDir.mkpath(dataDirectory());
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
QString bestValidDirectoryMatch(const QString& rawpath) {
    if (rawpath.trimmed().isEmpty())
        return QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();

    QDir startDir(rawpath);
    QString path = startDir.absolutePath(); // remove costructs like "..", "." etc.
    QStringList pathParts = path.split("/");
    QString validPath;
    validPath.reserve(256);
    foreach (const QString& part, pathParts) {
        //qInfo() << "part: " << part;
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

    return validPath;
}

void createDesktopFile()
{
    // set up .desktop file content
    QString content("[Desktop Entry]\n\
Comment=Backup application for the Linux Desktop\n\
Exec=\"LB_BINARY_PATH\" -m\n\
Name=Lisa Backup\n\
Type=Application\n\
Version=0.5\n\
");
    content.replace("LB_BINARY_PATH", lbLauncherScriptBinaryPath() );

    // save .desktop file
    QFile desktopFile(autoStartDesktopFilePath());
    if (desktopFile.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&desktopFile);
        stream << content;
        stream.flush();
        desktopFile.close();
    }

}

void removeDesktopFile()
{
    QFile desktopFile(autoStartDesktopFilePath());
    desktopFile.remove();
}



} // Lb namespace
