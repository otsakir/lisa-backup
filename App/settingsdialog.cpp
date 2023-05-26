#include "settingsdialog.h"
#include "qdebug.h"
#include "ui_settingsdialog.h"

#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->comboBoxTaskRunner->setCurrentText(settings.value("taskrunner/TaskRunner").toString());
    connect(ui->comboBoxTaskRunner, &QComboBox::currentTextChanged, [this] (const QString& value) {
        this->settings.setValue("taskrunner/TaskRunner", value);
    });

    ui->checkBoxGenerateScripts->setChecked(settings.value("taskrunner/GenerateBashScripts").toInt());
    connect(ui->checkBoxGenerateScripts, &QCheckBox::stateChanged, [this] (const int value) {
        this->settings.setValue("taskrunner/GenerateBashScripts", value);
    });

    ui->checkBoxShowConfirmation->setChecked(settings.value("taskrunner/ShowConfirmation").toInt());
    connect(ui->checkBoxShowConfirmation, &QCheckBox::stateChanged, [this] (const int value){
        this->settings.setValue("taskrunner/ShowConfirmation", value);
    });


}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

