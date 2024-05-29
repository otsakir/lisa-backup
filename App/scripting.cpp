#include "core.h"

#include <QFile>
#include <QProcess>
#include <QDebug>
#include <QSettings>

#include "utils.h"
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
            QString destinationRoot = QString("%1").arg(appstate.backupDetails.destinationPath);
            //rsync -avzh /sourcedir /destinationdir
            QString command;
            command.reserve(100);
            command.append("\"").append("/" + appstate.backupDetails.destinationPath).append("\"");
            command = QString("%1/backup-one.sh -a %4 \"%2\" \"%3\"").arg(Lb::appScriptsDir()).arg(destinationRoot).arg(source.sourcePath).arg(SourceDetails::toScriptParam(source.actionType));

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
            QString destinationRoot = QString("\"%2/%3/\" ").arg(appstate.backupDetails.destinationPath, sourcePathLastDir);
            if (!destinationRoot.startsWith("\"/"))
                destinationRoot.insert(1, '/');
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
            find_command.append(QString("-name \"%1\" ! -path \"*/.*/%2\" ").arg(name).arg(name)); // exclude hidden directories from the search predicates


            // get matched entry or its parent ?


            // 'find' report, null-terminated
            if (getparent)
                find_command.append("-printf \"%h\\0\" ");
            else
                find_command.append("-print0 ");

            // action
            find_command.append(QString(" | xargs -0 -I files %1/backup-one.sh -a %3 %2 files").arg(Lb::appScriptsDir()).arg(destinationRoot).arg(SourceDetails::toScriptParam(source.actionType)));

            commands.append(find_command);
        }

    }
    return true;
}


} // Scripting namespace

