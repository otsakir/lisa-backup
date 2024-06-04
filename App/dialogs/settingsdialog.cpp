#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QSettings>
#include <QSystemTrayIcon>
#include <QDebug>
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

    ui->checkBoxAutoStart->setChecked(settings.value(Settings::Keys::AutoStart).toInt());
    connect(ui->checkBoxAutoStart, &QCheckBox::stateChanged, [this] (const int value){
        this->settings.setValue(Settings::Keys::AutoStart, value);
        emit autoStartUpdate(value != 0);

    });

    // comboBoxLogging
    Settings::Loglevel loglevel = GET_INT_SETTING(Settings::Loglevel);
    ADD_COMBO_ITEM(ui->comboBoxLogging, Settings::Loglevel::All);
    ADD_COMBO_ITEM(ui->comboBoxLogging, Settings::Loglevel::Errors);

    ui->comboBoxLogging->setCurrentText(Settings::toCaption(loglevel));

    connect(ui->comboBoxLogging, QOverload<int>::of(&QComboBox::currentIndexChanged),[this](int index) {
        int value = this->ui->comboBoxLogging->currentData().toInt();
        this->settings.setValue(Settings::LoglevelKey, value);
    });

    // external file manager command
    ui->lineEditExternalFileManager->setText(settings.value(Settings::Keys::ExternalFileManagerCommand).toString());
    connect(ui->lineEditExternalFileManager, &QLineEdit::textEdited, [this] (const QString& newtext) {
        settings.setValue(Settings::Keys::ExternalFileManagerCommand, newtext);
    });
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::showEvent(QShowEvent *event)
{
    QWidget::showEvent( event );

    // hook up UI checkbox with settings and perform signal wiring (once, guarded)
    // evaluate ability to use system tray every time the dialog is shown. This helps avoid the issue of misleading isSystemTrayAvailable() result when Desktop boots.
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qWarning() << "I couldn't detect any system tray on this system. Minimize-to-tray won't be available.";
        ui->checkBoxKeepRunningInTray->setEnabled(false);
    } else
    {
        ui->checkBoxKeepRunningInTray->setChecked(settings.value(Settings::Keys::KeepRunningInTray).toInt());
        if (!runInTraySignalConnected)
        {
            connect(ui->checkBoxKeepRunningInTray, &QCheckBox::stateChanged, [this] (const int value){
                this->settings.setValue(Settings::Keys::KeepRunningInTray, value);
                emit trayIconUpdate(value != 0); // 1 or 2 means "shown". 0 means hide.
            });
            runInTraySignalConnected = true;
        }
    }
}

