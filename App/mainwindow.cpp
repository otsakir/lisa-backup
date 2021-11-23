#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "systemdunitdialog.h"
#include "utils.h"

#include <core.h>

#include <memory>

#include <QDebug>

#include <QFileDialog>

#include <QProcess>
#include <QStandardPaths>

Q_DECLARE_METATYPE(std::shared_ptr<SourceDetails>)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sourcesDataMapper(new QDataWidgetMapper(this))
{
    ui->setupUi(this);

    sourcesModel = new QStandardItemModel(0,2, this);
    ui->sourcesListView->setModel(sourcesModel);

    /*QStandardItem* modelItem = new QStandardItem("test");
    QList<QStandardItem*> itemList;
    itemList << modelItem;
    sourcesModel->appendRow(itemList);
*/
    sourcesDataMapper->setModel(sourcesModel);
    //sourcesDataMapper->addMapping(ui->lineEdit_2, 0);

    QItemSelectionModel* selectionModel = ui->sourcesListView->selectionModel();
    QObject::connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &MainWindow::on_updateSelection );

    QObject::connect(selectionModel, &QItemSelectionModel::currentRowChanged, this, &MainWindow::on_currentChanged);

    QObject::connect(ui->comboBoxPredicate, QOverload<int>::of(&QComboBox::activated), ui->stackedWidgetPredicate, &QStackedWidget::setCurrentIndex);

    ui->groupBoxSourceDetails->setHidden( ui->sourcesListView->selectionModel()->selection().empty() );

    qInfo() << "Data directory: " << Lb::dataDirectory();

    Lb::setupDirs();

}

MainWindow::~MainWindow()
{
    delete sourcesModel;
    delete ui;
}



void MainWindow::on_pushButton_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOptions(QFileDialog::ShowDirsOnly);

    QStringList selected;
    if (dialog.exec()) {
        selected= dialog.selectedFiles();
    }

    for (int i=0; i<selected.size(); i++) {
        QStandardItem* modelItem = new QStandardItem(selected.at(i));
        QList<QStandardItem*> itemList;

        //QVariant var("cRaP!");
        QVariant var;
        var.setValue( std::shared_ptr<SourceDetails>(new SourceDetails(SourceDetails::selective, "[[ -f dobackup ]]")) );

        QStandardItem* item = new QStandardItem();
        item->setData(var, Qt::UserRole+1);

        itemList << modelItem << item;

        sourcesModel->appendRow(itemList);
        QItemSelectionModel* selModel = ui->sourcesListView->selectionModel();

        //selModel->select(sourcesModel->index(0,0), QItemSelectionModel::SelectCurrent);
    }
}

void MainWindow::on_currentChanged(const QModelIndex &current, const QModelIndex &previous) {
    //updateSourceDetailControls(current);
}

// param rowIndex: Points to the first item of the selected row or is an empty (invalid) index
void MainWindow::updateSourceDetailControls(const QModelIndex& rowIndex) {

    //QVariant
    if (rowIndex.siblingAtColumn(1).isValid()) {
        QString sourcePath = rowIndex.data().toString();
        SourceDetails* pDetails = rowIndex.siblingAtColumn(1).data(Qt::UserRole+1).value<std::shared_ptr<SourceDetails>>().get();
        //ui->lineEdit_3->setText(pDetails->predicate);
        //ui->lineEdit_2->setText(sourcePath);
        qInfo() << "In updateSourceDetailControls: " << pDetails->predicate;
        ui->groupBoxSourceDetails->setTitle( QString("Source %1").arg(sourcePath) );
        ui->groupBoxSourceDetails->setHidden(false);
    } else {
        ui->groupBoxSourceDetails->setHidden(true);
    }
}

void MainWindow::on_lineEdit_2_editingFinished()
{
    qInfo() << "editing finished";
}

void MainWindow::on_updateSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
    //qInfo()  << "selection changed. empty: " << selected.empty();
    //ui->groupBox_2->setHidden(selected.empty());

    //if ( !deselected.empty()) {
    //    qInfo() << "deselected: " << deselected.indexes().first().data();
    //}

    if ( !selected.empty()) {
        qInfo() << "selected: " << selected.indexes().first().data();
        //qInfo() << "selected.indexes().first(): " << selected.indexes().first().data();
        //qInfo() << "selected.indexes().first(): " << selected.indexes().first().data();
        //qInfo() << "selected.indexes().first(): " << selected.indexes().first().siblingAtColumn(1).data(Qt::UserRole+1).value<std::shared_ptr<SourceDetails>>().get()->filterCommand;
        updateSourceDetailControls(selected.indexes().first());
    } else {
        updateSourceDetailControls(QModelIndex()); // empty stuff
    }
}



void MainWindow::on_pushButton_2_clicked()
{
    QProcess process;
    //process.startDetached("xterm", {"-e", "/home/nando/tmp/s.sh"});

    QString backupName = "backup1.sh";
    QString backupScriptPath = Lb::scriptsDirectory() + "/" + backupName;

    process.startDetached("xterm", {"-e", "/opt/lbackup/install-systemd-hook.sh","install","-s", "gui-service", "-u", ui->lineEditSystemdUnit->text(), backupScriptPath});
    process.waitForFinished(-1);
}




void MainWindow::on_removeSourceButton_clicked()
{
    const QItemSelection selection = ui->sourcesListView->selectionModel()->selection();
    if (!selection.isEmpty()) {
        QModelIndex i = selection.indexes().first();
        qInfo() << "will remove " << i.data();
        sourcesModel->removeRow(i.row());
    } else {
        qInfo() << "nothing selected";
    }
}


// this saves backup configuration and generates scripts
void MainWindow::on_pushButton_3_clicked()
{

    QProcess process;
    process.startDetached("xterm", {"-e", "/home/nando/tmp/s.sh"});
    process.waitForFinished(-1);

}


void MainWindow::on_pushButtonSelectDevice_clicked()
{
    QString stringResult;
    SystemdUnitDialog dialog(stringResult,this);
    if ( dialog.exec() == QDialog::Accepted) {
        qInfo() << "resulting value: " << stringResult;
        ui->lineEditSystemdUnit->setText(stringResult);
    }
}


void MainWindow::on_pushButton_4_clicked()
{
    QProcess process;

    process.startDetached("xterm", {"-e", "/opt/lbackup/install-systemd-hook.sh","remove","-s", "gui-service"});
    process.waitForFinished(-1);
}

