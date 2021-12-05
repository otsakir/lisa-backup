#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "systemdunitdialog.h"
#include "utils.h"
#include "scripting.h"

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

    QObject::connect(ui->comboBoxPredicate, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->stackedWidgetPredicate, &QStackedWidget::setCurrentIndex);
    QObject::connect(ui->comboBoxPredicate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updatePredicateTypeIndex);

    QObject::connect(this, &MainWindow::methodChanged, this, &MainWindow::on_activeBackupMethodChanged);

    ui->groupBoxSourceDetails->setHidden( ui->sourcesListView->selectionModel()->selection().empty() );

    session.defaultBrowseBackupDirectory = Lb::homeDirectory();

    Lb::setupDirs();
    backupDetails = new BackupDetails();

    PersistenceModel persisted;
    // Try to load existing configuration. If not available, defaults from PersistenceModel constructor will be used.
    if (! loadPersisted(persisted)) {
        // init with defaults
        persisted.backupDetails.systemdId = Lb::randomString(16);
        persisted.backupDetails.backupName = "personal-stuff";
    }
    initAppData(persisted);
}

void MainWindow::on_activeBackupMethodChanged(int backupType) {
    qInfo() << "backup method changed: " << backupType;
    ui->groupBoxCriteria->setEnabled(backupType != SourceDetails::all);
}

MainWindow::~MainWindow()
{
    delete backupDetails;
    delete sourcesModel;
    delete ui;
}

// appends a 'backup source' to the list on the left
// Note, sourceDetails memory will be automatically released
void MainWindow::appendSource(SourceDetails* sourceDetails) {
    //QString sourcePath = selected.at(i);
    QStandardItem* modelItem = new QStandardItem(sourceDetails->sourcePath);
    QList<QStandardItem*> itemList;

    //SourceDetails* sourceDetails = new SourceDetails(SourceDetails::selective, "[[ -f dobackup ]]");
    //sourceDetails->sourcePath = sourcePath;
    QVariant var;
    var.setValue( std::shared_ptr<SourceDetails>(sourceDetails) );

    QStandardItem* item = new QStandardItem();
    item->setData(var, Qt::UserRole+1);

    itemList << modelItem << item;

    sourcesModel->appendRow(itemList);
}

void MainWindow::on_pushButton_clicked()
{    
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOptions(QFileDialog::ShowDirsOnly);
    dialog.setDirectory(session.defaultBrowseBackupDirectory);

    QStringList selected;
    if (dialog.exec()) {
        selected= dialog.selectedFiles();
        session.defaultBrowseBackupDirectory = dialog.directory().path();
    }

    for (int i=0; i<selected.size(); i++) {
        SourceDetails* sourceDetails = new SourceDetails();
        sourceDetails->sourcePath = selected.at(i);
        appendSource(sourceDetails); // sourceDetails memory willbe automatically released
    }
}

void MainWindow::on_currentChanged(const QModelIndex &current, const QModelIndex &previous) {
    //updateSourceDetailControls(current);
}

/**
 * When a source list item is clicked, populate UI (the right hand side of it) with sources details
 * param rowIndex: Points to the first item of the selected row or is an empty (invalid) index
 *
 */
void MainWindow::updateSourceDetailControls(const QModelIndex& rowIndex) {

    //QVariant
    if (rowIndex.siblingAtColumn(1).isValid()) {
        QString sourcePath = rowIndex.data().toString();
        SourceDetails* pDetails = rowIndex.siblingAtColumn(1).data(Qt::UserRole+1).value<std::shared_ptr<SourceDetails>>().get();
        ui->comboBoxDepth->setCurrentIndex(pDetails->backupDepth);
        if (pDetails->backupType == SourceDetails::all) {
            ui->radioButtonAll->setChecked(true);
            emit methodChanged(SourceDetails::all);
        }
        else if (pDetails->backupType == SourceDetails::selective) {
            ui->radioButtonSelective->setChecked(true);
            emit methodChanged(SourceDetails::selective);
        }
        ui->lineEditContainsFilename->setText(pDetails->containsFilename);
        ui->lineEditNameMatches->setText(pDetails->nameMatches);
        //ui->comboBoxPredicate->currentIndexChanged(pDetails->predicateType);
        ui->comboBoxPredicate->setCurrentIndex(pDetails->predicateType);
        //qInfo() << "In updateSourceDetailControls: " << pDetails->predicate;
        ui->groupBoxSourceDetails->setTitle( QString("Source %1").arg(sourcePath) );
        ui->groupBoxSourceDetails->setHidden(false);
    } else {
        ui->groupBoxSourceDetails->setHidden(true);
    }
}

