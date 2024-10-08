#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>



#include <QProcess>
#include "../core.h"
#include "../common.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Forward (abstract) type declarations
struct BackupDetails;
class BackupModel;
class SourceDetails;
class Session;
class TaskLoader;
class AppContext;
class TriggeringComboBox;
class TaskManager;
class SettingsDialog;
class SourceDetailsView;
class QSystemTrayIcon;
class QStandardItemModel;
class QStandardItem;
class ListViewSources;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(bool startInTray, QString openingTaskName, AppContext* appContext,  QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void PleaseQuit(); // graceful quit signal
    void gotDirty(bool dirty);
    void gotClean(bool dirty);
    void sourceChanged(const QModelIndex &current); //selected backup source changed, got initialized or got zero
    void taskSaved(const QString taskId); // a task was saved to disk; state.modelCopy model has been updated.
    void taskNowEdited(const QString taskId); // when a new task get open in the editor throw this signal

    void newTaskCreated(const QString taskId);

private slots:

    void onModelUpdated(); // any change in the model of the selected task will trigger this
    void onDirtyChanged(bool isdirty);

    void removeSelectedSourceFromList();
    void updateSourceDetails(QModelIndex rowIndex);
    void moveSourceItemUp();
    void moveSourceItemDown();

    // destination path
    void updatetDestinationPathModel(const QString &arg1);
    void openDestinationDirExternal();

    void checkLineEditDestinationPath(const QString& newText);

    void showAboutDialog();
    void showSettingsDialog();
    void createNewTaskDialog(); // Initiates new task workflow. Asks user for id, validates, creates task file.

    void _triggerEntrySelected(MountedDevice newTriggerEntry);

    void on_action_New_triggered();


public slots:
    void editTask(const QString& taskid);

protected:
    virtual void closeEvent (QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;

    QStandardItemModel* sourcesModel;
    BackupModel* activeBackup = nullptr; // contains additional info about a backup except source stuff (i.e.like path, predicate, type etc.)
    State state; // generic application state. Not part of a backup.
    bool newBackupTaskDialogShown = false;
    bool dirty = false;

    TaskLoader* taskLoader;
    AppContext* appContext;
    TaskManager* taskManager;
    SettingsDialog* settingsDialog;

    TriggeringComboBox* triggeringCombo;
    SourceDetailsView* sourceDetails;
    ListViewSources* sourcesListView;

    // tray icon
    QAction *restoreAction;
    QAction *quitAction;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;


    bool openTask(QString taskId);
    bool loadPersisted(QString backupName, BackupModel& persisted);
    QStandardItem* appendSource(BackupModel::SourceDetailsIndex sourceDetails);
    void collectUIControls(BackupModel& persisted);
    void initUIControls(BackupModel& persisted);
    void swapSources(BackupModel::SourceDetailsIndex source1, BackupModel::SourceDetailsIndex source2); // change the order two sources

    int checkSave(); // returns QMessageBox::X status or -1
    void checkSaveAndRun(const QString taskname, Common::TaskRunnerReason reason, bool show); // check if task edited before running it
    void applyChanges();
    void appendBaseBath(const QString mountPath, const QString uuid, const QString label, const QString caption);

    virtual void showEvent(QShowEvent* event) override;
    // tray icon
    void createTrayIcon();
    void showTrayIcon(bool show);
    bool trayIconShown();

    void initButtonIcons();
    void trivialWiring();

    QString taskName(); // name of task currently edited or empty string there is none
    QString suggestDestinationPath(); // suggest a destination path for the backup based on triggering setting and existing path
private slots:
    void afterWindowShown();
    void on_pushButtonChooseDestinationSubdir_clicked();
    void askUserAndAddSources();
    void refreshTriggerEntries();
};
#endif // MAINWINDOW_H
