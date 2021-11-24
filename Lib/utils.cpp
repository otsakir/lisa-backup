#include <utils.h>

#include <QStandardPaths>
#include <QProcessEnvironment>
#include <QDir>
#include <QRandomGenerator>


#include <QDebug>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>


// stands for Lisa-Backup
namespace Lb {


// narrow down options
QString dataDirectory() {
    QStringList pathlist = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    return pathlist.first();
}

QString configDirectory() {
    QStringList pathlist = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
    return pathlist.first();
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



}
