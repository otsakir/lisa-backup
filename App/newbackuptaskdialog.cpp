#include "newbackuptaskdialog.h"
#include "ui_newbackuptaskdialog.h"

#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDebug>
#include <QDir>
#include <QStandardItem>
#include <QMessageBox>

#include <core.h>
#include <utils.h>
#include <task.h>
#include <systemd.h>
#include <terminal.h>
#include <scripting.h>

NewBackupTaskDialog::NewBackupTaskDialog(QWidget *parent, Mode pMode) :
    QDialog(parent),
    ui(new Ui::NewBackupTaskDialog)
{
    ui->setupUi(this);

    ui->pushButtonBackFromOpen->setIcon(QIcon(":/custom-icons/chevron-left.svg"));
    ui->pushButtonBackFromCreate->setIcon(QIcon(":/custom-icons/chevron-left.svg"));
    ui->pushButtonDeleteTask->setIcon(QIcon(":/custom-icons/trash.svg"));
    //ui->pushButtonOpen->setIcon(QIcon(":/custom-icons/chevron-right.svg"));

    QStandardItemModel* model = new QStandardItemModel(0,2,this);
    ui->treeViewTasks->setModel(model);
    ui->treeViewTasks->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QObject::connect(this, &NewBackupTaskDialog::wizardStepActivated, this, &NewBackupTaskDialog::on_wizardStepActivated);
    QObject::connect(ui->treeViewTasks, &QTreeView::clicked, this, &NewBackupTaskDialog::taskClicked);

    mode = pMode;

    QRegularExpression* re = new QRegularExpression("^[a-zA-Z_][a-zA-Z0-9_-]+$"); // TODO - check memory releast with breakpoint on destructor
    QRegularExpressionValidator* validator = new QRegularExpressionValidator(*re, ui->lineEditId); // TODO - check memory release with breakpoint on destructor
    ui->lineEditId->setValidator(validator);

    ui->stackedWidgetWizard->setCurrentIndex(mode);

    // assume all buttons enabled
    ui->pushButtonCancelFromCreate->setVisible(mode != Wizard);
    ui->pushButtonCancelFromOpen->setVisible(mode != Wizard);
    ui->pushButtonBackFromCreate->setVisible(mode == Wizard);
    ui->pushButtonBackFromOpen->setVisible(mode == Wizard);

    ui->pushButtonOpen->setDisabled(true); // nothing selected

    if (mode == Wizard) {
        this->setWindowTitle("Backup task wizard");
    } else if (mode == OpenOnly) {
        this->setWindowTitle("Open task");
    } else if (mode == CreateOnly) {
        this->setWindowTitle("Start new task");
    } else {
        this->setWindowTitle("Hmmm...");
    }

    emit ui->lineEditId->textChanged(ui->lineEditId->text()); // initialize state
    emit ui->stackedWidgetWizard->currentChanged(mode);
}

QString NewBackupTaskDialog::getSelectedTaskId()
{
    QStandardItemModel* model = (QStandardItemModel*) ui->treeViewTasks->model();
    QModelIndex index = ui->treeViewTasks->currentIndex(); //.siblingAtColumn(1);
    if (index.isValid())
    {
        QString taskId = model->data(index).toString();
        return taskId;
    }
    return QString();
}


void NewBackupTaskDialog::taskClicked(const QModelIndex& index) {
    ui->pushButtonDeleteTask->setDisabled( ! index.isValid());
    ui->pushButtonOpen->setDisabled( ! index.isValid());
}


NewBackupTaskDialog::~NewBackupTaskDialog()
{
    delete ui;
}

void NewBackupTaskDialog::on_wizardStepActivated(int index, int prevIndex) {
}

void NewBackupTaskDialog::on_pushButtonCreate_clicked()
{
    //result.name = ui->lineEditName->text();
    result.id = ui->lineEditId->text();
    if (result.name.isEmpty())
        result.name = result.id;

    // store to task file
    BackupModel model;
    model.backupDetails.friendlyName = result.name;
    Tasks::saveTask(result.id, model);
    // TODO error handling

    this->accept();
}


void NewBackupTaskDialog::on_lineEditId_textChanged(const QString &arg1)
{
    ui->pushButtonCreate->setEnabled(arg1.length() >= 2); // have at least two chars
}

void NewBackupTaskDialog::on_pushButtonBackFromOpen_clicked()
{
    ui->stackedWidgetWizard->setCurrentIndex(0);
}


void NewBackupTaskDialog::on_pushButtonBackFromCreate_clicked()
{
    ui->stackedWidgetWizard->setCurrentIndex(0);
}


