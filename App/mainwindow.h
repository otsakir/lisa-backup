#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDataWidgetMapper>
#include <QItemSelection>
#include <QMainWindow>
#include <QStandardItemModel>

#include <QProcess>
#include "core.h"
#include "dbusutils.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Forward (abstract) type declarations
struct BackupDetails;
class BackupModel;
class SourceDetails;
class Session;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString taskName, QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void methodChanged(int methodIndex); // raised when the backup method UI control is updated. Helps to chain actions to update the ui state (hide/show other controls etc.)
    void actionChanged(SourceDetails::ActionType action);
    void newBackupName(QString backupName); // there is a new backup name established!
    void PleaseQuit();
    void friendlyNameEdited(); // there is new content in activeBackup.backupDetails.friendlyName
    void systemdUnitChanged(QString unitName); // raised when the contents of the systemd lineedit control have been modified
    void modelUpdated(BackupModel::ValueType valueType = BackupModel::unset); // any change in the model triggers this
    void sourceChanged(const QModelIndex &current); //selected backup source changed, got initialized or got zero

private slots:

    // custom slots
    void on_actionChanged(SourceDetails::ActionType action);
    void onNewBackupName(QString backupName);
    void onModelUpdated(BackupModel::ValueType valueType);

    void updateSourceDetailControls(const QModelIndex& current);

    SourceDetails* getSelectedSourceDetails();

    void on_removeSourceButton_clicked();

    void on_comboBoxDepth_currentIndexChanged(int index);

    void on_radioButtonAll_toggled(bool checked);

    void on_radioButtonSelective_toggled(bool checked);

    void updatePredicateTypeIndex(int index);

    void on_lineEditDestinationSuffixPath_textChanged(const QString &arg1);

    void checkLineEditDestinationSuffixPath();

    void on_activeBackupMethodChanged(int backupType);

    void on_action_New_triggered();

    void on_action_Open_triggered();

    void on_pushButtonRefreshBasePaths_clicked();

    void on_comboBoxBasePath_currentIndexChanged(int index);

    void on_action_Save_triggered();

    void on_actionDelete_triggered();

    void newBackupTaskFromDialog(qint32 dialogMode);

    void on_radioButtonRsync_toggled(bool checked);

    void on_radioButtonGitBundle_toggled(bool checked);

    void on_actionAbout_triggered();

    void runActiveTask();

    void on_pushButtonAdd_clicked();

    void on_radioButtonAuto_toggled(bool checked);

    void on_lineEditContainsFilename_textEdited(const QString &arg1);

    void on_lineEditNameMatches_textEdited(const QString &arg1);

    void on_toolButtonSourceUp_clicked();

    void on_toolButtonSourceDown_clicked();

    void on_actionSe_ttings_triggered();

    void handleMounted(const QString& label, const QString& uuid);

protected:
    virtual void closeEvent (QCloseEvent *event);

private:
    Ui::MainWindow *ui;

    QStandardItemModel* sourcesModel;
    QDataWidgetMapper* sourcesDataMapper;
    BackupModel* activeBackup; // contains additional info about a backup except source stuff (i.e.like path, predicate, type etc.)
    State state; // generic application state. Not part of a backup.
    QString taskName;
    bool newBackupTaskDialogShown = false;
    QProcess consoleProcess;
    DbusUtils dbusUtils;

    bool openTask(QString taskId);
    bool loadPersisted(QString backupName, BackupModel& persisted);
    QStandardItem* appendSource(BackupModel::SourceDetailsIndex sourceDetails);
    void collectUIControls(BackupModel& persisted);
    void initUIControls(BackupModel& persisted);
    void swapSources(BackupModel::SourceDetailsIndex source1, BackupModel::SourceDetailsIndex source2); // change the order two sources

    int checkSave(); // returns QMessageBox::X status or -1
    void applyChanges();
    void appendBaseBath(const QString mountPath, const QString uuid, const QString label, const QString caption);
    void refreshBasePaths(const QString& current);

    void consoleProcessStarted();
    void consoleProcessDataAvail();
    void consoleProcessFinished(int exitCode);

    void printCombo();

    virtual void showEvent(QShowEvent* event) override;


private slots:
    void afterWindowShown();
    void onCheckBoxMountTriggerClicked(int status);
    void on_pushButtonChooseDestinationSubdir_clicked();
    void on_groupBoxTriggering_clicked(bool checked);
};
#endif // MAINWINDOW_H
