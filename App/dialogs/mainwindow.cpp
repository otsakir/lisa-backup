#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "newbackuptaskdialog.h"
#include "aboutdialog.h"
#include "../utils.h"
#include "../scripting.h"
#include "../task.h"
#include "components/multipledirdialog.h"

#include "../core.h"

#include "../logging.h"

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
#include <QFileSystemModel>
#include <QTreeView>
#include <QScroller>
#include <QCloseEvent>

#include "../dbusutils.h"
#include "../triggering.h"
#include "../appcontext.h"
#include "components/triggeringcombobox.h"
#include "../taskrunnermanager.h"



//Q_DECLARE_METATYPE(std::shared_ptr<SourceDetails>)


MainWindow::MainWindow(QString taskName, AppContext* appContext, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sourcesDataMapper(new QDataWidgetMapper(this))
    , appContext(appContext)

{
    ui->setupUi(this);
    taskLoader = appContext->getTaskLoader();

    qDebug() << "Tasks in " << Lb::dataDirectory();
    qDebug() << "Application scripts in " << Lb::appScriptsDir();

    //QStringList configLocations = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation);
    //qInfo() << "Config locations: " << configLocations;

//    qDebug() << "[debug]";
//    qInfo() << "[info]";
//    qWarning() << "[warning]";
//    qCritical() << "[critical]";

    // set up models for sources listview
    sourcesModel = new QStandardItemModel(0,2, this);
    ui->sourcesListView->setModel(sourcesModel);
    ui->sourcesListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    sourcesDataMapper->setModel(sourcesModel);
    QItemSelectionModel* selectionModel = ui->sourcesListView->selectionModel();

    ui->toolButtonRun->setIcon(QIcon(":/custom-icons/play.svg"));
    ui->toolButtonSaveTask->setIcon(QIcon(":/custom-icons/save.svg"));
    ui->toolButtonAdd->setIcon(QIcon(":/custom-icons/folder-plus.svg"));
    ui->removeSourceButton->setIcon(QIcon(":/custom-icons/folder-minus.svg"));
    ui->toolButtonSourceUp->setIcon(QIcon(":/custom-icons/chevron-up.svg"));
    ui->toolButtonSourceDown->setIcon(QIcon(":/custom-icons/chevron-down.svg"));


    connect(selectionModel, &QItemSelectionModel::currentRowChanged, this, &MainWindow::sourceChanged);
    connect(this, &MainWindow::sourceChanged, this, &MainWindow::updateSourceDetailControls);
    connect(ui->comboBoxPredicate, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->stackedWidgetPredicate, &QStackedWidget::setCurrentIndex);
    connect(ui->comboBoxPredicate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updatePredicateTypeIndex); // updates internal model
    connect(this, &MainWindow::methodChanged, this, &MainWindow::on_activeBackupMethodChanged);
    connect(this, &MainWindow::actionChanged, this, &MainWindow::on_actionChanged);
    connect(this, &MainWindow::newBackupName, this, &MainWindow::onNewBackupName);
    connect(this, &MainWindow::modelUpdated, this, &MainWindow::onModelUpdated);
    // 'PleaseQuit' signal bound to application quit
    connect(this, &MainWindow::PleaseQuit, QCoreApplication::instance(), QCoreApplication::quit, Qt::QueuedConnection);
    // 'Exit' action bount to 'PleaseQuit' signal
    connect(ui->actionE_xit, &QAction::triggered, this, &MainWindow::PleaseQuit);

    connect(ui->toolButtonRun, &QPushButton::clicked, this, &MainWindow::runActiveTask);
    //connect(ui->checkBoxOnMountTrigger, &QCheckBox::clicked, this, &MainWindow::onCheckBoxMountTriggerClicked);
    //connect(ui->lineEditDestinationSuffixPath, &QLineEdit::editingFinished, this, &MainWindow::checkLineEditDestinationSuffixPath);
    connect(ui->lineEditDestinationSuffixPath, &QLineEdit::textChanged, this, &MainWindow::checkLineEditDestinationSuffixPath);
    //connect(ui->comboBoxBasePath, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::on_comboBoxBasePath_currentIndexChanged);

    //connect(ui->action_ManageTasks, &QAction::triggered, this, &MainWindow::showTriggerMonitorClicked);


    triggeringCombo = new TriggeringComboBox(this);
    triggeringCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    static_cast<QHBoxLayout*>(ui->horizontalLayout_Triggering->layout())->insertWidget(0, triggeringCombo);
    connect(triggeringCombo, &TriggeringComboBox::triggerEntrySelected, this, &MainWindow::_triggerEntrySelected);

    Lb::setupDirs();
    activeBackup = new BackupModel();

    if (!taskName.isEmpty())
        openTask(taskName);
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    if (checkSave() == QMessageBox::Cancel) {
        event->ignore();
    } else {
        event->accept();
    }
}


bool MainWindow::openTask(QString taskId) {
    assert(!taskId.isEmpty());

    if (!taskId.isEmpty())
    {
        BackupModel persisted;
        if (taskLoader->loadTask(taskId,persisted)) {
            *activeBackup = persisted;
            activeBackup->backupDetails.tmp.taskId = taskId;
            initUIControls(*activeBackup);
            state.modelCopy = *activeBackup;
            this->taskName = taskId;
            ui->stackedMain->setCurrentIndex(ui->stackedMain->indexOf(ui->pageAll));
            return true;
        } else {
            qCritical() << "[critical] error loading task " << taskId;
        }
    }

    this->taskName.clear();
    ui->stackedMain->setCurrentIndex(ui->stackedMain->indexOf(ui->pageNothing));
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
    Logging::setUiConsole(0);
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
        ui->radioButtonAuto->setChecked(pDetails->actionType == SourceDetails::automatic);
        emit actionChanged(pDetails->actionType);

        ui->lineEditContainsFilename->setText(pDetails->containsFilename);
        ui->lineEditNameMatches->setText(pDetails->nameMatches);

        // update UI field contentand trigger changes to dependent fields
        ui->comboBoxPredicate->setCurrentIndex(pDetails->predicateType);
        emit ui->comboBoxPredicate->currentIndexChanged(pDetails->predicateType);

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
    this->setWindowTitle( Lb::windowTitle(backupModel.backupDetails.tmp.taskId ));  //friendlyName) );
    //ui->lineEditBackupName->setText(backupModel.backupDetails.backupName);
    ui->lineEditDestinationSuffixPath->setText(backupModel.backupDetails.destinationPath);

    sourcesModel->clear();

    QStandardItem* lastSourceAdded = 0;
    for (int i=0; i<backupModel.allSourceDetails.size(); i++) {
        lastSourceAdded = appendSource(i);
    }
    if (lastSourceAdded)
        ui->sourcesListView->selectionModel()->setCurrentIndex(sourcesModel->indexFromItem(lastSourceAdded), QItemSelectionModel::ClearAndSelect);
    else
        emit sourceChanged(QModelIndex()); // list is empty

    // set up triggering combo
    QList<MountedDevice> availableTriggerEntries;
    DbusUtils::getMountedDevices(availableTriggerEntries);
    MountedDevice taskTriggerEntry; // triggering entry for the current task
    Triggering::triggerEntryForTask(backupModel.backupDetails.tmp.taskId, taskTriggerEntry);
    triggeringCombo->refresh(availableTriggerEntries, taskTriggerEntry);
}

/**
 * @brief MainWindow::applyChanges
 *
 * Populate a task model object from UI and persist it to a task file on the disk
 *
 */
void MainWindow::applyChanges() {
    QSettings settings;

    // gather model data from UI and put in a big object
    BackupModel persisted;
    collectUIControls(persisted);

    // store model to disk
    QString taskId = activeBackup->backupDetails.tmp.taskId;
    Tasks::saveTask(taskId, persisted);

    state.modelCopy = *activeBackup; // freshen model state
    emit taskSaved(taskId);

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

// update internal model
void MainWindow::updatePredicateTypeIndex(int index)
{
    SourceDetails* sourcep = getSelectedSourceDetails();
    if (sourcep)
        sourcep->predicateType = (SourceDetails::PredicateType) index;
}

void MainWindow::on_lineEditDestinationSuffixPath_textChanged(const QString &arg1)
{
    activeBackup->backupDetails.destinationPath = ui->lineEditDestinationSuffixPath->text();
}

void MainWindow::checkLineEditDestinationSuffixPath(const QString& newText)
{
    //QString path = "/" + activeBackup->backupDetails.destinationPath;
    QString path = newText; //ui->lineEditDestinationSuffixPath->text();
    QFileInfo dirInfo(path);
    QString message;
    bool ok;

    MountedDevice triggerEntry = triggeringCombo->currentEntry();
    if (!triggerEntry.uuid.isEmpty())
        if (!path.startsWith(triggerEntry.mountPoint))
            message = "Path not present in triggering device";

    if (dirInfo.isDir() && dirInfo.isWritable()) {

    } else {
        if (! dirInfo.exists() )
            message = "Directory does not exist";
        else if (!dirInfo.isDir())
            message = "This is not a directory";
        else if (!dirInfo.isWritable())
            message = "Directory is not writable";
    }

    if (message.isEmpty()) {
        ui->label_DestinationWarning->clear();
    } else {
        ui->label_DestinationWarning->setText(message);
    }
}


void MainWindow::on_action_New_triggered()
{
    if (checkSave() != QMessageBox::Cancel) {
        NewBackupTaskDialog dialog(appContext, this, NewBackupTaskDialog::CreateOnly);
        if ( dialog.exec() == QDialog::Accepted) {
            openTask(dialog.result.id);
        }
    }
}

/**
 * @brief MainWindow::checkSave
 *
 * Conditionally save a task. If there non-persisted modifications ask user what to do with them. He/she can either
 * save them or ignore the operation.
 *
 * @return QMessageBox::X status or -1 if no dialog was presented
 */
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


void MainWindow::editTask(const QString& taskid)
{
    qInfo() << "will now open task " << taskid;
    if (checkSave() != QMessageBox::Cancel) {
        //NewBackupTaskDialog dialog(appContext, this, NewBackupTaskDialog::OpenOnly);
        //if (dialog.exec() == QDialog::Accepted) {
        openTask(taskid);
        //}
    }
}


void MainWindow::on_pushButtonRefreshBasePaths_clicked()
{
    MountedDevice triggerEntry;
    Triggering::triggerEntryForTask(activeBackup->backupDetails.tmp.taskId, triggerEntry);
    QList<MountedDevice> triggerEntries;
    DbusUtils::getMountedDevices(triggerEntries);
    triggeringCombo->refresh(triggerEntries, triggerEntry);
}


void MainWindow::_triggerEntrySelected(MountedDevice newTriggerEntry)
{
    Triggering::disableMountTrigger(activeBackup->backupDetails.tmp.taskId);
    if (!newTriggerEntry.uuid.isEmpty())
    {
        Triggering::enableMountTrigger(activeBackup->backupDetails.tmp.taskId, newTriggerEntry);
    }
    emit ui->lineEditDestinationSuffixPath->textChanged(ui->lineEditDestinationSuffixPath->text()); // simulate text change to trigger effect
}


void MainWindow::on_action_Save_triggered()
{
    applyChanges();
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


// show newtask dialog, get name and id, persist to .task file, reload and populate UI
void MainWindow::newBackupTaskFromDialog(qint32 dialogMode)
{
    QString stringResult;
    NewBackupTaskDialog dialog(appContext, this, (NewBackupTaskDialog::Mode) dialogMode);
    if ( dialog.exec() == QDialog::Accepted) {
        qInfo() << "dialog returned: " << dialog.result.name << " - " << dialog.result.id;

        if (checkSave() != QMessageBox::Cancel) {
            // store to task file
            BackupModel model;
            model.backupDetails.friendlyName = dialog.result.name;
            Tasks::saveTask(dialog.result.id, model);

            // reload task file and init UI
            BackupModel persisted;
            if (taskLoader->loadTask(dialog.result.id,persisted)) {
                *activeBackup = persisted;
                activeBackup->backupDetails.tmp.taskId = dialog.result.id;
                initUIControls(*activeBackup);
            }
        }
    }
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

void MainWindow::on_radioButtonAuto_toggled(bool checked)
{
    if (checked) {
        SourceDetails* sourcep = getSelectedSourceDetails();
        if (sourcep)
            sourcep->actionType = SourceDetails::automatic;

        emit actionChanged(SourceDetails::automatic);
    }
}

// higher-level handler. Model (getSelectedSourceDetails()) is assumed to contain the updated value
void MainWindow::on_actionChanged(SourceDetails::ActionType action) {
    // TODO
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

void MainWindow::runActiveTask()
{
    TaskRunnerManager* taskRunnerHelper = appContext->taskRunnerManager;
    taskRunnerHelper->runTask(taskName, Common::TaskRunnerReason::Manual);
}


// assumes there is an source item selected
void MainWindow::swapSources(BackupModel::SourceDetailsIndex sourceIndex1, BackupModel::SourceDetailsIndex sourceIndex2)
{
    // make sure index1 is smaller and index2
    BackupModel::SourceDetailsIndex toIndex = sourceIndex2; // sourceIndex2 is the "target" index. We keep it to re-select
    if (sourceIndex2 < sourceIndex1)
    {
        BackupModel::SourceDetailsIndex tmp = sourceIndex2;
        sourceIndex2 = sourceIndex1;
        sourceIndex1 = tmp;
    }

    SourceDetails source = activeBackup->allSourceDetails[sourceIndex1];
    activeBackup->allSourceDetails[sourceIndex1] = activeBackup->allSourceDetails[sourceIndex2];
    activeBackup->allSourceDetails[sourceIndex2] = source;

    QList<QStandardItem*> rowItems1 = sourcesModel->takeRow(sourceIndex1);
    QList<QStandardItem*> rowItems2 = sourcesModel->takeRow(sourceIndex2-1);
    sourcesModel->insertRow(sourceIndex1,rowItems2);
    sourcesModel->insertRow(sourceIndex2,rowItems1);

    // restore selected item
    ui->sourcesListView->selectionModel()->clearSelection();
    QModelIndex newCurrent = ui->sourcesListView->model()->index(toIndex,0);
    ui->sourcesListView->selectionModel()->select( newCurrent, QItemSelectionModel::Select );
    emit sourceChanged(newCurrent);
}


void MainWindow::on_lineEditContainsFilename_textEdited(const QString &newText)
{
    SourceDetails* sourcep = getSelectedSourceDetails();
    if (sourcep)
        sourcep->containsFilename = newText;
}


void MainWindow::on_lineEditNameMatches_textEdited(const QString &newText)
{
    SourceDetails* sourcep = getSelectedSourceDetails();
    if (sourcep)
        sourcep->nameMatches = newText;

}


void MainWindow::on_toolButtonSourceUp_clicked()
{
    const QItemSelection selection = ui->sourcesListView->selectionModel()->selection();
    if (!selection.isEmpty()) {
        QModelIndex i = selection.indexes().first();
        qDebug() << "will move up " << i.data();
        BackupModel::SourceDetailsIndex iSourceDetails = i.row();
        if (iSourceDetails > 0) // is there any space above ?
            swapSources(iSourceDetails, iSourceDetails-1);
    }
}


void MainWindow::on_toolButtonSourceDown_clicked()
{
    const QItemSelection selection = ui->sourcesListView->selectionModel()->selection();
    if (!selection.isEmpty()) {
        QModelIndex i = selection.indexes().first();
        qDebug() << "will move down " << i.data();
        BackupModel::SourceDetailsIndex iSourceDetails = i.row();
        if (iSourceDetails < ui->sourcesListView->model()->rowCount()-1) // is there any space above ?
            swapSources(iSourceDetails, iSourceDetails+1);
    }
}


void MainWindow::on_actionSe_ttings_triggered()
{
    SettingsDialog dialog(this);
    dialog.exec();
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    QMetaObject::invokeMethod(this, "afterWindowShown", Qt::ConnectionType::QueuedConnection);
}

void MainWindow::afterWindowShown()
{
    if (taskName.isEmpty() && !newBackupTaskDialogShown)
    {
        newBackupTaskDialogShown = true;
        // show newtask dialog, retrieve name, create task file and show
        NewBackupTaskDialog dialog(appContext, this, NewBackupTaskDialog::Wizard);
        if ( dialog.exec() == QDialog::Accepted) {
            openTask(dialog.result.id);
        }
    }
}


void MainWindow::on_pushButtonChooseDestinationSubdir_clicked()
{
    MountedDevice triggerEntry = triggeringCombo->currentEntry();
    QString dir = QFileDialog::getExistingDirectory(this, "Choose destination directory", triggerEntry.mountPoint.isEmpty() ? "/home" : triggerEntry.mountPoint, QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty())
    {
        ui->lineEditDestinationSuffixPath->setText(dir);
    }

}


void MainWindow::on_pushButtonSaveTask_clicked()
{
    applyChanges();
}


void MainWindow::on_action_ManageTasks_triggered()
{
    emit showTaskManagerTriggered(this->taskName);

}


void MainWindow::on_toolButtonAdd_clicked()
{
    MultipleDirDialog dialog(this);
    if ( dialog.exec() == QDialog::Accepted) {

        for (int i=0; i < dialog.selectedPaths.size(); i++)
        {
            SourceDetails sourceDetails;
            sourceDetails.sourcePath = dialog.selectedPaths[i];
            activeBackup->allSourceDetails.append(sourceDetails);
            BackupModel::SourceDetailsIndex sourceDetailsIndex = activeBackup->allSourceDetails.size()-1; // points to last item added
            ui->sourcesListView->selectionModel()->setCurrentIndex(sourcesModel->indexFromItem(appendSource(sourceDetailsIndex)), QItemSelectionModel::ClearAndSelect);
        }
    }
}

