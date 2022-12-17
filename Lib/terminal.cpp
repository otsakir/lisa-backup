#include <QFile>
#include <QTemporaryFile>
#include <QProcess>
#include <QDebug>


namespace Terminal
{


// process creation with logging
void startProcess(QProcess& process, const QString& program, const QStringList& arguments) {
    qDebug() << "[debug] running external process " << program << "with arguments: " << arguments;
    process.start(program, arguments);
}

void runScriptInWindow(QString scriptPath)
{
    QProcess process;
    //process.startDetached("xterm", {"-e", "/home/nando/tmp/s.sh"});

    //QString backupName = "backup1.sh";
    //QString backupScriptPath = Lb::backupScriptFilePath(backup.backupName);

    //process.startDetached("xterm", {"-e", scriptPath});
    startProcess(process, "xterm", {"-e", scriptPath});
    process.waitForFinished(-1);
}

// run a bash command and return what's written to stdout
QString runShellCommand(QString commandString)
{
    QProcess process;
    //process.start("bash", {"-c", "systemctl list-units --type=mount | grep mounted > a"});

    //process.start("bash", {"-c", commandString});
    startProcess(process, "bash", {"-c", commandString});

    process.waitForFinished(-1);

    QString out = process.readAllStandardOutput();
    return out;
}

// return command status code or -1 in case of other error. 0 for success.
int runCommandInTerminal(QString commandLine)
{
    QProcess process;

    QTemporaryFile exitStatusFile; // temporary file to keep xterm child process exit status code
    if (exitStatusFile.open()) {
        exitStatusFile.close();
        qInfo() << "exitStatusFile: " << exitStatusFile.fileName();
        startProcess(process, "xterm", {"-e", "bash", "-c", QString("%1 ; echo $? > %2").arg(commandLine,exitStatusFile.fileName())});
        process.waitForFinished(-1);
        // check exitStatusFile content for exit code of the process
        if (exitStatusFile.open()) {
            QTextStream in(&exitStatusFile);
            int status;
            in >> status;
            qInfo() << "child process status: " << status;
            return status;
        }
    }
    return -1; // error (not the one returned by child process)
}

} // Terminal namespace


