#include "settingsdialog.h"
#include "qdebug.h"
#include "ui_settingsdialog.h"

#include <QSettings>
#include "../settings.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->checkBoxShowConfirmation->setChecked(settings.value(Settings::Keys::TaskrunnerConfirm).toInt());
    connect(ui->checkBoxShowConfirmation, &QCheckBox::stateChanged, [this] (const int value){
        this->settings.setValue(Settings::Keys::TaskrunnerConfirm, value);
    });
    ui->checkBoxShowRunnerPopup->setChecked(settings.value(Settings::Keys::TaskrunnerShowDialog).toInt());
    connect(ui->checkBoxShowRunnerPopup, &QCheckBox::stateChanged, [this] (const int value){
        this->settings.setValue(Settings::Keys::TaskrunnerShowDialog, value);
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
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

