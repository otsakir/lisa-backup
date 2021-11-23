#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDataWidgetMapper>
#include <QItemSelection>
#include <QMainWindow>
#include <QStandardItemModel>
#include <systemdunitdialog.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_lineEdit_2_editingFinished();

    void on_updateSelection(const QItemSelection &selected, const QItemSelection &deselected);

    void on_currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void on_pushButton_2_clicked();

    void updateSourceDetailControls(const QModelIndex& current);

    void on_removeSourceButton_clicked();

    void on_pushButton_3_clicked();

    void on_pushButtonSelectDevice_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::MainWindow *ui;

    QStandardItemModel* sourcesModel;
    QDataWidgetMapper* sourcesDataMapper;
};
#endif // MAINWINDOW_H
