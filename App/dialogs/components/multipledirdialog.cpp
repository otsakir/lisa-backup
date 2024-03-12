#include "multipledirdialog.h"
#include "ui_multipledirdialog.h"

#include <QFileSystemModel>

MultipleDirDialog::MultipleDirDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MultipleDirDialog)
{
    ui->setupUi(this);
    fsModel = new QFileSystemModel(this);
    fsModel->setFilter(QDir::Dirs|QDir::NoDot|QDir::NoDotDot);
    fsModel->setRootPath("/");

    ui->dirTreeView->setModel(fsModel);

    // Demonstrating look and feel features
    ui->dirTreeView->setAnimated(false);
    ui->dirTreeView->setIndentation(20);
    //ui->dirTreeView->setSortingEnabled(true);
    ui->dirTreeView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    ui->dirTreeView->setColumnWidth(0, this->width() / 3);

    const QModelIndex rootPosition = fsModel->index(QDir::cleanPath("/"));
    if (rootPosition.isValid())
    {
        ui->dirTreeView->setRootIndex(rootPosition);
    }

    QModelIndex currentPosition = fsModel->index(QDir::cleanPath("/home/nando/Qt")); //"session.defaultBrowseBackupDirectory"));
    if (currentPosition.isValid())
    {
        ui->dirTreeView->setCurrentIndex(currentPosition);
        ui->dirTreeView->expand(currentPosition);
    }

    ui->dirTreeView->show();

    // TODO - it does not really scroll-to. Why ?
    ui->dirTreeView->scrollTo(currentPosition, QAbstractItemView::EnsureVisible);

}

MultipleDirDialog::~MultipleDirDialog()
{
    delete fsModel;
    delete ui;
}

void MultipleDirDialog::on_selectPushButton_clicked()
{
    QItemSelectionModel* selModel = ui->dirTreeView->selectionModel();
    QModelIndexList selectedIndexes = selModel->selectedRows(); //selectedIndexes();

    for (int i=0; i < selectedIndexes.size(); i++)
    {
        selectedPaths << fsModel->filePath(selectedIndexes[i]);
    }
    this->accept();
}

