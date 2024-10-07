#include "multipledirdialog.h"
#include "ui_multipledirdialog.h"

#include <QFileSystemModel>
#include <QSettings>
#include "../utils.h"
#include "../settings.h"

#include <QDebug>
#include <QTimer>

MultipleDirDialog::MultipleDirDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MultipleDirDialog)
{
    QSettings settings;

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


    QString startingPath = settings.value(Settings::Keys::SourceDirStartingPoint).toString();
    if (startingPath.isEmpty())
        startingPath = Lb::homeDirectory();

    QModelIndex currentPosition = fsModel->index(QDir::cleanPath(startingPath));
    if (currentPosition.isValid())
    {
        ui->dirTreeView->setCurrentIndex(currentPosition);
        ui->dirTreeView->expand(currentPosition);
    }
    ui->dirTreeView->show();

    // Scrolling to the selected directory entry only works if it's done async after some time.
    // TODO - find a decent way to do this
    QTreeView* treeView = ui->dirTreeView;
    QTimer::singleShot(30, [treeView]{treeView->scrollTo(treeView->currentIndex()); });
}

MultipleDirDialog::~MultipleDirDialog()
{
    delete fsModel;
    delete ui;
}


void MultipleDirDialog::on_selectPushButton_clicked()
{
    QSettings settings;

    QItemSelectionModel* selModel = ui->dirTreeView->selectionModel();
    QModelIndexList selectedIndexes = selModel->selectedRows(); //selectedIndexes();

    for (int i=0; i < selectedIndexes.size(); i++)
    {
        selectedPaths << fsModel->filePath(selectedIndexes[i]);
    }
    // keep the first of the selected paths for next time we open the dialog
    if (!selectedPaths.empty())
        settings.setValue(Settings::Keys::SourceDirStartingPoint,selectedPaths.first());

    this->accept();
}

