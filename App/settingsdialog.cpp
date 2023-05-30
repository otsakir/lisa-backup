#include "settingsdialog.h"
#include "qdebug.h"
#include "ui_settingsdialog.h"

#include <QSettings>
#include <settings.h>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->checkBoxGenerateScripts->setChecked(settings.value("taskrunner/GenerateBashScripts").toInt());
    connect(ui->checkBoxGenerateScripts, &QCheckBox::stateChanged, [this] (const int value) {
        this->settings.setValue("taskrunner/GenerateBashScripts", value);
    });

    ui->checkBoxShowConfirmation->setChecked(settings.value("taskrunner/ShowConfirmation").toInt());
    connect(ui->checkBoxShowConfirmation, &QCheckBox::stateChanged, [this] (const int value){
        this->settings.setValue("taskrunner/ShowConfirmation", value);
    });

    // comboBoxLogging
    Settings::Loglevel loglevel = GET_INT_SETTING(Settings::Loglevel);
    ADD_COMBO_ITEM(ui->comboBoxLogging, Settings::Loglevel::All);
    ADD_COMBO_ITEM(ui->comboBoxLogging, Settings::Loglevel::Errors);

    ui->comboBoxLogging->setCurrentText(Settings::toCaption(loglevel));

    connect(ui->comboBoxLogging, QOverload<int>::of(&QComboBox::currentIndexChanged),[this](int index) {
        int value = ui->comboBoxLogging->currentData().toInt();
        this->settings.setValue(Settings::LoglevelKey, value);
    });

    // comboBoxTaskRunner
    Settings::Taskrunner taskrunnerMode = GET_INT_SETTING(Settings::Taskrunner);
    ADD_COMBO_ITEM(ui->comboBoxTaskRunner, Settings::Taskrunner::Gui);
    ADD_COMBO_ITEM(ui->comboBoxTaskRunner, Settings::Taskrunner::Script);
    ui->comboBoxTaskRunner->setCurrentText(Settings::toCaption(taskrunnerMode));

    connect(ui->comboBoxTaskRunner, QOverload<int>::of(&QComboBox::currentIndexChanged),[this](int index) {
        int value = ui->comboBoxTaskRunner->currentData().toInt();
        this->settings.setValue(Settings::TaskrunnerKey, value);
    });


}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

