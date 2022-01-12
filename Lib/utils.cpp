
#include "utils.h"

#include <QStandardPaths>
#include <QProcessEnvironment>
#include <QDir>
#include <QRandomGenerator>
#include <QStandardItem>


#include <QDebug>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>


// stands for Lisa-Backup
namespace Lb {

// CONVENTION: directory paths don't include trailing slash

// narrow down options
QString dataDirectory() {
    QStringList pathlist = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    return pathlist.first();
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

// TODO - fix fallback logic (?)
QString scriptsDirectory() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString homepath = env.value("HOME");
    if (homepath.isEmpty())
        return QString("/tmp/lisa-backup-scripts"); // fallback to temp-directory
    else
        return QString("%1/.lbackup").arg(homepath);
}

// generates the full path to the backup data file based on the 'backup name'
QString backupDataFilePath(const QString& backupName) {
    return dataDirectory() + "/" + backupName + ".data";
}

QString backupScriptFilePath(const QString& backupName) {
    return scriptsDirectory() + "/backup-" + backupName + ".sh";
}

void setupDirs() {
    QString datadir = dataDirectory();

    QDir dataDir(datadir);
    if (!dataDir.exists())
        dataDir.mkpath(datadir);

    assert( dataDir.exists() );

    if (!dataDir.exists("templates"))
        assert(dataDir.mkdir("templates"));
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

// run a bash command and return what's written to stdout
QString runShellCommand(QString commandString) {
    QProcess process;
    //process.start("bash", {"-c", "systemctl list-units --type=mount | grep mounted > a"});
    process.start("bash", {"-c", commandString});
    process.waitForFinished(-1);

    QString out = process.readAllStandardOutput();
    return out;
}

void runScriptInWindow(QString scriptPath) {
    QProcess process;
    //process.startDetached("xterm", {"-e", "/home/nando/tmp/s.sh"});

    //QString backupName = "backup1.sh";
    //QString backupScriptPath = Lb::backupScriptFilePath(backup.backupName);

    process.startDetached("xterm", {"-e", scriptPath});
    process.waitForFinished(-1);
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
    QString mountUnitsString = Lb::runShellCommand("systemctl list-units --type=mount | grep 'loaded active mounted' | sed -e 's/^\\s*//' -e 's/\\.mount\\s\\s*loaded active mounted\\s/.mount||/'");
    if (!mountUnitsString.isEmpty()) {
        QStringList lines = mountUnitsString.split("\n", QString::SkipEmptyParts);
        //qInfo() << "lines: " << lines << "\n";

        for (int i = 0; i<lines.size(); i++) {
            QString line = lines.at(i);
            QStringList unitinfo = line.split("||", QString::SkipEmptyParts);
            //qInfo() << unitinfo;

            if ( unitinfo.size() == 2) {
                if (unitinfo[1] == path) {
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

namespace Triggers {

    void installSystemdHook(const BackupDetails& backup) {
        QProcess process;
        //process.startDetached("xterm", {"-e", "/home/nando/tmp/s.sh"});

        QString backupName = "backup1.sh";
        QString backupScriptPath = Lb::backupScriptFilePath(backup.backupName);

        process.startDetached("xterm", {"-e", "/opt/lbackup/install-systemd-hook.sh","install","-s", backup.backupName, "-u", backup.systemdMountUnit, backupScriptPath});
        process.waitForFinished(-1);
    }

    void removeSystemdHook(const BackupDetails &backup) {
        QProcess process;

        process.startDetached("xterm", {"-e", "/opt/lbackup/install-systemd-hook.sh","remove","-s", backup.backupName});
        process.waitForFinished(-1);
    }

    /**
     * @brief Checks if systemd service is inplace
     * @param backupName The name of the backup as entered by the user
     * @return true if systemd service if present, false otherwise
     */
    bool systemdHookPresent(const QString& backupName) {
        QString path = systemdDirectory();
        path.reserve(128);
        path.append("/").append("lbackup-").append(backupName).append(".service");

        QFile serviceFile(path);
        return serviceFile.exists();
    }

}



} // Lb namespace
