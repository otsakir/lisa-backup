#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDataWidgetMapper>
#include <QItemSelection>
#include <QMainWindow>
#include <QStandardItemModel>
#include <systemdunitdialog.h>

#include <core.h>

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
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void methodChanged(int methodIndex); // signal raised when the backup method is altered between all/selective cases
    //void backupNameChanged(QString backupName); // signal raised when the backup name is changed. Be it set to another value or cleared altogether.
    void newBackupName(QString backupName); // there is a new backup name established!


private slots:
    void onNewBackupName(QString backupName);
    void on_pushButton_clicked();

    void on_updateSelection(const QItemSelection &selected, const QItemSelection &deselected);

    void on_currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void on_pushButton_2_clicked();

    void updateSourceDetailControls(const QModelIndex& current);

    SourceDetails* getSelectedSourceDetails();

    void on_removeSourceButton_clicked();

    void on_pushButtonSelectDevice_clicked();

    void on_pushButton_4_clicked();

    void on_comboBoxDepth_currentIndexChanged(int index);

    void on_radioButtonAll_toggled(bool checked);

    void on_radioButtonSelective_toggled(bool checked);

    void on_lineEditContainsFilename_editingFinished();

    void on_lineEditNameMatches_editingFinished();

    void updatePredicateTypeIndex(int index);

    void on_lineEditSystemdUnit_textChanged(const QString &arg1);

    void on_lineEditDestinationSuffixPath_textChanged(const QString &arg1);

    void on_toolButton_toggled(bool checked);

    //void on_lineEditBackupName_editingFinished();

    void on_lineEditDestinationSuffixPath_editingFinished();

    void on_pushButtonChooseDestinationSubdir_clicked();

    void on_activeBackupMethodChanged(int backupType);

    void on_action_New_triggered();

    void on_ButtonApply_clicked();

    void on_action_Open_triggered();

    void on_pushButtonRefreshBasePaths_clicked();

    void on_comboBoxBasePath_currentIndexChanged(const QString &arg1);

    void on_comboBoxBasePath_currentIndexChanged(int index);

    void on_action_Save_triggered();

    void on_lineEditBackupName_editingFinished();

    void on_lineEditBackupName_returnPressed();

    void on_pushButtonInstallTrigger_clicked();

    void on_pushButton_TestEdit_clicked();

    void on_pushButtonOk_clicked();

    void on_pushButtonRemoveTrigger_clicked();

private:
    Ui::MainWindow *ui;

    QStandardItemModel* sourcesModel;
    QDataWidgetMapper* sourcesDataMapper;
    BackupModel* activeBackup; // contains additional info about a backup except source stuff (i.e.like path, predicate, type etc.)
    Session session;
    State state; // generic application state. Not part of a backup.

    bool loadPersisted(QString backupName, BackupModel& persisted);
    void appendSource(SourceDetails* sourceDetails);
    void collectUIControls(BackupModel& persisted);
    void initUIControls(const BackupModel& persisted);

    void applyChanges();
    void refreshBasePaths(QString current);
    void enableMostUI(bool enable);
    void setupTriggerButtons(const QString& backupName);
};
#endif // MAINWINDOW_H
