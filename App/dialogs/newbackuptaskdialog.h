#ifndef NEWBACKUPTASKDIALOG_H
#define NEWBACKUPTASKDIALOG_H

#include "../appcontext.h"

#include <QDialog>

namespace Ui {
class NewBackupTaskDialog;
}
class TreeViewTasks;

class NewBackupTaskDialog : public QDialog
{
    Q_OBJECT

public:
    struct Result {
        QString name;
        QString id;
    } result;

    enum Mode {
        Wizard =0, // this also works as an index to the page of the stacked widget
        CreateOnly = 1,
        OpenOnly = 2
    } mode;

    explicit NewBackupTaskDialog(AppContext* appContext, QWidget *parent = nullptr, Mode pMode=Wizard);
    ~NewBackupTaskDialog();

signals:

    void wizardStepActivated(int stepIndex, int prevStepIndex); // prevStepIndex is -1 if there is no previous index (initialization)

private slots:

    void on_pushButtonCreate_clicked();

    void on_lineEditId_textChanged(const QString &arg1);

    void on_pushButtonBackFromCreate_clicked();

    void on_pushButtonCreateStep_clicked();

    void on_wizardStepActivated(int index, int prevIndex);

    void on_OpenTask();

    void on_pushButtonCancelFromCreate_clicked();

    void onCurrentTaskIs(QString taskName, const QModelIndex& modelIndex);

private:
    Ui::NewBackupTaskDialog *ui;
    QString getSelectedTaskId();

    QString selectedTask;
};

#endif // NEWBACKUPTASKDIALOG_H
