#include "settingsdialog.h"
#include "qdebug.h"
#include "ui_settingsdialog.h"

#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->comboBoxTaskRunner->setCurrentText(settings.value("TaskRunner").toString());
    connect(ui->comboBoxTaskRunner, &QComboBox::currentTextChanged, [this] (const QString& value) {
        qDebug() << "combo: current text changed to " << value;
        this->settings.setValue("TaskRunner", value);
    });

    ui->checkBoxGenerateScripts->setChecked(settings.value("GenerateBashScripts").toInt());
    connect(ui->checkBoxGenerateScripts, &QCheckBox::stateChanged, [this] (const int value) {
        qDebug() << "checkbox: setting state to " << value;
        this->settings.setValue("GenerateBashScripts", value);
    });

    ui->checkBoxShowConfirmation->setChecked(settings.value("ShowConfirmation").toInt());
    connect(ui->checkBoxShowConfirmation, &QCheckBox::stateChanged, [this] (const int value){
        this->settings.setValue("ShowConfirmation", value);
    });


}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

