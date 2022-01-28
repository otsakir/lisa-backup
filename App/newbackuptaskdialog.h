#ifndef NEWBACKUPTASKDIALOG_H
#define NEWBACKUPTASKDIALOG_H

#include <QDialog>

namespace Ui {
class NewBackupTaskDialog;
}

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

    explicit NewBackupTaskDialog(QWidget *parent = nullptr, Mode pMode=Wizard);
    ~NewBackupTaskDialog();

signals:

    void wizardStepActivated(int stepIndex, int prevStepIndex); // prevStepIndex is -1 if there is no previous index (initialization)

private slots:
    void on_pushButtonCreate_clicked();

    void on_lineEditId_textChanged(const QString &arg1);

    void on_pushButton_clicked();

    void on_pushButtonBackFromOpen_clicked();

    void on_pushButtonBackFromCreate_clicked();

    void on_pushButtonCreateStep_clicked();

    void on_pushButtonOpenStep_clicked();

    // custom slots
    void on_wizardStepActivated(int index, int prevIndex);

    void on_stackedWidgetWizard_currentChanged(int arg1);

    void on_pushButtonOpen_clicked();

    void on_pushButtonCancelFromOpen_clicked();

    void on_pushButtonCancelFromCreate_clicked();

    void on_treeViewTasks_doubleClicked(const QModelIndex &index);

private:
    Ui::NewBackupTaskDialog *ui;
};

#endif // NEWBACKUPTASKDIALOG_H
