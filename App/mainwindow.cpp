#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "systemdunitdialog.h"
#include "newbackuptaskdialog.h"
#include "aboutdialog.h"
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
#include <QFontDatabase>
#include <QDateTime>
#include <QAbstractItemView>
#include <QLoggingCategory>

//Q_DECLARE_METATYPE(std::shared_ptr<SourceDetails>)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sourcesDataMapper(new QDataWidgetMapper(this))
{
    ui->setupUi(this);

    qInfo() << "Starting Lisa Backup v.xyz...";
    qInfo() << "Tasks in " << Lb::dataDirectory();
    qInfo() << "Task scripts in " << Lb::scriptsDirectory();
    qInfo() << "Application scripts in " << Lb::appScriptsDir();

    //QStringList configLocations = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation);
    //qInfo() << "Config locations: " << configLocations;

    //QLoggingCategory::setFilterRules(QStringLiteral("*.debug=false\n*.info=false"));

    qDebug() << "[debug]";
    qInfo() << "[info]";
    qWarning() << "[warning]";
    qCritical() << "[critical]";

    //if (QFontDatabase::addApplicationFont(":/FontAwesome.otf") < 0)
    //    qWarning() << "FontAwesome cannot be loaded !";

    /*QFont font;
    font.setFamily("FontAwwwesome");
    font.setPixelSize(16);

    ui->pushButton_5->setFont(font);
    ui->pushButton_5->setText("Run \uf04b");
*/

    //qDebug() << "themeSearchPaths:" << QIcon::themeSearchPaths() << QIcon::themeName();

    QIcon::setThemeName("Papirus");
    ui->pushButtonEditFriendlyName->setIcon(QIcon::fromTheme("document-edit"));
    //ui->toolButton_5->setIcon(QIcon::fromTheme("media-play"));

    sourcesModel = new QStandardItemModel(0,2, this);
    ui->sourcesListView->setModel(sourcesModel);
    ui->sourcesListView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    sourcesDataMapper->setModel(sourcesModel);

    QItemSelectionModel* selectionModel = ui->sourcesListView->selectionModel();

    QObject::connect(selectionModel, &QItemSelectionModel::currentRowChanged, this, &MainWindow::sourceChanged);
    QObject::connect(this, &MainWindow::sourceChanged, this, &MainWindow::updateSourceDetailControls);
    QObject::connect(ui->comboBoxPredicate, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->stackedWidgetPredicate, &QStackedWidget::setCurrentIndex);
    QObject::connect(ui->comboBoxPredicate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updatePredicateTypeIndex);
    QObject::connect(this, &MainWindow::methodChanged, this, &MainWindow::on_activeBackupMethodChanged);
    QObject::connect(this, &MainWindow::actionChanged, this, &MainWindow::on_actionChanged);
    QObject::connect(this, &MainWindow::newBackupName, this, &MainWindow::onNewBackupName);
    QObject::connect(this, &MainWindow::friendlyNameEdited, this, &MainWindow::onFriendlyNameEdited);
    QObject::connect(ui->lineEditSystemdUnit, &QLineEdit::textChanged, this, &MainWindow::onSystemdUnitChanged);
    QObject::connect(this, &MainWindow::modelUpdated, this, &MainWindow::onModelUpdated);

    // 'PleaseQuit' signal bound to application quit
    QObject::connect(this, &MainWindow::PleaseQuit, QCoreApplication::instance(), QCoreApplication::quit, Qt::QueuedConnection);
    // 'Exit' action bount to 'PleaseQuit' signal
    QObject::connect(ui->actionE_xit, &QAction::triggered, this, &MainWindow::PleaseQuit);
    // consoleProcess events
    //QObject::connect(&consoleProcess, &QProcess::started, this, &MainWindow::consoleProcessStarted );
    //QObject::connect(&consoleProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::consoleProcessDataAvail);
    QObject::connect(&consoleProcess, &QProcess::readyReadStandardError, this, &MainWindow::consoleProcessDataAvail);

    //QObject::connect(&consoleProcess, &QProcess::readyReadStandardError, this, &MainWindow::consoleProcessDataAvail);
    QObject::connect(&consoleProcess, QOverload<int>::of(&QProcess::finished), this, &MainWindow::consoleProcessFinished);

    QObject::connect(ui->pushButtonUpdateTrigger, &QPushButton::clicked, this, &MainWindow::on_pushButtonInstallTrigger_clicked);

    ui->lineEditSystemdUnit->setVisible(false);
    ui->pushButtonSelectDevice->setVisible(false);

    session.defaultBrowseBackupDirectory = Lb::homeDirectory();

    Lb::setupDirs();
    activeBackup = new BackupModel();    

    session.recentBackupNames.append("music2"); // manually initialize it for now. Later it will be loaded from file upon startup.

    // determine backup task to load
    QString taskId;
    // Try to load from command parameters
    // TODO
    // otherwise...
    // show newtask dialog, retrieve name, create task file and show
    NewBackupTaskDialog dialog(this, NewBackupTaskDialog::Wizard);
    if ( dialog.exec() == QDialog::Accepted) {
        qDebug() << "[debug] dialog returned: " << dialog.result.id;
        taskId = dialog.result.id;
    } else {
        emit PleaseQuit();
    }

    // ok, we have a valid task id. Let's load it...
    loadTask(taskId);

    //ui->statusbar->showMessage("Ready");
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    qInfo() << "closeEvent()";
    if (checkSave() == QMessageBox::Cancel) {
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::loadTask(QString taskId) {
    // ok, we have a valid task id. Let's load it...
    BackupModel persisted;
    if (Lb::loadPersisted(taskId,persisted)) {
        *activeBackup = persisted;
        activeBackup->backupDetails.tmp.name = taskId;
        initUIControls(*activeBackup);
        state.modelCopy = *activeBackup;
    } else {
        //ui->statusbar->showMessage(QString("Error opening '%1' task").arg(taskId));
        // TODO show wizard again or exit program?
    }
}


void MainWindow::onNewBackupName(QString backupName) {
}

void MainWindow::on_activeBackupMethodChanged(int backupType) {
    ui->groupBoxCriteria->setEnabled(backupType != SourceDetails::all);
    ui->groupBoxAction->setEnabled(backupType != SourceDetails::all);
}

MainWindow::~MainWindow()
{
    delete activeBackup;
    delete sourcesModel;
    delete ui;
}

// appends a 'backup source' to the list on the left
// Note, sourceDetails memory will be automatically released
QStandardItem* MainWindow::appendSource(BackupModel::SourceDetailsIndex iSourceDetails) {
    QStandardItem* modelItem = new QStandardItem(activeBackup->allSourceDetails[iSourceDetails].sourcePath);
    QList<QStandardItem*> itemList;

    QVariant var;
    var.setValue( iSourceDetails );

    itemList << modelItem;

    sourcesModel->appendRow(itemList);
    // select new item in the list view
    QItemSelectionModel* selModel = ui->sourcesListView->selectionModel();

    return modelItem;
}

/**
 * When a source list item is clicked, populate UI (the right hand side of it) with sources details
 * param rowIndex: Points to the first item of the selected row or is an empty (invalid) index
 *
 */
void MainWindow::updateSourceDetailControls(const QModelIndex& rowIndex) {
    ui->widgetSourceDetails->setDisabled(!rowIndex.isValid());
    //QVariant
    if (rowIndex.isValid()) {
        int row = rowIndex.row();
        QString sourcePath = rowIndex.data().toString();
        SourceDetails* pDetails = &activeBackup->allSourceDetails[row];

        //SourceDetails* pDetails = &activeBackup->allSourceDetails[rowIndex.siblingAtColumn(1).data(Qt::UserRole+1).value<BackupModel::SourceDetailsIndex>()];
        ui->comboBoxDepth->setCurrentIndex(pDetails->backupDepth);
        if (pDetails->backupType == SourceDetails::all) {
            ui->radioButtonAll->setChecked(true);
            emit methodChanged(SourceDetails::all);
        }
        else if (pDetails->backupType == SourceDetails::selective) {
            ui->radioButtonSelective->setChecked(true);
            emit methodChanged(SourceDetails::selective);
        }
        ui->radioButtonRsync->setChecked(pDetails->actionType == SourceDetails::rsync);
        ui->radioButtonGitBundle->setChecked(pDetails->actionType == SourceDetails::gitBundle);
        emit actionChanged(pDetails->actionType);

        ui->lineEditContainsFilename->setText(pDetails->containsFilename);
        ui->lineEditNameMatches->setText(pDetails->nameMatches);
        ui->comboBoxPredicate->setCurrentIndex(pDetails->predicateType);
        //ui->widgetSourceDetails->setHidden(false);
    } else {
        //ui->widgetSourceDetails->setHidden(true);
    }
}

void MainWindow::on_removeSourceButton_clicked()
{
    const QItemSelection selection = ui->sourcesListView->selectionModel()->selection();
    if (!selection.isEmpty()) {
        QModelIndex i = selection.indexes().first();
        qInfo() << "will remove " << i.data();
        BackupModel::SourceDetailsIndex iSourceDetails = i.row();
        sourcesModel->removeRow(i.row());
        activeBackup->allSourceDetails.remove(iSourceDetails);
    } else {
        qInfo() << "nothing selected";
    }
}

// gathers application data and populates a PersistenceModel. Used right before storage.
void MainWindow::collectUIControls(BackupModel& persisted) {
    persisted = *activeBackup;
    persisted.allSourceDetails.clear();
    for (int i=0; i<sourcesModel->rowCount(); i++) {
        SourceDetails* sourcep = &activeBackup->allSourceDetails[i];
        persisted.allSourceDetails.append(*sourcep);
    }
}

void MainWindow::initUIControls(BackupModel& backupModel) {
    //*activeBackup = persisted.backupDetails;
    this->setWindowTitle( Lb::windowTitle(backupModel.backupDetails.friendlyName) );
    //ui->lineEditBackupName->setText(backupModel.backupDetails.backupName);
    ui->lineEditFriendlyName->setText(backupModel.backupDetails.friendlyName);
    ui->labelFriendlyName->setText(backupModel.backupDetails.friendlyName);
    ui->lineEditSystemdUnit->setText(backupModel.backupDetails.systemdMountUnit);
    emit ui->lineEditSystemdUnit->textChanged(ui->lineEditSystemdUnit->text()); // updates Install/Update button state
    ui->lineEditDestinationSuffixPath->setText(backupModel.backupDetails.destinationBaseSuffixPath);

    sourcesModel->clear();

    QStandardItem* lastSourceAdded = 0;
    for (int i=0; i<backupModel.allSourceDetails.size(); i++) {
        lastSourceAdded = appendSource(i);
    }
    if (lastSourceAdded)
        ui->sourcesListView->selectionModel()->setCurrentIndex(sourcesModel->indexFromItem(lastSourceAdded), QItemSelectionModel::ClearAndSelect);
    else
        emit sourceChanged(QModelIndex()); // list is empty

    refreshBasePaths(backupModel.backupDetails.destinationBasePath.isEmpty() ? "/" : backupModel.backupDetails.destinationBasePath);

    setupTriggerButtons(activeBackup->backupDetails.tmp.name);
}

void MainWindow::setupTriggerButtons(const QString& backupName) {
    bool triggerExists = Lb::Triggers::systemdHookPresent(backupName);
    //qInfo() << "systemd service present: " << triggerExists;
    ui->pushButtonInstallTrigger->setVisible(!triggerExists);
    ui->pushButtonUpdateTrigger->setVisible(triggerExists);
    ui->pushButtonRemoveTrigger->setEnabled(triggerExists);
}

void MainWindow::on_pushButtonSelectDevice_clicked()
{
    QString stringResult;
    DialogResult dialogResult;
    SystemdUnitDialog dialog(dialogResult,this);
    if ( dialog.exec() == QDialog::Accepted) {
        qInfo() << "result: " << dialogResult.mountId << " - " << dialogResult.mountPath;
        ui->lineEditSystemdUnit->setText(dialogResult.mountId);
        //activeBackup->backupDetails.systemdMountUnit = dialogResult.mountPath;
    }
}

void MainWindow::applyChanges() {
    // 1. gather model data from UI and put in a big object
    // 2. generate backup script file based on the model
    // 3. store model to disk

    BackupModel persisted;
    collectUIControls(persisted);

    QString dataFilePath = Lb::taskFilePathFromName(activeBackup->backupDetails.tmp.name);
    qInfo() << "data file path: " << dataFilePath;
    QFile file(dataFilePath);
    file.open(QIODevice::WriteOnly);
    QDataStream stream(&file);
    stream << persisted;
    file.close();

    QString scriptName = Lb::backupScriptFilePath(activeBackup->backupDetails.tmp.name);
    if (! Lb::generateBackupScript( QString("%1/template/%2").arg(Lb::appScriptsDir(),"backup.sh.tmpl"), scriptName, persisted)) {
                //append(QString("<font color=red>%1</font>").arg(chr));
        ui->plainTextConsole->appendHtml(QString("<font color='red'>Error generating backup script '%1'</font>").arg(scriptName));
    }

    state.modelCopy = *activeBackup; // freshen model state
}

SourceDetails* MainWindow::getSelectedSourceDetails() {
    QModelIndex sourcesModelIndex = ui->sourcesListView->selectionModel()->currentIndex();
    if (sourcesModelIndex.isValid()) {
        SourceDetails* sourcep = &activeBackup->allSourceDetails[sourcesModelIndex.row()];
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
        if (sourcep && sourcep->backupType != SourceDetails::all) {
                sourcep->backupType = SourceDetails::all;
                emit modelUpdated(BackupModel::ValueType::backupType);
        }

        emit methodChanged(SourceDetails::BackupType::all);
    }
}


void MainWindow::on_radioButtonSelective_toggled(bool checked)
{
    if (checked) {
        SourceDetails* sourcep = getSelectedSourceDetails();
        if (sourcep && sourcep->backupType != SourceDetails::selective ) {
            sourcep->backupType = SourceDetails::selective;
            emit modelUpdated(BackupModel::ValueType::backupType);
        }

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

void MainWindow::on_lineEditDestinationSuffixPath_editingFinished()
{
    QString path = activeBackup->backupDetails.destinationBasePath + "/" + activeBackup->backupDetails.destinationBaseSuffixPath;

    QFileInfo dirInfo(path);
    //qInfo() << "exists: " << dirInfo.exists();
    //qInfo() << "isDir: " << dirInfo.isDir();
    //qInfo() << "is writable: " << dirInfo.isWritable();

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
        }
    }
}



void MainWindow::on_action_New_triggered()
{
    if (checkSave() != QMessageBox::Cancel) {
        NewBackupTaskDialog dialog(this, NewBackupTaskDialog::CreateOnly);
        if ( dialog.exec() == QDialog::Accepted) {
            loadTask(dialog.result.id);
        }
    }
}

// if there non-persisted modifications ask user what to do with them
// returns QMessageBox::X status or -1 if no dialog presented
int MainWindow::checkSave() {
    // if ther ehave been any unsaved changed in the model, ask for saving first
    if (! (state.modelCopy == (*activeBackup))) {
        int ret = QMessageBox::warning(this, "Unsaved changes",tr("The backup task has been modified.\nDo you want to save your changes first?"),QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Save);
        if (ret == QMessageBox::Save) {
            applyChanges();
        }
        return ret;
    }
    return -1;
}

void MainWindow::on_action_Open_triggered()
{
    if (checkSave() != QMessageBox::Cancel) {
    NewBackupTaskDialog dialog(this, NewBackupTaskDialog::OpenOnly);
        if (dialog.exec() == QDialog::Accepted) {
            loadTask(dialog.result.id);
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
    activeBackup->backupDetails.destinationBasePath = newPath;
    // select respective systemd unit
    QString systemdUnit;
    if ( ! Lb::systemdUnitForMountPath(newPath, systemdUnit) ) {
        qWarning() << "[warning] no systemd unit for path " << newPath;
    }
    ui->lineEditSystemdUnit->setText(systemdUnit);
    emit ui->lineEditDestinationSuffixPath->editingFinished();
}


void MainWindow::on_action_Save_triggered()
{
    applyChanges();
}

void MainWindow::on_pushButtonInstallTrigger_clicked()
{
    if (Lb::Triggers::installSystemdHook(activeBackup->backupDetails) != 0) {
        ui->plainTextConsole->appendHtml(QString("<font color='red'>Error installing trigger</font>"));
    } else {
        ui->plainTextConsole->appendHtml(QString("On-mount trigger installed"));
    }
    setupTriggerButtons(activeBackup->backupDetails.tmp.name); // re-evaluate button state
}

void MainWindow::on_pushButtonRemoveTrigger_clicked()
{
    if (Lb::Triggers::removeSystemdHook(activeBackup->backupDetails) != 0) {
        ui->plainTextConsole->appendHtml(QString("<font color='red'>Error removing on-mount trigger</font>"));
    } else {
        ui->plainTextConsole->appendHtml(QString("On-mount trigger removed"));
    }
    setupTriggerButtons(activeBackup->backupDetails.tmp.name); // re-evaluate button state
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

void MainWindow::consoleProcessDataAvail() {
    qInfo() << "console process data available!";
    QString out = consoleProcess.readAllStandardError();
    ui->plainTextConsole->appendPlainText(out);
}

void MainWindow::consoleProcessFinished(int exitCode) {
    ui->plainTextConsole->appendHtml(QString("<strong>----- Backup script finished. Exit code: %1 -----</strong>").arg(exitCode));
}

// show newtask dialog, get name and id, persist to .task file, reload and populate UI
void MainWindow::newBackupTaskFromDialog(qint32 dialogMode)
{
    QString stringResult;
    NewBackupTaskDialog dialog(this, (NewBackupTaskDialog::Mode) dialogMode);
    if ( dialog.exec() == QDialog::Accepted) {
        qInfo() << "dialog returned: " << dialog.result.name << " - " << dialog.result.id;

        if (checkSave() != QMessageBox::Cancel) {
            // store to task file
            BackupModel model;
            model.backupDetails.friendlyName = dialog.result.name;
            QString taskFilename = Lb::taskFilePathFromName(dialog.result.id);
            Lb::persistTaskModel(model, taskFilename);
            // TODO error handling

            // reload task file and init UI
            BackupModel persisted;
            if (Lb::loadPersistedFile(taskFilename,persisted)) {
                *activeBackup = persisted;
                activeBackup->backupDetails.tmp.name = dialog.result.id;
                initUIControls(*activeBackup);
            }
        }
    }
}


void MainWindow::on_pushButtonEditFriendlyName_toggled(bool checked)
{
    ui->stackedWidgetFriendlyName->setCurrentIndex(checked);
    if(!checked) {
        if ( activeBackup->backupDetails.friendlyName != ui->lineEditFriendlyName->text() ) {
            activeBackup->backupDetails.friendlyName = ui->lineEditFriendlyName->text();
            emit friendlyNameEdited();
        }
    }
}

void MainWindow::onFriendlyNameEdited() {
    qInfo() << "friendlyNameEdited: " << activeBackup->backupDetails.friendlyName;
    ui->labelFriendlyName->setText(activeBackup->backupDetails.friendlyName);
    setWindowTitle(Lb::windowTitle(activeBackup->backupDetails.friendlyName));
}

void MainWindow::on_lineEditFriendlyName_returnPressed()
{
    qInfo() << "return pressed";
    ui->pushButtonEditFriendlyName->setChecked(false);
}


void MainWindow::on_radioButtonRsync_toggled(bool checked)
{
    if (checked) {
        SourceDetails* sourcep = getSelectedSourceDetails();
        if (sourcep)
            sourcep->actionType = SourceDetails::rsync;

        emit actionChanged(SourceDetails::rsync);
    }
}


void MainWindow::on_radioButtonGitBundle_toggled(bool checked)
{
    if (checked) {
        SourceDetails* sourcep = getSelectedSourceDetails();
        if (sourcep)
            sourcep->actionType = SourceDetails::gitBundle;

        emit actionChanged(SourceDetails::gitBundle);
    }
}

// higher-level handler. Model (getSelectedSourceDetails()) is assumed to contain the updated value
void MainWindow::on_actionChanged(SourceDetails::ActionType action) {
    // TODO
}

void MainWindow::onSystemdUnitChanged(QString newUnitName) {
    //qInfo() << "onSystemdUnitChanged(): systemd unit changed: " << newUnitName;
    ui->pushButtonInstallTrigger->setEnabled(!newUnitName.isEmpty());
    ui->pushButtonUpdateTrigger->setEnabled(!newUnitName.isEmpty());
    // update model
    activeBackup->backupDetails.systemdMountUnit = newUnitName;
    emit modelUpdated(BackupModel::ValueType::systemdMountUnit);
}

void MainWindow::onModelUpdated(BackupModel::ValueType valueType) {
    qDebug() << "[debug] model updated";
}


void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dialog(this);
    if ( dialog.exec() == QDialog::Accepted) {
    } else {
    }
}

void MainWindow::on_toolButtonRun_triggered(QAction *arg1)
{
    checkSave();
    QString backupScriptFile = Lb::backupScriptFilePath(activeBackup->backupDetails.tmp.name);
    consoleProcess.start("bash", {"-c", backupScriptFile});
    if (!consoleProcess.waitForStarted(5000)) {
        ui->plainTextConsole->appendHtml("<strong>Error starting backup script</strong>");
        return;
    }

    ui->plainTextConsole->appendHtml("<strong>----- Launched backup script at " + QDateTime::currentDateTime().toString() + " -----</strong>");
}


void MainWindow::on_toolButtonRun_clicked()
{
    if (checkSave() != QMessageBox::Cancel) {
        QString backupScriptFile = Lb::backupScriptFilePath(activeBackup->backupDetails.tmp.name);
        consoleProcess.start("bash", {"-c", backupScriptFile});
        if (!consoleProcess.waitForStarted(5000)) {
            ui->plainTextConsole->appendHtml("<strong>Error starting backup script</strong>");
            return;
        }
        ui->plainTextConsole->appendHtml("<strong>----- Launched backup script at " + QDateTime::currentDateTime().toString() + " -----</strong>");
    }
}


void MainWindow::on_pushButton_2_clicked()
{
    qInfo() << "equal: " << (state.modelCopy == (*activeBackup));
}


void MainWindow::on_pushButton_3_clicked()
{
    state.modelCopy = *activeBackup;
    qInfo() << "kept a copy of state";
}


void MainWindow::on_pushButtonAdd_clicked()
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
        SourceDetails sourceDetails;
        sourceDetails.sourcePath = selected.at(i);
        activeBackup->allSourceDetails.append(sourceDetails);
        BackupModel::SourceDetailsIndex sourceDetailsIndex = activeBackup->allSourceDetails.size()-1; // points to last item added
        ui->sourcesListView->selectionModel()->setCurrentIndex(sourcesModel->indexFromItem(appendSource(sourceDetailsIndex)), QItemSelectionModel::ClearAndSelect);

    }
}

