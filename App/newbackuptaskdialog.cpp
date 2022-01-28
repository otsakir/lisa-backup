#include "newbackuptaskdialog.h"
#include "ui_newbackuptaskdialog.h"

#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QDebug>
#include <QDir>

#include <core.h>
#include <utils.h>

NewBackupTaskDialog::NewBackupTaskDialog(QWidget *parent, Mode pMode) :
    QDialog(parent),
    ui(new Ui::NewBackupTaskDialog)
{
    ui->setupUi(this);

    QObject::connect(this, &NewBackupTaskDialog::wizardStepActivated, this, &NewBackupTaskDialog::on_wizardStepActivated);

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
    //emit wizardStepActivated(mode, -1); // -1 because there is no previous step index
}

NewBackupTaskDialog::~NewBackupTaskDialog()
{
    delete ui;
}

void NewBackupTaskDialog::on_wizardStepActivated(int index, int prevIndex) {
    qInfo() << "wizardStepActivated:: " << index << " from " << prevIndex;
}

void NewBackupTaskDialog::on_pushButtonCreate_clicked()
{
    result.name = ui->lineEditName->text();
    result.id = ui->lineEditId->text();
    if (result.name.isEmpty())
        result.name = result.id;

    // store to task file
    BackupModel model;
    model.backupDetails.friendlyName = result.name;
    QString taskFilename = Lb::taskFilePathFromName(result.id);
    Lb::persistTaskModel(model, taskFilename);
    // TODO error handling

    this->accept();
}


void NewBackupTaskDialog::on_lineEditId_textChanged(const QString &arg1)
{
    ui->pushButtonCreate->setEnabled(arg1.length() >= 2); // have at least two chars
}

void NewBackupTaskDialog::on_pushButton_clicked()
{
    this->reject();
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
    qInfo() << "wizardStepActivated:: " << stepIndex;
    if (stepIndex == Mode::OpenOnly) {
        // when we move to "open" step, scan directory for tasks
        ui->listWidgetTasks->clear();
        QDir taskDir(Lb::dataDirectory());
        QStringList filters;
        filters << "*.task";
        taskDir.setNameFilters(filters);
        taskDir.setFilter(QDir::Files);
        taskDir.setSorting(QDir::Time);

        QStringList entries = taskDir.entryList();
        for (int i = 0; i < entries.size(); i++) {
            qInfo() << "entry " << entries.at(i);
            QString taskId = entries.at(i);
            taskId = taskId.replace(QRegularExpression("\\.task$"),"");
            ui->listWidgetTasks->addItem(taskId);
        }
    }
}


void NewBackupTaskDialog::on_pushButtonOpen_clicked()
{
    if (! ui->listWidgetTasks->selectedItems().empty()) {
        QListWidgetItem* currentItem = ui->listWidgetTasks->currentItem();
        qInfo() << "currentItem: " << currentItem;

        QString taskId =  currentItem->data(Qt::DisplayRole).toString();

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

