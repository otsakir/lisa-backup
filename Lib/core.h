#ifndef CORE_H
#define CORE_H

#include <QDataStream>
#include <QVector>

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
        selective = 1
    } backupType; // how to backup each sub-dir entry

    enum BackupDepth {
        rootOnly = 0,
        directChildren = 1
    } backupDepth;

    enum PredicateType {
        containsFilenameId = 0,
        nameMatchesId = 1
    } predicateType;

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
    //SourceDetails(const BackupType psourceType = all, const QString& ppredicate = nullptr, const BackupDepth pbackupDepth = rootOnly);
    SourceDetails();


    ~SourceDetails();

    friend QDataStream& operator<<(QDataStream& s, const SourceDetails& item);
    friend QDataStream& operator>>(QDataStream& s, SourceDetails& item);
};

// Model struct to keep everything related to a backup in a single data structure.
struct BackupDetails {

    QString systemdId; // identifier part for the systemd service name
    QString backupName;
    QString systemdMountUnit; // e.g.  media-username-label
    QString destinationBasePath; // root directory of the inserted medium when mounted
    QString destinationBaseSuffixPath; // together with 'destinationBasePath' it form the destination directory for the backup

    BackupDetails() {}

    BackupDetails& operator=(const BackupDetails& from) {
        systemdId = from.systemdId;
        backupName = from.backupName;
        systemdMountUnit = from.systemdMountUnit;
        destinationBasePath = from.destinationBasePath;
        destinationBaseSuffixPath = from.destinationBaseSuffixPath;

        return *this;
    }

    BackupDetails(const BackupDetails& from) {
        systemdId = from.systemdId;
        backupName = from.backupName;
        systemdMountUnit = from.systemdMountUnit;
        destinationBasePath = from.destinationBasePath;
        destinationBaseSuffixPath = from.destinationBaseSuffixPath;
    }

public:
    const QString &getDestinationBasePath() const;

private:
    Q_PROPERTY(QString destinationBasePath READ getDestinationBasePath CONSTANT)
};

// Data model used for readind/writing to disk. It doesn't allocate memory.

class PersistenceModel {
public:
    BackupDetails backupDetails;
    QVector<SourceDetails> allSourceDetails;

    PersistenceModel() {};

    friend QDataStream& operator << (QDataStream& s, const PersistenceModel& pmodel);
    friend QDataStream& operator >> (QDataStream& s, PersistenceModel& pmodel);
};




#endif // CORE_H
