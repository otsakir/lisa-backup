#include <QDebug>
#include <QFile>

#include "utils.h"


bool loadTask(const QString taskId, BackupModel& persisted) {

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

// TODO - handle errors
void saveTask(const QString taskId, const BackupModel& persisted) {

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