void MainWindow::on_updateSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
    if ( !selected.empty()) {
        qInfo() << "selected: " << selected.indexes().first().data();
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
    QString backupScriptPath = Lb::backupScriptFilePath(backupDetails->backupName);

    process.startDetached("xterm", {"-e", "/opt/lbackup/install-systemd-hook.sh","install","-s", backupDetails->backupName, "-u", backupDetails->systemdMountUnit, backupScriptPath});
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

// gathers application data and populates a PersistenceModel. Used right before storage.
void MainWindow::collectAppData(PersistenceModel& persisted) {
    persisted.backupDetails = *backupDetails;
    for (int i=0; i<sourcesModel->rowCount(); i++) {
        SourceDetails* sourcep = sourcesModel->index(i, 1).data(Qt::UserRole+1).value<std::shared_ptr<SourceDetails>>().get();
        persisted.allSourceDetails.append(*sourcep);
        qInfo() << "source type: " << sourcep->backupType << sourcep->sourcePath;
    }
}

void MainWindow::initAppData(const PersistenceModel& persisted) {
    *backupDetails = persisted.backupDetails;
    ui->lineEditSystemdId->setText(backupDetails->systemdId);
    ui->lineEditBackupName->setText(backupDetails->backupName);
    ui->lineEditSystemdUnit->setText(backupDetails->systemdMountUnit);
    ui->lineEditDestinationBasePath->setText(backupDetails->destinationBasePath + "/");
    ui->lineEditDestinationSuffixPath->setText(backupDetails->destinationBaseSuffixPath);

    sourcesModel->clear();
    for (int i=0; i<persisted.allSourceDetails.size(); i++) {
        appendSource(new SourceDetails(persisted.allSourceDetails.at(i)));
    }
}

// this saves backup configuration and generates scripts
void MainWindow::on_pushButton_3_clicked()
{
    // 1. gather model data from UI and put in a big object
    // 2. generate backup script file based on the model
    // 3. store model to disk

    PersistenceModel persisted;
    collectAppData(persisted);

    QString dataFilePath = Lb::backupDataFilePath(backupDetails->backupName);
    qInfo() << "data file path: " << dataFilePath;
    QFile file(dataFilePath);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream << persisted;
    file.close();

    QString scriptName = Lb::backupScriptFilePath(backupDetails->backupName);
    qInfo() << "script name: " << scriptName;
    Lb::generateBackupScript("/home/nando/src/qt/LisaBackup/scripts/templates/backup.sh.tmpl", scriptName, persisted);

}


void MainWindow::on_pushButtonSelectDevice_clicked()
{
    QString stringResult;
    DialogResult dialogResult;
    SystemdUnitDialog dialog(dialogResult,this);
    if ( dialog.exec() == QDialog::Accepted) {
        qInfo() << "result: " << dialogResult.mountId << " - " << dialogResult.mountPath;
        ui->lineEditSystemdUnit->setText(dialogResult.mountId);
        backupDetails->destinationBasePath = dialogResult.mountPath;
        ui->lineEditDestinationBasePath->setText(dialogResult.mountPath);
        ui->lineEditDestinationSuffixPath->setText(dialogResult.backupSubdir);
        //ui->lineEditDestinationBasePath->setText(dialogResult.mountPath);
    }
}


void MainWindow::on_pushButton_4_clicked()
{
    QProcess process;

    process.startDetached("xterm", {"-e", "/opt/lbackup/install-systemd-hook.sh","remove","-s", backupDetails->backupName});
    process.waitForFinished(-1);
}


bool MainWindow::loadPersisted(PersistenceModel& persisted) {
    QFile ifile(Lb::backupDataFilePath("personal-stuff"));
    if (ifile.open(QIODevice::ReadOnly)) {
        QDataStream istream(&ifile);
        istream >> persisted;
        ifile.close();
        return true;
    }
    return false;
}

void MainWindow::on_pushButtonLoad_clicked()
{
    PersistenceModel persisted;
    loadPersisted(persisted);
    initAppData(persisted);
}

SourceDetails* MainWindow::getSelectedSourceDetails() {
    QModelIndex sourcesModelIndex = ui->sourcesListView->selectionModel()->currentIndex().siblingAtColumn(1);
    if (sourcesModelIndex.isValid()) {
        SourceDetails* sourcep = sourcesModelIndex.data(Qt::UserRole+1).value<std::shared_ptr<SourceDetails>>().get();
        return sourcep;
    }
    else
        return 0;
}

void MainWindow::on_comboBoxDepth_currentIndexChanged(int comboIndex)
{
    SourceDetails* sourcep = getSelectedSourceDetails();
    if (sourcep)
        sourcep->backupDepth = (SourceDetails::BackupDepth) comboIndex;
    else
        qWarning() << "No valid source model available";
}


void MainWindow::on_radioButtonAll_toggled(bool checked)
{
    if (checked) {
        SourceDetails* sourcep = getSelectedSourceDetails();
        if (sourcep)
            sourcep->backupType = SourceDetails::all;

        emit methodChanged(SourceDetails::BackupType::all);
    }
}


void MainWindow::on_radioButtonSelective_toggled(bool checked)
{
    if (checked) {
        SourceDetails* sourcep = getSelectedSourceDetails();
        if (sourcep)
            sourcep->backupType = SourceDetails::selective;

        emit methodChanged(SourceDetails::BackupType::selective);
    }


}


void MainWindow::on_lineEditContainsFilename_editingFinished()
{
    SourceDetails* sourcep = getSelectedSourceDetails();
    if (sourcep)
        sourcep->containsFilename = ui->lineEditContainsFilename->text();
}


void MainWindow::on_lineEditNameMatches_editingFinished()
{
    SourceDetails* sourcep = getSelectedSourceDetails();
    if (sourcep)
        sourcep->nameMatches = ui->lineEditNameMatches->text();

}

// when combo box value changes, we should update the index variable stored in
// current SourceDetails struct
void MainWindow::updatePredicateTypeIndex(int index)
{
    SourceDetails* sourcep = getSelectedSourceDetails();
    if (sourcep)
        sourcep->predicateType = (SourceDetails::PredicateType) index;
}

void MainWindow::on_lineEditSystemdUnit_textChanged(const QString &arg1)
{
    backupDetails->systemdMountUnit = ui->lineEditSystemdUnit->text();
}

void MainWindow::on_lineEditDestinationSuffixPath_textChanged(const QString &arg1)
{
    backupDetails->destinationBaseSuffixPath = ui->lineEditDestinationSuffixPath->text();
}


void MainWindow::on_toolButton_toggled(bool checked)
{
    if (! checked) {
        // looks like we're done editing
        if ( ui->lineEditBackupName->text() != backupDetails->backupName) {
            //qInfo() << "backupName updated";
            // at this point we can display a confirmation for the passing on the udpate or prevent the update
            backupDetails->backupName = ui->lineEditBackupName->text();
        }
    }
    ui->lineEditBackupName->setEnabled(checked);
}


void MainWindow::on_lineEditBackupName_editingFinished()
{
}

void MainWindow::on_lineEditDestinationSuffixPath_editingFinished()
{
    QString path = backupDetails->destinationBasePath + "/" + backupDetails->destinationBaseSuffixPath;

    QFileInfo dirInfo(path);
    qInfo() << "exists: " << dirInfo.exists();
    qInfo() << "isDir: " << dirInfo.isDir();
    qInfo() << "is writable: " << dirInfo.isWritable();

    QString message;
    bool ok;
    if (dirInfo.isDir() && dirInfo.isWritable()) {
        ok = true;
    } else {
        if (! dirInfo.exists() )
            message = "Directory does not exist";
        else if (!dirInfo.isDir())
            message = "This is not a directory";
        else if (!dirInfo.isWritable())
            message = "Directory is not writable. Make sure you can write to it";
        ok = false;
    }

    if (ok) {
        ui->labelDirectoryStatusMessage->clear();
    } else {
        ui->labelDirectoryStatusMessage->setText(message);
    }
}


void MainWindow::on_pushButtonChooseDestinationSubdir_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOptions(QFileDialog::ShowDirsOnly);
    dialog.setDirectory(backupDetails->destinationBasePath);

    QStringList selectedItems;
    if (dialog.exec()) {
        selectedItems = dialog.selectedFiles();
        for (int i=0; i<selectedItems.size(); i++) {
            QString selected = selectedItems.at(i);
            qInfo() << "selected dir: " << selected;

            if (selected.startsWith( backupDetails->destinationBasePath )) {
                QString suffix = selected.right(selected.size()-backupDetails->destinationBasePath.size());
                qInfo() << "suffix: " << suffix;

                if (suffix.startsWith("/"))
                    suffix.remove(0,1);

                qInfo() << "suffix after: " << suffix;
                ui->lineEditDestinationSuffixPath->setText(suffix);
                ui->lineEditDestinationSuffixPath->editingFinished();



            }


            //SourceDetails* sourceDetails = new SourceDetails();
            //sourceDetails->sourcePath = selected.at(i);
            //appendSource(sourceDetails); // sourceDetails memory willbe automatically released
        }
    }
}


