#include <core.h>

#include <QFile>
#include <QProcess>
#include <QTextStream>
#include <QDebug>
#include <QSettings>

#include <utils.h>
#include <logging.h>
#include "settings.h"


bool generateBackupScript(QString scriptTemplate, QString outfilename, const BackupModel& appstate);

//// public implementation

namespace Scripting
{

bool buildBackupCommands(const BackupModel& appstate, QVector<QString>& commands)
{
    QSettings settings;
    Settings::Loglevel loglevel = GET_INT_SETTING(Settings::Loglevel);

    for (int i=0; i < appstate.allSourceDetails.size(); i++) {

        const SourceDetails& source = appstate.allSourceDetails.at(i);
        if (source.backupType == SourceDetails::all) {
            //rsync -avzh /sourcedir /destinationdir
            QString command;
            command.reserve(100);
            command.append(loglevel == Settings::Loglevel::All ? "rsync -avzh \"" : "rsync -azh \"");
            command.append(source.sourcePath).append("\" ");
            command.append("\"").append(appstate.backupDetails.destinationBasePath + "/" + appstate.backupDetails.destinationBaseSuffixPath).append("\"");

            commands.append(command);
        } else if (source.backupType == SourceDetails::selective) {
            /* nameMatches == 'dobackup' && directChildren
             * $ find /rootpath -maxdepth 1 -name 'filename' -exec rsync -avzh {} destinationPath
             * $ find ./Projects -maxdepth 1 -name "*radio*" -print0 | xargs -0 -I files rsync -avzh files /home/nando/tmp/backup/
             *
             * Backup 'selective' from /home/nando/music 'direct children' whose 'name matches' '*_user' -- -maxdepth=1  -name '*_user'
             * Backup 'selective' from /home/nando/Projects 'direct children' that 'contain file 'dobackup' --  -maxdepth=2 -mindepth=2 -name 'dobackup' -printf "%h\0" | xargs -0 -I files rsync -avzh files /home/nando/tmp/backup
             *
             * Backup 'selective' from /home/nando/Documents 'recursively' any directory that 'contains file' '.git'
             *   find /home/nando/Documents -mindepth 1  -name ".git" -printf "h\0 | xargs....
             *
             * - always add -mindepth=1 if not defined otherwise
             *
             *
             */
            QString command;
            command.reserve(100);

            // find last part of the source path
            QString sourcePathLastDir = Lb::lastDirInPath(source.sourcePath);

            int mindepth = 1; // by default, not match the command arguments theselves
            int maxdepth = -1; // my default, do not set at all
            bool getparent = false; // should 'find' report the matched entry or its parent
            QString name;
            // note the trailing slash after %3 below: it results in creating the source directory under the destination
            QString destinationRoot = QString("'%1/%2/%3/' ").arg(appstate.backupDetails.destinationBasePath, appstate.backupDetails.destinationBaseSuffixPath, sourcePathLastDir);
            if (source.predicateType == SourceDetails::nameMatchesId) {
                if (source.backupDepth == SourceDetails::directChildren) {
                    maxdepth = 1;
                } else if (source.backupDepth == SourceDetails::recursive) {
                    maxdepth = -1; // do not set at all (default)
                }

                name = source.nameMatches;
                getparent = false;
            } else if (source.predicateType == SourceDetails::containsFilenameId) {
                if (source.backupDepth == SourceDetails::directChildren) {
                    maxdepth = 2;
                    mindepth = 2;
                } else if (source.backupDepth == SourceDetails::recursive) {
                    maxdepth = -1; // do not set at all
                }

                name = source.containsFilename;
                getparent = true;
            }

            QString find_command("find ");
            find_command.append(QString("\"%1\"").arg(source.sourcePath)).append(" ");
            if (mindepth != -1)
                find_command.append("-mindepth ").append(QString::number(mindepth)).append(" ");
            if (maxdepth != -1)
                find_command.append("-maxdepth ").append(QString::number(maxdepth)).append(" ");
            // -name parameter
            find_command.append(QString("-name '%1' ! -path '*/.*/%2' ").arg(name).arg(name)); // exclude hidden directories from the search predicates


            // get matched entry or its parent ?


            // 'find' report, null-terminated
            if (getparent)
                find_command.append("-printf \"%h\\0\" ");
            else
                find_command.append("-print0 ");

            // action
            if (source.actionType == SourceDetails::rsync) {
                //rsync copy
                find_command.append(QString(" | xargs -0 -I files %1/backup-one.sh -a rsync %2 files").arg(Lb::appScriptsDir()).arg(destinationRoot));
            } else if ( source.actionType == SourceDetails::gitBundle) {
                // git bundle
                find_command.append(QString(" | xargs -0 -I files %1/backup-one.sh -a gitbundle %2 files").arg(Lb::appScriptsDir()).arg(destinationRoot));
            } else if (source.actionType == SourceDetails::automatic) {
                find_command.append(QString(" | xargs -0 -I files %1/backup-one.sh -a auto %2 files").arg(Lb::appScriptsDir()).arg(destinationRoot));
            }

            commands.append(find_command);
        }

    }
    return true;
}


} // Scripting namespace

