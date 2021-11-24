#include "core.h"

#include <QDebug>

SourceDetails::SourceDetails(const BackupType psourceType, const QString& ppredicate, const BackupDepth pbackupDepth)
    : backupType(psourceType)
    , backupDepth(pbackupDepth)
    , predicate(ppredicate) {
    qInfo() << "creating MyData - " << backupType;
}

SourceDetails::~SourceDetails() {
    qInfo() << "destroying MyData - ";
}

QDataStream& operator<<(QDataStream& s, const SourceDetails& item) {
    s << (qint32)item.backupType << (qint32)item.backupDepth << item.predicate << item.sourcePath;
    return s;
}

QDataStream& operator>>(QDataStream& s, SourceDetails& item) {
    qint32 temp;
    s >> temp;
    item.backupType = (SourceDetails::BackupType)temp;
    s >> temp;
    item.backupDepth = (SourceDetails::BackupDepth)temp;
    s >> item.predicate;
    s >> item.sourcePath;
    return s;
}

QDataStream& operator << (QDataStream& s, const BackupDetails& backupDetails) {
    s << backupDetails.backupName;
    s << backupDetails.systemdId;
    return s;
}

QDataStream& operator >> (QDataStream& s, BackupDetails& backupDetails) {
    s >> backupDetails.backupName;
    s >> backupDetails.systemdId;
    return s;
}

QDataStream& operator << (QDataStream& s, const PersistenceModel& pmodel) {
    s << pmodel.backupDetails;
    s << pmodel.allSourceDetails;
    return s;
}

QDataStream& operator >> (QDataStream& s, PersistenceModel& pmodel) {
    s >> pmodel.backupDetails;
    s >> pmodel.allSourceDetails;
    return s;
}
