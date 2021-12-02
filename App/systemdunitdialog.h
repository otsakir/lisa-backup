#ifndef SYSTEMDUNITDIALOG_H
#define SYSTEMDUNITDIALOG_H

#include <QDialog>
#include <QItemSelection>
#include <QStandardItemModel>

namespace Ui {
class SystemdUnitDialog;
}

struct DialogResult {
    QString mountId;
    QString mountPath;
    QString backupSubdir;
};

class SystemdUnitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SystemdUnitDialog(DialogResult& dialogResult, QWidget *parent = nullptr);
    ~SystemdUnitDialog();
protected:

private slots:
    void on_UpdateSelection(const QItemSelection& selected,const QItemSelection& dselected);

    void on_buttonOk_clicked();

    void on_buttonReload_clicked();

private:
    Ui::SystemdUnitDialog *ui;
    QStandardItemModel* systemdUnitsModel;
    DialogResult& dialogResult;

    void reloadMountUnits();
};

#endif // SYSTEMDUNITDIALOG_H
