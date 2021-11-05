#include "core.h"

#include <QDebug>

SourceDetails::SourceDetails(const BackupType psourceType, const QString& ppredicate, const BackupDepth pbackupDepth)
    : backupType(psourceType)
    , predicate(ppredicate) {
    qInfo() << "creating MyData - " << backupType;
}

SourceDetails::~SourceDetails() {
    qInfo() << "destroying MyData - ";
}

QDataStream& operator<<(QDataStream& s, const SourceDetails& item) {
    s << (qint32)item.backupType << (qint32)item.backupDepth << item.predicate;
    return s;
}

QDataStream& operator>>(QDataStream& s, SourceDetails& item) {
    qint32 temp;
    s >> temp;
    item.backupType = (SourceDetails::BackupType)temp;
    s >> temp;
    item.backupDepth = (SourceDetails::BackupDepth)temp;
    s >> item.predicate;
    return s;
}
