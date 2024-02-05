#include <QDebug>
#include <QDir>
#include <QFile>
#include <QRegularExpression>

#include "utils.h"

#include "task.h"

TaskLoader::TaskLoader(QString path)
    : taskPath(path)
{}


void TaskLoader::refresh()
{
    qDebug() << "refreshing tasks";

    taskInfo.clear();
    QDir taskDir(taskPath);
    assert (taskDir.exists());

    QStringList filters;
    filters << "*.task";
    taskDir.setNameFilters(filters);
    taskDir.setFilter(QDir::Files);
    taskDir.setSorting(QDir::Time);

    QStringList entries = taskDir.entryList();
    BackupModel backupModel;
    for (int i = 0; i < entries.size(); i++) {
        QString taskFilename = entries.at(i); // returns "{id}.task"

        QString taskId = taskFilename.replace(QRegularExpression("\\.task$"),"");
        if (loadTask(taskId, backupModel)) {
            taskInfo << taskId;
        } else {
            qWarning() << QString("error processing task file '%1'").arg(taskFilename);
        }
    }
}

bool TaskLoader::loadTask(const QString taskId, BackupModel& persisted)
{

    QString backupFilename = QString("%1/%2.task").arg(taskPath, taskId);

    qDebug() << "[debug] loading task file: " << backupFilename;
    QFile ifile(backupFilename);
    if (ifile.open(QIODevice::ReadOnly)) {
        QDataStream istream(&ifile);
        // read and check header
        quint32 magic;
        istream >> magic;
        if (magic != (quint32)0x6C697361)  // l-i-s-a
            return false;
        // read version
        qint32 version;
        istream >> version; // we won't do something with it for now
        istream.setVersion(QDataStream::Qt_5_12);
        // read data
        istream >> persisted;
        ifile.close();
        return true;
    }
    return false;
}

const QList<QString>& TaskLoader::getTaskInfo()
{
    refresh();
    return taskInfo;
}


namespace Tasks
{

/*
bool loadTask(const QString taskId, BackupModel& persisted)
{

    QString backupFilename = Lb::taskFilePathFromName(taskId);

    qDebug() << "[debug] loading task file: " << backupFilename;
    QFile ifile(backupFilename);
    if (ifile.open(QIODevice::ReadOnly)) {
        QDataStream istream(&ifile);
        // read and check header
        quint32 magic;
        istream >> magic;
        if (magic != (quint32)0x6C697361)  // l-i-s-a
            return false;
        // read version
        qint32 version;
        istream >> version; // we won't do something with it for now
        istream.setVersion(QDataStream::Qt_5_12);
        // read data
        istream >> persisted;
        ifile.close();
        return true;
    }
    return false;
}
*/

// TODO - handle errors
/**
 * @brief saveTask
 *
 * Persist task data in 'persisted' to a task file on the disk
 *
 * @param taskId
 * @param persisted
 */
void saveTask(const QString taskId, const BackupModel& persisted)
{

    QString dataFilePath = Lb::taskFilePathFromName(taskId);
    qInfo() << "data file path: " << dataFilePath;
    QFile file(dataFilePath);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);

    // Write a header with a "magic number" and a version
    stream << (quint32)0x6C697361; // l-i-s-a
    stream << (qint32)1;
    stream.setVersion(QDataStream::Qt_5_12);

    stream << persisted;
    file.close();
}



bool deleteTask(const QString taskId)
{
    QString dataFilePath = Lb::taskFilePathFromName(taskId);
    QFile file(dataFilePath);

    return file.remove();
}

} // namespace Tasks
