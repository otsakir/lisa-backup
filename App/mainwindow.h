#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDataWidgetMapper>
#include <QItemSelection>
#include <QMainWindow>
#include <QStandardItemModel>
#include <systemdunitdialog.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Forward (abstract) type declarations
class BackupDetails;
class PersistenceModel;
class SourceDetails;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_lineEdit_2_editingFinished();

    void on_updateSelection(const QItemSelection &selected, const QItemSelection &deselected);

    void on_currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void on_pushButton_2_clicked();

    void updateSourceDetailControls(const QModelIndex& current);

    SourceDetails* getSelectedSourceDetails();

    void on_removeSourceButton_clicked();

    void on_pushButton_3_clicked();

    void on_pushButtonSelectDevice_clicked();

    void on_pushButton_4_clicked();

    void on_pushButtonLoad_clicked();

    void on_comboBoxDepth_currentIndexChanged(int index);

    void on_radioButtonAll_toggled(bool checked);

    void on_radioButtonSelective_toggled(bool checked);

    void on_lineEditContainsFilename_editingFinished();

    void on_lineEditNameMatches_editingFinished();

    //void on_comboBoxPredicate_currentIndexChanged(int index);

    void updatePredicateTypeIndex(int index);

    void on_lineEditSystemdUnit_textChanged(const QString &arg1);

    void on_lineEditDestinationBasePath_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;

    QStandardItemModel* sourcesModel;
    QDataWidgetMapper* sourcesDataMapper;
    BackupDetails* backupDetails; // contains additional info about a backup except source stuff (i.e.like path, predicate, type etc.)

    bool loadPersisted(PersistenceModel& persisted);
    void appendSource(SourceDetails* sourceDetails);
    void collectAppData(PersistenceModel& persisted);
    void initAppData(const PersistenceModel& persisted);

};
#endif // MAINWINDOW_H
