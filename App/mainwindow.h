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
class PersistenceModel;
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

private slots:
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

    void on_lineEditBackupName_editingFinished();

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

private:
    Ui::MainWindow *ui;

    QStandardItemModel* sourcesModel;
    QDataWidgetMapper* sourcesDataMapper;
    BackupDetails* activeBackup; // contains additional info about a backup except source stuff (i.e.like path, predicate, type etc.)
    Session session;

    bool loadPersisted(QString backupName, PersistenceModel& persisted);
    void appendSource(SourceDetails* sourceDetails);
    void collectAppData(PersistenceModel& persisted);
    void initAppData(const PersistenceModel& persisted);

    void applyChanges();
    void refreshBasePaths(QString current);
};
#endif // MAINWINDOW_H
