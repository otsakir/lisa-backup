#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

signals:
    void trayIconUpdate(bool show); // user updated icon tray show status
    void autoStartUpdate(bool autoStart); // user updated the "Auto start with Desktop" setting in the dialog.

private:
    Ui::SettingsDialog *ui;
    QSettings settings;

    virtual void showEvent(QShowEvent* event) override;
    bool runInTraySignalConnected = false;
};

#endif // SETTINGSDIALOG_H
