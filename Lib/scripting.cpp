#include <core.h>

#include <QFile>
#include <QProcess>
#include <QTextStream>
#include <QDebug>

#include <utils.h>
#include <logging.h>


bool buildBackupCommands(const BackupModel& appstate, QVector<QString>& commands) {
    for (int i=0; i < appstate.allSourceDetails.size(); i++) {

        const SourceDetails& source = appstate.allSourceDetails.at(i);
        if (source.backupType == SourceDetails::all) {
            //rsync -avzh /sourcedir /destinationdir
            QString command;
            command.reserve(100);
            command.append("rsync -avzh \"");
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

                name = QString("'%1'").arg(source.nameMatches);
                getparent = false;
            } else if (source.predicateType == SourceDetails::containsFilenameId) {
                if (source.backupDepth == SourceDetails::directChildren) {
                    maxdepth = 2;
                    mindepth = 2;
                } else if (source.backupDepth == SourceDetails::recursive) {
                    maxdepth = -1; // do not set at all
                }

                name = QString("'%1'").arg(source.containsFilename);
                getparent = true;
            }

            QString find_command("        find ");
            find_command.append(QString("\"%1\"").arg(source.sourcePath)).append(" ");
            if (mindepth != -1)
                find_command.append("-mindepth ").append(QString::number(mindepth)).append(" ");
            if (maxdepth != -1)
                find_command.append("-maxdepth ").append(QString::number(maxdepth)).append(" ");
            // -name parameter
            find_command.append("-name ").append(name).append(" ");

            // get matched entry or its parent ?


            // 'find' report, null-terminated
            if (getparent)
                find_command.append("-printf \"%h\\0\" ");
            else
                find_command.append("-print0 ");

            // action
            if (source.actionType == SourceDetails::rsync) {
                //rsync copy
                find_command.append(" | xargs -0 -I files rsync -avzh files ").append(destinationRoot);
            } else if ( source.actionType == SourceDetails::gitBundle) {
                // git bundle
                find_command.append(QString(" | xargs -0 -I files %1/bundle-git-repo.sh files %2").arg(Lb::appScriptsDir()).arg(destinationRoot));
            }

            commands.append(find_command);
        }

    }
    return true;
}

bool generateBackupScript(QString scriptTemplate, QString outfilename, const BackupModel& appstate) {

    qInfo() << "[info] Generating " << outfilename << " backup script from " << scriptTemplate;

    // open script template file
    QFile f(scriptTemplate);
    if ( ! f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        qCritical("[error] script template file '%s' does not exist", qUtf8Printable(scriptTemplate) );
        return false;
    }

    // read file content
    QTextStream s(&f);
    QString content = s.readAll();
    f.close();

    // search and replace placeholders with actual content
    content.replace("$BACKUP_NAME", appstate.backupDetails.tmp.taskId);
    content.replace("$DEST_PATH", appstate.backupDetails.destinationBasePath + "/" + appstate.backupDetails.destinationBaseSuffixPath);
    QVector<QString> commands;
    buildBackupCommands(appstate, commands);
    QString commandsString;
    commandsString.reserve(100 * commands.size()); // assume an average of 100 bytes per command
    for (int i=0; i<commands.size(); i++)
        commandsString.append(commands.at(i)).append("\n");
    content.replace("$BACKUP_COMMANDS", commandsString);

    // create executable script
    //QString outfilename = outputDirectory + "/backup-" + appstate.backupDetails.backupName + ".sh";
    QFile outf(outfilename);
    if (! outf.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream outs(&outf);
    outs << content;

    if (outs.status() != QTextStream::Ok) {
        return false;
    }

    outf.setPermissions(QFile::ExeOwner | QFile::ReadOwner | QFile::WriteOwner);
    return true;
}

//// public implementation

namespace Scripting
{

bool buildBackupScript(QString taskId, const BackupModel& persisted)
{
    QString scriptName = Lb::backupScriptFilePath(taskId);
    return generateBackupScript( QString("%1/template/%2").arg(Lb::appScriptsDir(),"backup.sh.tmpl"), scriptName, persisted);
}

/**
 * @brief removeBackupScript
 * @param taskId
 * @return true or false
 *
 * Removes the respective backup script for the task. This is not always present. It
 * will return false only if tried to removed an existing file and it failed.
 */
bool removeBackupScript(QString taskId)
{
    QString scriptName = Lb::backupScriptFilePath(taskId);
    QFile f(scriptName);
    if (f.exists())
    {
        return f.remove();
    }
    return true;
}


} // Scripting namespace
