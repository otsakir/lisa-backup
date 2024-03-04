#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDataWidgetMapper>
#include <QItemSelection>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QSystemTrayIcon>

#include <QProcess>
#include "../core.h"

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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString openingTaskName, AppContext* appContext,  QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void actionChanged(SourceDetails::ActionType action);
    void PleaseQuit(); // graceful quit signal
    void friendlyNameEdited(); // there is new content in activeBackup.backupDetails.friendlyName
    void systemdUnitChanged(QString unitName); // raised when the contents of the systemd lineedit control have been modified
    void dirtyChanged(bool isdirty); // "dirty" status of model changed
    void sourceChanged(const QModelIndex &current); //selected backup source changed, got initialized or got zero
    void taskSaved(const QString taskId); // a task was saved to disk; state.modelCopy model has been updated.

    void newTaskCreated(const QString taskId);

private slots:

    void onModelUpdated(); // any change in the model of the selected task will trigger this

    void onDirtyChanged(bool isdirty);

    void on_removeSourceButton_clicked();

    void on_lineEditDestinationSuffixPath_textChanged(const QString &arg1);

    void checkLineEditDestinationSuffixPath(const QString& newText);

    void on_action_Save_triggered();

    void newBackupTaskFromDialog(qint32 dialogMode);

    void on_actionAbout_triggered();

    void on_toolButtonSourceUp_clicked();

    void on_toolButtonSourceDown_clicked();

    void on_actionSe_ttings_triggered();

    void _triggerEntrySelected(MountedDevice newTriggerEntry);

    void on_action_New_triggered();

    void createNewTask(); // Initiates new task workflow. Asks user for id, validates, creates task file.

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
    void applyChanges();
    void appendBaseBath(const QString mountPath, const QString uuid, const QString label, const QString caption);

    virtual void showEvent(QShowEvent* event) override;
    // tray icon
    void createTrayIcon();
    void showTrayIcon(bool show);
    bool trayIconShown();

    void initButtonIcons();
    QString taskName();

private slots:
    void afterWindowShown();
    void on_pushButtonChooseDestinationSubdir_clicked();
    void askUserAndAddSources();
};
#endif // MAINWINDOW_H
