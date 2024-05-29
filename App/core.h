#ifndef CORE_H
#define CORE_H

#include <QDataStream>
#include <QVector>

/*!
 * \class MountedDevice
 * \brief Cached information for mounted devices. Also saved in settings 'triggersAsUuids/' per task.
 */
struct MountedDevice
{
    QString mountPoint;
    QString label;
    QString uuid; // dbus-provided UUID for mounted disk

    MountedDevice()
        : mountPoint(""), label(""), uuid("")
    {}

    MountedDevice(QString uuid)
        : MountedDevice()
    {
        this-> uuid = uuid;
    }

    MountedDevice(const MountedDevice& other)
    {
        this->uuid = other.uuid;
        this->label = other.label;
        this->mountPoint = other.mountPoint;
    }

    operator QString()
    {
        return QString("{mountPoint: %1, label: %2, uuid: %3}").arg(mountPoint, label, uuid);
    }

    void clear()
    {
        mountPoint.clear();
        label.clear();
        uuid.clear();
    }
};
Q_DECLARE_METATYPE(MountedDevice)

QDataStream &operator<<(QDataStream &out, const MountedDevice &myObj);

QDataStream &operator>>(QDataStream &in, MountedDevice &myObj);



/*!
 * \class SourceDetails
 * \brief Holds all details of a backup path.
 *
 * BackupType   Determines whether to everything under the directory
 *              as it is or use a selective approach.
 *
 *              all     take everything
 *              selective   use one or more criteria to decide which sub-directories
 *              to include in the bacup
 *
 *
 * \note The backup path itself is not included in the data structure.
 *
 * @
 */
class SourceDetails {
public:
    /*!
     * \enum BackupType
     * \brief Backup to approach. Pick all or some.
     *
     * Determines whether to take everything under the root path or select some of the
     * sub-directories using one or more criteria.
     *
     * \value   all
     *          Blind backup everything
     * \value   selective
     *          Apply a picking algorithm based on BackupDepth and Selection fields.
     */
    enum BackupType {
        all = 0,
        selective = 1 // i.e. select sub-directories
    } backupType; // how to backup each sub-dir entry

    enum BackupDepth {
        directChildren = 0,
        recursive = 1
    } backupDepth;

    enum PredicateType {
        containsFilenameId = 0,
        nameMatchesId = 1
    } predicateType;

    enum ActionType {
        rsync = 0,
        gitBundle = 1,
        automatic = 2 // let the backup script decide on the fly
    } actionType;

    static const QString toScriptParam(ActionType actionType) // converts ActionType to string to be passed to "backup-one.sh. -a XXX" command
    {
        switch (actionType)
        {
            case rsync: return "rsync";
            case gitBundle: return "gitbundle";
            case automatic: return "auto";
            default: assert(false);
        }
    }

    /*!
     * \variable SourceDetails::precicate
     * \brief Applies filtering criteria for subdir selection.
     *
     * Each sub-directory resulting from backupType and backupDepth will be run against
     * the _predicate_ to deterine if it will be included in the backup. Returns true to
     * mark inclusion and false otherwise.
     *
     * The predicate syntactically takes the form a bash command.
     */
    QString predicate;
    QString sourcePath;

    QString containsFilename; // match when _traversed_ directory contains a file like this (use bash shell patterns)
    QString nameMatches; // match when _traversed_ directory's name is like this (use bash shell pattern)

public:
    SourceDetails();


    ~SourceDetails();

    bool operator==(const SourceDetails& other) const {
        return (backupType == other.backupType) &&
                (backupDepth == other.backupDepth) &&
                (predicateType == other.predicateType) &&
                (actionType == other.actionType) &&
                (predicate == other.predicate) &&
                (sourcePath == other.sourcePath) &&
                (containsFilename == other.containsFilename) &&
                (nameMatches == other.nameMatches);
    }

    friend QDataStream& operator<<(QDataStream& s, const SourceDetails& item);
    friend QDataStream& operator>>(QDataStream& s, SourceDetails& item);
    //friend bool operator==(const SourceDetails& one, const SourceDetails& another);
};

// Model struct to keep everything related to a backup in a single data structure.
struct BackupDetails {

    struct Tmp { // temprary stuff not stored to disk but required as long as the applications runs. Per-task.
        QString taskId;   //  taskId. It's used to load the task but we have to keep it somewhere
        QString taskFilepath;
    } tmp;

    QString systemdId; // identifier part for the systemd service name
    //QString backupName; // identifier for task resources. systemd service, backup bash script etc.
    QString friendlyName; // user friendly name of the task
    QString destinationPath; // absolute path of destination diriectory

    BackupDetails() {}

    BackupDetails& operator=(const BackupDetails& from) {
        systemdId = from.systemdId;
        //backupName = from.backupName;
        tmp = from.tmp;
        friendlyName = from.friendlyName;
        destinationPath = from.destinationPath;

        return *this;
    }

    bool operator==(const BackupDetails& other) const {
        return (systemdId == other.systemdId) &&
                (friendlyName == other.friendlyName) &&
                (destinationPath == other.destinationPath);
    }

    BackupDetails(const BackupDetails& from) {
        systemdId = from.systemdId;
        //backupName = from.backupName;
        tmp = from.tmp;
        friendlyName = from.friendlyName;
        destinationPath = from.destinationPath;
    }

public:
    const QString &getDestinationBasePath() const;

};

// Data model used for readind/writing to disk. It doesn't allocate memory.

class BackupModel {
public:
    enum ValueType {
        // TODO - populates with all different types of data the model can have like backupType, backupDepth etc.
        unset,
        backupType,
        systemdMountUnit
    };


    BackupDetails backupDetails;
    QVector<SourceDetails> allSourceDetails;
    typedef int SourceDetailsIndex; // used for indexing allSourceDetails

    BackupModel() {
        allSourceDetails.reserve(10);
    };

    BackupModel& operator=(const BackupModel& other);
    bool operator==(const BackupModel& other) const;



    friend QDataStream& operator << (QDataStream& s, const BackupModel& pmodel);
    friend QDataStream& operator >> (QDataStream& s, BackupModel& pmodel);
};


// application operation state. Things that we don't want to load and check again and again
class State {
public:
    BackupModel modelCopy; // reflects the state of the application as it is persisted to storage. Helps to see if anything has changed.
    QString lastSystemdMountUnit; // last unit name we believe is used in the trigger i.e. unit name in a freshly loaded task or unit name used in a trigger just installed
    QList<MountedDevice> mountedDevices;
};




void registerQtMetatypes();


#endif // CORE_H
