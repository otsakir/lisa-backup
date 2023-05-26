#include "newbackuptaskdialog.h"
#include "ui_newbackuptaskdialog.h"

#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDebug>
#include <QDir>
#include <QStandardItem>
#include <QMessageBox>
#include <treeviewtasks.h>

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

    TreeViewTasks* tasks = new TreeViewTasks(this);

    QObject::connect(this, &NewBackupTaskDialog::wizardStepActivated, this, &NewBackupTaskDialog::on_wizardStepActivated);
    connect(tasks, &TreeViewTasks::currentTaskIs, this, &NewBackupTaskDialog::onCurrentTaskIs);
    connect(ui->pushButtonDeleteTask, &QPushButton::clicked, tasks, &TreeViewTasks::removeCurrent);
    connect(tasks, &TreeViewTasks::doubleClicked,this, &NewBackupTaskDialog::on_OpenTask);
    connect(ui->pushButtonOpen, &QPushButton::clicked, this, &NewBackupTaskDialog::on_OpenTask);

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

    static_cast<QHBoxLayout*>(ui->page3Open->layout())->insertWidget(0, tasks);
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


void NewBackupTaskDialog::on_OpenTask()
{
    assert(!selectedTask.isEmpty());
    this->result.id = selectedTask;
    this->accept();
}


void NewBackupTaskDialog::on_pushButtonCancelFromOpen_clicked()
{
    this->reject();
}


void NewBackupTaskDialog::on_pushButtonCancelFromCreate_clicked()
{
    this->reject();
}


// when the current task is updated
void NewBackupTaskDialog::onCurrentTaskIs(QString taskName, const QModelIndex& modelIndex)
{
    bool valid = modelIndex.isValid();
    ui->pushButtonOpen->setEnabled(valid);
    ui->pushButtonDeleteTask->setEnabled(valid);
    selectedTask = taskName;
}

