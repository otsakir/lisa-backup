#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "systemdunitdialog.h"
#include "utils.h"
#include "scripting.h"

#include <core.h>

#include <memory>

#include <QDebug>

#include <QFileDialog>

#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>
#include <QStorageInfo>

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

    //QObject::connect(this, &MainWindow::backupNameChanged, this, &MainWindow::setTriggerButtons);
    QObject::connect(this, &MainWindow::newBackupName, this, &MainWindow::onNewBackupName);

   // QObject::connect(ui->actionE_xit, &QAction::triggered,  )
     //       connect(quitButton, &QPushButton::clicked, &app, &QCoreApplication::quit, Qt::QueuedConnection);

    // 'PleaseQuit' signal bound to application quit
    QObject::connect(this, &MainWindow::PleaseQuit, QCoreApplication::instance(), QCoreApplication::quit, Qt::QueuedConnection);
    // 'Exit' action bount to 'PleaseQuit' signal
    QObject::connect(ui->actionE_xit, &QAction::triggered, this, &MainWindow::PleaseQuit);

    ui->groupBoxSourceDetails->setHidden( ui->sourcesListView->selectionModel()->selection().empty() );

    session.defaultBrowseBackupDirectory = Lb::homeDirectory();

    Lb::setupDirs();
    activeBackup = new BackupModel();

    session.recentBackupNames.append("personal-stuff"); // manually initialize it for now. Later it will be loaded from file upon startup.

    // load latest backup name from configuration
    QString latestBackupName = session.recentBackupNames.first();
    // Try to load existing configuration. If not available, defaults from PersistenceModel constructor will be used.
    if (! loadPersisted(latestBackupName, *activeBackup)) {
        // init with defaults
        //persisted.backupDetails.systemdId = Lb::randomString(16);
        //persisted.backupDetails.backupName = "";
    }

    initUIControls(*activeBackup);
}


void MainWindow::onNewBackupName(QString backupName) {
    qInfo() << "new backup name!";
}

void MainWindow::on_activeBackupMethodChanged(int backupType) {
    qInfo() << "backup method changed: " << backupType;
    ui->groupBoxCriteria->setEnabled(backupType != SourceDetails::all);
}

