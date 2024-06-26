#include "core.h"

#include <QDebug>
#include <QFile>



SourceDetails::SourceDetails() :
    backupType(all),
    backupDepth(directChildren),
    predicateType(containsFilenameId),
    actionType(rsync) {}

SourceDetails::~SourceDetails() {
}

bool BackupModel::operator==(const BackupModel& other) const {
    return (allSourceDetails == other.allSourceDetails) &&
            (backupDetails == other.backupDetails);

}

BackupModel& BackupModel::operator=(const BackupModel& other) {
    backupDetails = other.backupDetails;
    allSourceDetails = other.allSourceDetails;
    allSourceDetails.detach();
    return *this;
}

QDataStream& operator<<(QDataStream& s, const SourceDetails& item) {
    s << (qint32)item.backupType
      << (qint32)item.backupDepth
      << item.predicate
      << item.sourcePath
      << item.containsFilename
      << item.nameMatches
      << (qint32) item.predicateType
      << (qint32) item.actionType;
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
    s >> temp;
    item.actionType = (SourceDetails::ActionType) temp;
    return s;
}

QDataStream& operator << (QDataStream& s, const BackupDetails& backupDetails) {
    //s << backupDetails.backupName;  // skip this. We treat it separately
    s << backupDetails.friendlyName;
    s << backupDetails.systemdId;
    s << backupDetails.destinationPath;
    return s;
}

QDataStream& operator >> (QDataStream& s, BackupDetails& backupDetails) {
    //s >> backupDetails.backupName; // skip this. We treat it separately
    s >> backupDetails.friendlyName;
    s >> backupDetails.systemdId;
    s >> backupDetails.destinationPath;
    return s;
}

QDataStream& operator << (QDataStream& s, const BackupModel& pmodel) {
    s << pmodel.backupDetails;
    s << pmodel.allSourceDetails;
    return s;
}

QDataStream& operator >> (QDataStream& s, BackupModel& pmodel) {
    s >> pmodel.backupDetails;
    s >> pmodel.allSourceDetails;
    return s;
}

QDataStream &operator<<(QDataStream &out, const MountedDevice &triggerSettingsEntry) {
    out << triggerSettingsEntry.uuid;
    out << triggerSettingsEntry.label;
    out << triggerSettingsEntry.mountPoint;

    return out;
}

QDataStream &operator>>(QDataStream &in, MountedDevice &triggerSettingsEntry) {
    in >> triggerSettingsEntry.uuid;
    in >> triggerSettingsEntry.label;
    in >> triggerSettingsEntry.mountPoint;
    return in;
}



void registerQtMetatypes() {
    qRegisterMetaTypeStreamOperators<MountedDevice>("MountedDevice");
}
