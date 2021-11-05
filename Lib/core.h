#ifndef CORE_H
#define CORE_H

#include <QDataStream>

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


public:
    SourceDetails(const BackupType psourceType = selective, const QString& ppredicate = nullptr, const BackupDepth pbackupDepth = rootOnly);
    ~SourceDetails();

    friend QDataStream& operator<<(QDataStream& s, const SourceDetails& item);
    friend QDataStream& operator>>(QDataStream& s, SourceDetails& item);
};




#endif // CORE_H
