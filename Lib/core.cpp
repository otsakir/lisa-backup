#include "core.h"

#include <QDebug>

SourceDetails::SourceDetails() :
    backupType(all),
    backupDepth(rootOnly),
    predicateType(containsFilenameId) {}

SourceDetails::~SourceDetails() {
    qInfo() << "destroying MyData - ";
}

QDataStream& operator<<(QDataStream& s, const SourceDetails& item) {
    s << (qint32)item.backupType << (qint32)item.backupDepth << item.predicate << item.sourcePath << item.containsFilename << item.nameMatches << (qint32) item.predicateType;
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
    s >> item.containsFilename;
    s >> item.nameMatches;
    s >> temp;
    item.predicateType = (SourceDetails::PredicateType) temp;
    return s;
}

QDataStream& operator << (QDataStream& s, const BackupDetails& backupDetails) {
    s << backupDetails.backupName;
    s << backupDetails.systemdId;
    s << backupDetails.systemdMountUnit;
    s << backupDetails.destinationBasePath;
     s << backupDetails.destinationBaseSuffixPath;
    return s;
}

QDataStream& operator >> (QDataStream& s, BackupDetails& backupDetails) {
    s >> backupDetails.backupName;
    s >> backupDetails.systemdId;
    s >> backupDetails.systemdMountUnit;
    s >> backupDetails.destinationBasePath;
    s >> backupDetails.destinationBaseSuffixPath;
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