MainWindow::~MainWindow()
{
    delete activeBackup;
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
    Lb::Triggers::installSystemdHook(activeBackup->backupDetails);
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
void MainWindow::collectUIControls(BackupModel& persisted) {
    persisted = *activeBackup;
    persisted.allSourceDetails.clear();
    for (int i=0; i<sourcesModel->rowCount(); i++) {
        SourceDetails* sourcep = sourcesModel->index(i, 1).data(Qt::UserRole+1).value<std::shared_ptr<SourceDetails>>().get();
        persisted.allSourceDetails.append(*sourcep);
        qInfo() << "source type: " << sourcep->backupType << sourcep->sourcePath;
    }
}

void MainWindow::initUIControls(const BackupModel& backupModel) {
    //*activeBackup = persisted.backupDetails;
    ui->lineEditBackupName->setText(backupModel.backupDetails.backupName);
    ui->lineEditSystemdUnit->setText(backupModel.backupDetails.systemdMountUnit);
    ui->lineEditDestinationSuffixPath->setText(backupModel.backupDetails.destinationBaseSuffixPath);

    sourcesModel->clear();
    for (int i=0; i<backupModel.allSourceDetails.size(); i++) {
        appendSource(new SourceDetails(backupModel.allSourceDetails.at(i)));
    }

    refreshBasePaths(backupModel.backupDetails.destinationBasePath.isEmpty() ? "/" : backupModel.backupDetails.destinationBasePath);

    bool newBackup = ui->lineEditBackupName->text().isEmpty();

    ui->lineEditBackupName->setEnabled(newBackup);
    enableMostUI(!newBackup);
    ui->pushButtonOk->setVisible(newBackup);
    ui->toolButton->setVisible(!newBackup);

    // enable/disable trigger buttons
    if (!newBackup) {
        setupTriggerButtons(ui->lineEditBackupName->text());
    }
}

void MainWindow::setupTriggerButtons(const QString& backupName) {
    bool triggerExists = Lb::Triggers::systemdHookPresent(backupName);
    qInfo() << "systemd service present: " << triggerExists;
    ui->pushButtonInstallTrigger->setEnabled(!triggerExists); // already installed
    ui->pushButtonRemoveTrigger->setEnabled(triggerExists);
}

void MainWindow::enableMostUI(bool enable) {

    ui->groupBoxSourceList->setEnabled(enable);
    ui->groupBoxSourceDetails->setEnabled(enable);
    ui->groupBoxDestination->setEnabled(enable);

    // disable signals because setDisabled() will trigger another round of editingFinished() event
    //ui->lineEditBackupName->blockSignals(true);
    //ui->lineEditBackupName->setEnabled(!disableMostUi);
    //ui->lineEditBackupName->blockSignals(false);

    //ui->toolButton->blockSignals(true);
    //ui->toolButton->setChecked(disableMostUi);
    //ui->toolButton->blockSignals(false);

    /*
    activeBackup->backupName = ui->lineEditBackupName->text();
    if (!disableMostUi) {
        bool present = Lb::Triggers::systemdHookPresent(activeBackup->backupName);
        qInfo() << "systemd service present: " << present;
        //TODO: enable or disable pushButtonInstallTrigger and pushButtonRemoveTrigger
    }

    emit backupNameChanged(activeBackup->backupName);
    */

}



void MainWindow::on_pushButtonSelectDevice_clicked()
{
    QString stringResult;
    DialogResult dialogResult;
    SystemdUnitDialog dialog(dialogResult,this);
    if ( dialog.exec() == QDialog::Accepted) {
        qInfo() << "result: " << dialogResult.mountId << " - " << dialogResult.mountPath;
        ui->lineEditSystemdUnit->setText(dialogResult.mountId);
        activeBackup->backupDetails.systemdMountUnit = dialogResult.mountPath;
        //activeBackup->destinationBasePath = dialogResult.mountPath;
        //ui->lineEditDestinationBasePath->setText(dialogResult.mountPath);
        //ui->lineEditDestinationSuffixPath->setText(dialogResult.backupSubdir);
        //ui->lineEditDestinationBasePath->setText(dialogResult.mountPath);
    }
}


void MainWindow::on_pushButton_4_clicked()
{

}


bool loadPersistedFile(const QString backupFilename, BackupModel& persisted) {
    QFile ifile(backupFilename);
    if (ifile.open(QIODevice::ReadOnly)) {
        QDataStream istream(&ifile);
        istream >> persisted;
        ifile.close();
        return true;
    }
    return false;
}

bool MainWindow::loadPersisted(const QString backupName, BackupModel& persisted) {
    return loadPersistedFile(Lb::backupDataFilePath(backupName), persisted);
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
    activeBackup->backupDetails.systemdMountUnit = ui->lineEditSystemdUnit->text();
}

void MainWindow::on_lineEditDestinationSuffixPath_textChanged(const QString &arg1)
{
    activeBackup->backupDetails.destinationBaseSuffixPath = ui->lineEditDestinationSuffixPath->text();
}


void MainWindow::on_toolButton_toggled(bool checked)
{
    if (! checked) {
        // looks like we're done editing
        if ( ui->lineEditBackupName->text() != activeBackup->backupDetails.backupName) {
            //qInfo() << "backupName updated";
            // at this point we can display a confirmation for passing on the update or prevent it
            activeBackup->backupDetails.backupName = ui->lineEditBackupName->text();
        }
    }
    ui->lineEditBackupName->setEnabled(checked);
}

void MainWindow::on_lineEditDestinationSuffixPath_editingFinished()
{
    QString path = activeBackup->backupDetails.destinationBasePath + "/" + activeBackup->backupDetails.destinationBaseSuffixPath;

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
            message = "Directory is not writable";
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

    // try to find optimal starting directory for the dialog. Put it in validPath.

    QString validPath;
    Lb::bestValidDirectoryMatch(ui->comboBoxBasePath->currentText() + ui->lineEditDestinationSuffixPath->text(), validPath);

    dialog.setDirectory(validPath);
    QStringList selectedItems;
    if (dialog.exec()) {
        selectedItems = dialog.selectedFiles();
        for (int i=0; i<selectedItems.size(); i++) {
            QString selected = selectedItems.at(i);
            qInfo() << "selected dir: " << selected;

            if (selected.startsWith( activeBackup->backupDetails.destinationBasePath )) {
                QString suffix = selected.right(selected.size()-activeBackup->backupDetails.destinationBasePath.size());
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



void MainWindow::on_action_New_triggered()
{
    // if the active backup or sources have been touched, show a confirmation dialog
    // ...
    int ret = QMessageBox::warning(this, tr("My Application"),
                                   tr("The backup task has been modified.\nDo you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);

    if (ret == QMessageBox::Cancel) // or user pressed 'X' window key
        return;

    if (ret == QMessageBox::Save) {
        on_ButtonApply_clicked();
        // TODO possible errors when saving. Ask for confirmation and abort 'new' operation accordingly
    }

    // all clear or (ret == QMessageBox::Discard)

    BackupModel persisted;
    initUIControls(persisted);
}

void MainWindow::applyChanges() {
    // 1. gather model data from UI and put in a big object
    // 2. generate backup script file based on the model
    // 3. store model to disk

    BackupModel persisted;
    collectUIControls(persisted);

    QString dataFilePath = Lb::backupDataFilePath(activeBackup->backupDetails.backupName);
    qInfo() << "data file path: " << dataFilePath;
    QFile file(dataFilePath);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream << persisted;
    file.close();

    QString scriptName = Lb::backupScriptFilePath(activeBackup->backupDetails.backupName);
    qInfo() << "script name: " << scriptName;
    Lb::generateBackupScript("/home/nando/src/qt/LisaBackup/scripts/templates/backup.sh.tmpl", scriptName, persisted);

}
// this saves backup configuration and generates scripts
void MainWindow::on_ButtonApply_clicked()
{
    applyChanges();
}


void MainWindow::on_action_Open_triggered()
{
    QFileDialog dialog(this);
    dialog.setDirectory(Lb::dataDirectory());

    if (dialog.exec()) {
        QString filename = dialog.selectedFiles().first();
        qInfo() << "filename: " << filename;

        BackupModel persisted;
        if (loadPersistedFile(filename,persisted)) {
            *activeBackup = persisted;
            initUIControls(*activeBackup);
        }
    }
}

// reloads paths for system mounted devices. Adds 'current' if not already in the list
void MainWindow::refreshBasePaths(QString current) {
    ui->comboBoxBasePath->clear();
    int indexFound = -1; // assume not found
    int i = 0; // counter
    QString rootPath;
    foreach( const QStorageInfo& storage, QStorageInfo::mountedVolumes()) {
        rootPath = storage.rootPath();
        if (!current.isEmpty() && rootPath == current) {
            indexFound = i;
        }
        ui->comboBoxBasePath->addItem(storage.rootPath());
        i++;
    }
    if (!current.isEmpty()) {
        if (indexFound != -1) {
            ui->comboBoxBasePath->setCurrentIndex(indexFound);
        } else {
            ui->comboBoxBasePath->addItem(current);
            ui->comboBoxBasePath->setCurrentIndex(i);
            // todo - set current index point to this item
        }
    }
}

void MainWindow::on_pushButtonRefreshBasePaths_clicked()
{
    refreshBasePaths(activeBackup->backupDetails.destinationBasePath);
}


void MainWindow::on_comboBoxBasePath_currentIndexChanged(const QString &newPath)
{
    qInfo() << "selected base path changed: " << newPath;
    activeBackup->backupDetails.destinationBasePath = newPath;

    QString systemdUnit;
    if ( Lb::systemdUnitForMountPath(newPath, systemdUnit) ) {
        qInfo() << "systemd unit: " << systemdUnit;
        activeBackup->backupDetails.systemdMountUnit = systemdUnit;
        ui->lineEditSystemdUnit->setText(systemdUnit);
    } else {
        qWarning() << "no systemd unit for path " << newPath;
        ui->lineEditSystemdUnit->clear();
        activeBackup->backupDetails.systemdMountUnit.clear();
    }

    emit ui->lineEditDestinationSuffixPath->editingFinished();
}


void MainWindow::on_comboBoxBasePath_currentIndexChanged(int index)
{

}


void MainWindow::on_action_Save_triggered()
{
    applyChanges();
}


void MainWindow::on_lineEditBackupName_editingFinished()
{
    activeBackup->backupDetails.backupName = ui->lineEditBackupName->text();
}


void MainWindow::on_lineEditBackupName_returnPressed()
{
    qInfo () << "return pressed";
    emit ui->pushButtonOk->clicked();
}


void MainWindow::on_pushButton_TestEdit_clicked()
{
    qInfo() << "test  edit clicked";
    //ui->toolButton->setCheckable(true);
    ui->toolButton->setChecked(true);

}


void MainWindow::on_pushButtonInstallTrigger_clicked()
{
    Lb::Triggers::installSystemdHook(activeBackup->backupDetails);
    setupTriggerButtons(activeBackup->backupDetails.backupName); // re-evaluate button state
}


void MainWindow::on_pushButtonOk_clicked()
{
    // validate current backupName. For now we just check if non-empty
    if (!ui->lineEditBackupName->text().isEmpty()) {
        // ok, "valid"
        ui->lineEditBackupName->setEnabled(false);
        enableMostUI(true);
        setupTriggerButtons(ui->lineEditBackupName->text());
        ui->pushButtonOk->setVisible(false);
        ui->toolButton->setVisible(true);
    }

    qInfo() << "in buttonOk";
}


void MainWindow::on_pushButtonRemoveTrigger_clicked()
{
    Lb::Triggers::removeSystemdHook(activeBackup->backupDetails);
    setupTriggerButtons(activeBackup->backupDetails.backupName); // re-evaluate button state
}


void MainWindow::on_actionDelete_triggered()
{
    int ret = QMessageBox::warning(this, tr("Lisa Backup"),
                                   tr("You are about to delete your backup task!"),
                                   QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

    if (ret == QMessageBox::Cancel)
        return;

    if (ret == QMessageBox::Ok) {
        // Lb::removeBackupFiles() // removes systemd .service file, backup data file and backup scropt
        // clear backup task state - activeBackup
        // inform UI


    }
}

