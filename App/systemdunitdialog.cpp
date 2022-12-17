#include "systemdunitdialog.h"
#include "ui_systemdunitdialog.h"

#include <QDebug>

#include "utils.h"
#include "terminal.h"

#include "mylistview.h"

SystemdUnitDialog::SystemdUnitDialog(DialogResult& pdialogResult, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SystemdUnitDialog),
    systemdUnitsModel(new QStandardItemModel(0,2,this)),
    dialogResult(pdialogResult)
{
    ui->setupUi(this);

    myListView = new MyListView();
    ui->verticalLayout_2->addWidget(myListView);
    myListView->setModel(systemdUnitsModel);
    //ui->systemdUnitsView->setModel(systemdUnitsModel);

    //QObject::connect(ui->systemdUnitsView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SystemdUnitDialog::on_UpdateSelection);
    QObject::connect(myListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SystemdUnitDialog::on_UpdateSelection);

    //QObject::connect(myListView, &MyListView::doubleClicked, this, &SystemdUnitDialog::on_systemdUnitsView_doubleClicked);
    QObject::connect(myListView, &MyListView::doubleClicked, this, &SystemdUnitDialog::on_buttonOk_clicked); // not really a double click handler but will get the job done i.e. update return values and accept()

    // initialize buttonOk enabled state
    //ui->buttonOk->setEnabled(! ui->systemdUnitsView->selectionModel()->selection().isEmpty());
    ui->buttonOk->setEnabled(! myListView->selectionModel()->selection().isEmpty());

    reloadMountUnits();
}

SystemdUnitDialog::~SystemdUnitDialog()
{
    delete systemdUnitsModel;
    delete myListView;
    delete ui;
}

void SystemdUnitDialog::reloadMountUnits() {
    systemdUnitsModel->clear();
    systemdUnitsModel->setColumnCount(2);
    QString mountUnitsString = Terminal::runShellCommand("systemctl list-units --type=mount | grep 'loaded active mounted' | sed -e 's/^\\s*//' -e 's/\\.mount\\s\\s*loaded active mounted\\s/.mount||/'");
    if (!mountUnitsString.isEmpty()) {
        QStringList lines = mountUnitsString.split("\n", QString::SkipEmptyParts);
        //qInfo() << "lines: " << lines << "\n";

        for (int i = 0; i<lines.size(); i++) {
            QString line = lines.at(i);
            QStringList unitinfo = line.split("||", QString::SkipEmptyParts);
            //qInfo() << unitinfo;

            if ( unitinfo.size() == 2) {
                QList<QStandardItem*> itemList;
                itemList << new QStandardItem(unitinfo[1]);
                itemList << new QStandardItem(unitinfo[0]);
                systemdUnitsModel->appendRow(itemList);
            }
        }
    }
}

void SystemdUnitDialog::on_UpdateSelection(const QItemSelection &selected, const QItemSelection &dselected)
{
    ui->buttonOk->setEnabled(!selected.isEmpty());
}

void SystemdUnitDialog::on_buttonOk_clicked()
{
    //QModelIndex index = ui->systemdUnitsView->selectionModel()->selection().indexes().first();
    QModelIndex index = myListView->selectionModel()->selection().indexes().first();

    dialogResult.mountId = index.siblingAtColumn(1).data().toString();
    dialogResult.mountPath = index.data().toString();
    qInfo() << "Accepted";
    this->accept();
}


void SystemdUnitDialog::on_buttonReload_clicked()
{
    reloadMountUnits();
}

