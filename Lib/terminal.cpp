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
    startProcess(process, "xterm", {"-e", scriptPath});
    process.waitForFinished(-1);
}

// run a bash command and return what's written to stdout
int runShellCommand(QString commandString, QString* pout)
{
    QProcess process;
    //process.start("bash", {"-c", commandString});
    process.setProcessChannelMode(QProcess::MergedChannels);
    startProcess(process, "bash", {"-c", commandString});

    process.waitForFinished(-1);

    if (pout != nullptr)
        *pout = process.readAll();

    return process.exitCode();
}

// return child command status code or -1 in case of other error. 0 for success.
int runCommandInTerminal(QString commandLine)
{
    QProcess process;
    QTemporaryFile exitStatusFile; // temporary file to keep xterm child process exit status code
    if (exitStatusFile.open()) {
        exitStatusFile.close();
        qDebug() << "exitStatusFile: " << exitStatusFile.fileName();
        startProcess(process, "xterm", {"-e", "bash", "-c", QString("%1 ; echo $? > %2").arg(commandLine,exitStatusFile.fileName())});
        process.waitForFinished(-1);
        int exitcode = process.exitCode();
        if (exitcode != 0)
        {
            qCritical() << "error running command" << process.program() << ":" << process.error() << "-" << process.exitCode();
            return -1;
        }

        // check exitStatusFile content for exit code of the process
        if (exitStatusFile.open()) {
            QTextStream in(&exitStatusFile);
            int status;
            in >> status;
            if (status != 0)
                qCritical() << "error running child process: " << status;
            return status;
        }
    }
    return -1; // error (not the one returned by child process)
}


} // Terminal namespace


