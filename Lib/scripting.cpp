#include <core.h>

#include <QFile>
#include <QProcess>
#include <QTextStream>

namespace Lb {

void buildBackupCommands(const PersistenceModel& appstate, QVector<QString>& commands) {
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
        }
    }
    return;
}

bool generateBackupScript(QString scriptTemplate, QString outputDirectory, const PersistenceModel& appstate) {

    // open script template file
    QFile f(scriptTemplate);
    if ( ! f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        return false;
    }

    // read file content
    QTextStream s(&f);
    QString content = s.readAll();
    f.close();

    // search and replace placeholders with actual content
    content.replace("$BACKUP_NAME", appstate.backupDetails.backupName);
    QVector<QString> commands;
    buildBackupCommands(appstate, commands);
    QString commandsString;
    commandsString.reserve(100 * commands.size()); // assume an average of 100 bytes per command
    for (int i=0; i<commands.size(); i++)
        commandsString.append(commands.at(i)).append("\n");
    content.replace("$BACKUP_COMMANDS", commandsString);

    // create executable script
    QString outfilename = outputDirectory + "/backup-" + appstate.backupDetails.systemdId + ".sh";
    QFile outf(outfilename);
    if (! outf.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream outs(&outf);
    outs << content;

    return (outs.status() == QTextStream::Ok);
}

}