void NewBackupTaskDialog::on_pushButtonCreateStep_clicked()
{
    ui->stackedWidgetWizard->setCurrentIndex(1);
}


void NewBackupTaskDialog::on_pushButtonOpenStep_clicked()
{
    ui->stackedWidgetWizard->setCurrentIndex(2);
}


void NewBackupTaskDialog::on_stackedWidgetWizard_currentChanged(int stepIndex)
{
    //qInfo() << "wizardStepActivated:: " << stepIndex;
    if (stepIndex == Mode::OpenOnly) {
        // when we move to "open" step, scan directory for tasks
        //ui->listWidgetTasks->clear();
        QDir taskDir(Lb::dataDirectory());
        QStringList filters;
        filters << "*.task";
        taskDir.setNameFilters(filters);
        taskDir.setFilter(QDir::Files);
        taskDir.setSorting(QDir::Time);

        QStringList entries = taskDir.entryList();
        QStandardItemModel* model = (QStandardItemModel*) ui->treeViewTasks->model(); // listViewTasks->model();
        BackupModel backupModel;
        model->clear();
        //model->setHorizontalHeaderItem(0, new QStandardItem("Name"));
        model->setHorizontalHeaderItem(0, new QStandardItem("Id"));
        for (int i = 0; i < entries.size(); i++) {
            //qInfo() << "entry " << entries.at(i);
            QString taskId = entries.at(i); // returns "{id}.task"

            taskId = taskId.replace(QRegularExpression("\\.task$"),"");
            if (Tasks::loadTask(taskId, backupModel)) {
                QList<QStandardItem*> rowItems;
                rowItems << /*new QStandardItem(backupModel.backupDetails.friendlyName) << */ new QStandardItem(taskId);
                model->appendRow(rowItems);
            } else {
                // TODO - log error
            }
        }

        ui->pushButtonCreate->setDefault(false);
        ui->pushButtonCreate->setAutoDefault(false);
        ui->pushButtonOpen->setDefault(true);
        ui->pushButtonOpen->setAutoDefault(true);

        ui->pushButtonDeleteTask->setDisabled(true);

    } else if (stepIndex == Mode::CreateOnly) {
        ui->pushButtonOpen->setDefault(false);
        ui->pushButtonOpen->setAutoDefault(false);
        ui->pushButtonCreate->setDefault(true);
        ui->pushButtonCreate->setAutoDefault(true);
    }
}


void NewBackupTaskDialog::on_pushButtonOpen_clicked()
{
    QString taskId = getSelectedTaskId();
    if (!taskId.isEmpty())
    {
        this->result.id = taskId;
        this->accept();
    }
}


void NewBackupTaskDialog::on_pushButtonCancelFromOpen_clicked()
{
    this->reject();
}


void NewBackupTaskDialog::on_pushButtonCancelFromCreate_clicked()
{
    this->reject();
}


void NewBackupTaskDialog::on_treeViewTasks_doubleClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        QString taskId = ui->treeViewTasks->model()->data(index).toString(); //index.siblingAtColumn(1)).toString();
        //qInfo() << "selectedItem: " << taskId;
        this->result.id = taskId;
        this->accept();
    }
}


void NewBackupTaskDialog::on_pushButtonDeleteTask_clicked()
{
    QStandardItemModel* model = (QStandardItemModel*) ui->treeViewTasks->model();
    QModelIndex index = ui->treeViewTasks->currentIndex(); //).siblingAtColumn(1);

    QString taskId;
    if (!index.isValid())
        return;

    taskId = model->data(index).toString();

    if (! taskId.isEmpty())
    {
        // first, check if there is a systemd service attached
        if (Systemd::hookPresent(taskId))
        {
            // ask, the user before starting xterm fuss
            int ret = QMessageBox::warning(this, "Trigger active",tr("This backup task has an active trigger that should be removed.\n Should it be removed now ?"),QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
            if (ret == QMessageBox::Yes)
            {
                Systemd::removeHook(taskId); // TODO - error handling
            } else
            if (ret == QMessageBox::Cancel)
            {
                return;
            }
        }
        // The hook was either removed or was chosen not to be removed. Moving on with the rest of the removal steps.
        if (! Scripting::removeBackupScript(taskId))
        {
            qWarning() << "[warning] couldn't remove backup script for task '" << taskId << "'. I'll move on.";
        }
        // finally, remove the task itself
        Tasks::deleteTask(taskId); // TODO - error handling ? where ?
        // and the entry from the table/tree
        model->removeRow(index.row());

        taskClicked(QModelIndex()); // pass invalid index to trigger disabling the "Delete" button
    }

}

