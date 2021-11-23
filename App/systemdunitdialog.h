#ifndef SYSTEMDUNITDIALOG_H
#define SYSTEMDUNITDIALOG_H

#include <QDialog>
#include <QItemSelection>
#include <QStandardItemModel>

namespace Ui {
class SystemdUnitDialog;
}

class SystemdUnitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SystemdUnitDialog(QString& pstringResult, QWidget *parent = nullptr);
    ~SystemdUnitDialog();
protected:

private slots:
    void on_UpdateSelection(const QItemSelection& selected,const QItemSelection& dselected);

    void on_buttonOk_clicked();

    void on_buttonReload_clicked();

private:
    Ui::SystemdUnitDialog *ui;
    QStandardItemModel* systemdUnitsModel;
    QString& stringResult;

    void reloadMountUnits();
};

#endif // SYSTEMDUNITDIALOG_H
