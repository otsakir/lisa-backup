#include "newbackuptaskdialog.h"
#include "ui_newbackuptaskdialog.h"

NewBackupTaskDialog::NewBackupTaskDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewBackupTaskDialog)
{
    ui->setupUi(this);
}

NewBackupTaskDialog::~NewBackupTaskDialog()
{
    delete ui;
}

void NewBackupTaskDialog::on_pushButtonCreate_clicked()
{
    result.name = ui->lineEditName->text();
    result.id = ui->lineEditId->text();
    this->accept();
}

