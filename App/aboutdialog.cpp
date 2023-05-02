#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "conf.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->label_2->setText(QString("Version: %1").arg(LBACKUP_VERSION));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_pushButton_clicked()
{
    this->accept();
}

