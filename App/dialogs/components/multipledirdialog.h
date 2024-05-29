#ifndef MULTIPLEDIRDIALOG_H
#define MULTIPLEDIRDIALOG_H

#include <QDialog>


namespace Ui {
class MultipleDirDialog;
}

class QFileSystemModel;

class MultipleDirDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MultipleDirDialog(QWidget *parent = nullptr);
    ~MultipleDirDialog();

    QStringList selectedPaths;

private slots:
    void on_selectPushButton_clicked();

private:
    Ui::MultipleDirDialog *ui;
    QFileSystemModel* fsModel;
};

#endif // MULTIPLEDIRDIALOG_H
