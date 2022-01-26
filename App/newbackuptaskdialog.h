#ifndef NEWBACKUPTASKDIALOG_H
#define NEWBACKUPTASKDIALOG_H

#include <QDialog>

namespace Ui {
class NewBackupTaskDialog;
}

class NewBackupTaskDialog : public QDialog
{
    Q_OBJECT

public:
    struct Result {
        QString name;
        QString id;
    } result;

    explicit NewBackupTaskDialog(QWidget *parent = nullptr);
    ~NewBackupTaskDialog();

private slots:
    void on_pushButtonCreate_clicked();

    void on_lineEditId_textChanged(const QString &arg1);

    void on_pushButton_clicked();

private:
    Ui::NewBackupTaskDialog *ui;
};

#endif // NEWBACKUPTASKDIALOG_H
