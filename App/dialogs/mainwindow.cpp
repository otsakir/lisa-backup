#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "newbackuptaskdialog.h"
#include "aboutdialog.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QSystemTrayIcon>
#include <QStandardItemModel>
#include <QDebug>
#include <QDesktopServices>

#include "../utils.h"
#include "../task.h"
#include "../core.h"
#include "../logging.h"
#include "../dbusutils.h"
#include "../triggering.h"
#include "../appcontext.h"
#include "../settings.h"
#include "../taskrunnermanager.h"
#include "components/triggeringcombobox.h"
#include "components/taskmanager.h"
#include <components/sourcedetailsview.h>
#include "components/multipledirdialog.h"


MainWindow::MainWindow(QString openingTaskName, AppContext* appContext, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , appContext(appContext)
    , settingsDialog(new SettingsDialog(this))

{
    QSettings settings;

    ui->setupUi(this);
    taskLoader = appContext->getTaskLoader();
    initButtonIcons();
    trivialWiring();

    // set up models for sources listview
    sourcesModel = new QStandardItemModel(0,2, this);
    ui->sourcesListView->setModel(sourcesModel);
    ui->sourcesListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QItemSelectionModel* selectionModel = ui->sourcesListView->selectionModel();

    // Task Manager - create component and do wiring
    taskManager = new TaskManager(appContext, this);
    static_cast<QVBoxLayout*>(ui->tasksWrap->layout())->insertWidget(0, taskManager);
    connect(taskManager, &TaskManager::editTask, this, &MainWindow::editTask);
    connect(taskManager, &TaskManager::runTask, appContext->taskRunnerManager, &TaskRunnerManager::runTask); // run backup tasks from taskManager
    connect(taskManager, &TaskManager::newTask, this, &MainWindow::on_action_New_triggered);
    connect(this, &MainWindow::newTaskCreated, taskManager, &TaskManager::refreshView);
    connect(taskManager, &TaskManager::taskRemoved, [this](const QString taskid){
        qDebug() << "Task " << taskid << " removed";
        if (taskName() == taskid)
            ui->stackedWidget->setCurrentIndex(1); // hide "edit task" controls and show badge with user message
    });

    connect(selectionModel, &QItemSelectionModel::currentRowChanged, this, &MainWindow::sourceChanged);
    connect(this, &MainWindow::sourceChanged, this, &MainWindow::updateSourceDetails);
    connect(this, &MainWindow::PleaseQuit, QCoreApplication::instance(), QCoreApplication::quit, Qt::QueuedConnection); // 'PleaseQuit' signal bound to application quit
    connect(ui->actionE_xit, &QAction::triggered, this, &MainWindow::PleaseQuit);
    connect(this, &MainWindow::newTaskCreated, this, &MainWindow::editTask);
    connect(ui->lineEditDestinationSuffixPath, &QLineEdit::textChanged, this, &MainWindow::checkLineEditDestinationSuffixPath);
    connect(settingsDialog, &SettingsDialog::trayIconUpdate, [this] (bool show) {
        showTrayIcon(show);
    });
    connect(this, &MainWindow::gotClean, this, &MainWindow::onDirtyChanged);
    connect(this, &MainWindow::gotDirty, this, &MainWindow::onDirtyChanged);

    triggeringCombo = new TriggeringComboBox(this);
    triggeringCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    static_cast<QHBoxLayout*>(ui->horizontalLayout_Triggering->layout())->insertWidget(0, triggeringCombo);
    connect(triggeringCombo, &TriggeringComboBox::triggerEntrySelected, this, &MainWindow::_triggerEntrySelected);

    sourceDetails = new SourceDetailsView(this);
    ui->splitterSources->insertWidget(1, sourceDetails);
    connect(sourceDetails, &SourceDetailsView::gotDirty, this, &MainWindow::onModelUpdated);
    connect(this, &MainWindow::gotClean, sourceDetails, &SourceDetailsView::clearDirty);
    connect(ui->sourcesListView->model(), &QAbstractItemModel::rowsInserted, this, &MainWindow::onModelUpdated);
    connect(ui->sourcesListView->model(), &QAbstractItemModel::rowsRemoved, this, &MainWindow::onModelUpdated);

    activeBackup = new BackupModel();

    // init tray icon
    createTrayIcon();
    bool initiallyShowTrayIcon = (settings.value(Settings::Keys::KeepRunningInTray).toInt() != 0);
    showTrayIcon(initiallyShowTrayIcon);

    // if a task was given, open it
    if (!openingTaskName.isEmpty())
        openTask(openingTaskName);
    else
        ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    if (trayIconShown())
    {
        hide();
        event->ignore();
    } else
    {
        if (checkSave() == QMessageBox::Cancel) {
            event->ignore();
        } else {
            event->accept();
            emit MainWindow::PleaseQuit();
        }
    }
}

QString MainWindow::taskName()
{
    assert(activeBackup != nullptr);
    return activeBackup -> backupDetails.tmp.taskId;
}

bool MainWindow::openTask(QString taskId)
{
    assert(!taskId.isEmpty());

    BackupModel persisted;
    if (taskLoader->loadTask(taskId,persisted))
    {
        *activeBackup = persisted;
        activeBackup->backupDetails.tmp.taskId = taskId;
        initUIControls(*activeBackup);
        state.modelCopy = *activeBackup;
        return true;
    }

    qCritical() << "[critical] error loading task " << taskId;
    return false;
}


MainWindow::~MainWindow()
{
    delete activeBackup;
    delete sourcesModel;
    Logging::setUiConsole(0);
    delete ui;
}

void MainWindow::onModelUpdated()
{
    if (!dirty)
    {
        dirty = true;
        emit gotDirty(dirty);
    }
}

void MainWindow::onDirtyChanged(bool isdirty)
{
    QString title = this->windowTitle();
    if (isdirty)
        title.append("*");
    else
        title = title.replace("*", "");
    this->setWindowTitle(title);
    ui->toolButtonSaveTask->setEnabled(isdirty);
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

// when a different source list item is selected, update the source details view
void MainWindow::updateSourceDetails(QModelIndex rowIndex)
{
    if (rowIndex.isValid())
    {
        int row = rowIndex.row();
        QString sourcePath = rowIndex.data().toString();
        SourceDetails* pDetails = &activeBackup->allSourceDetails[row];
        sourceDetails->setDetails(pDetails);
        return;
    }
    sourceDetails->setDetails(nullptr);
}


void MainWindow::removeSelectedSourceFromList()
{
    const QItemSelection selection = ui->sourcesListView->selectionModel()->selection();
    if (!selection.isEmpty()) {
        QModelIndex i = selection.indexes().first();
        //qDebug() << "will remove " << i.data();
        BackupModel::SourceDetailsIndex iSourceDetails = i.row();
        sourcesModel->removeRow(i.row());
        activeBackup->allSourceDetails.remove(iSourceDetails);
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
    {
        QModelIndex index = sourcesModel->indexFromItem(lastSourceAdded);
        ui->sourcesListView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
        emit sourceChanged(index);
    }
    else
        emit sourceChanged(QModelIndex()); // list is empty

    // set up triggering combo
    QList<MountedDevice> availableTriggerEntries;
    DbusUtils::getMountedDevices(availableTriggerEntries);
    MountedDevice taskTriggerEntry; // triggering entry for the current task
    Triggering::triggerEntryForTask(backupModel.backupDetails.tmp.taskId, taskTriggerEntry);
    triggeringCombo->refresh(availableTriggerEntries, taskTriggerEntry);

    dirty = false;
    emit gotClean(dirty);

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
    dirty = false;
    emit gotClean(dirty);

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
        createNewTaskDialog();
    }
}

void MainWindow::createNewTaskDialog()
{
    NewBackupTaskDialog dialog(appContext, this, NewBackupTaskDialog::CreateOnly);
    if ( dialog.exec() == QDialog::Accepted) {
        // task has been created
        emit newTaskCreated(dialog.result.id);
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

    //if (! (state.modelCopy == (*activeBackup)))
    if (dirty)  // if there have been any unsaved changed in the model, ask for saving first
    {
        int ret = QMessageBox::warning(this, "Unsaved changes",tr("The backup task has been modified.\nDo you want to save your changes first?"),QMessageBox::Save | QMessageBox::No, QMessageBox::Save);
        if (ret == QMessageBox::Save) {
            applyChanges();
        }
        return ret;
    }
    return -1;
}


void MainWindow::editTask(const QString& taskid)
{
    if (checkSave() != QMessageBox::Cancel) {
        if (openTask(taskid))
        {
            ui->stackedWidget->setCurrentIndex(0);
            ui->labelTaskHeading->setText(QString("[%1]").arg(taskid));
            taskManager->setBoldListEntry(taskid);
        }
    }
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


void MainWindow::showAboutDialog()
{
    AboutDialog dialog(this);
    dialog.exec();
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


void MainWindow::moveSourceItemUp()
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


void MainWindow::moveSourceItemDown()
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


void MainWindow::showSettingsDialog()
{
    settingsDialog->exec();
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    QMetaObject::invokeMethod(this, "afterWindowShown", Qt::ConnectionType::QueuedConnection);
}

void MainWindow::afterWindowShown()
{
    if (taskManager->taskCount() == 0)
    {
        newBackupTaskDialogShown = true;
        // show newtask dialog, retrieve name, create task file and show
        NewBackupTaskDialog dialog(appContext, this, NewBackupTaskDialog::Wizard);
        if ( dialog.exec() == QDialog::Accepted) {
            emit newTaskCreated(dialog.result.id);
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

// opens the closest existing directory to destination path in an external file-manager window
void MainWindow::openDestinationDirExternal()
{
    QFileInfo destinationDir(ui->lineEditDestinationSuffixPath->text());
    QDesktopServices::openUrl( QUrl::fromLocalFile(destinationDir.path()) ); // path() returns the closest existing path to the destination directory
}


void MainWindow::askUserAndAddSources()
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


void MainWindow::createTrayIcon()
{
    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

}

void MainWindow::showTrayIcon(bool show)
{
    trayIcon->setVisible(show);
}

bool MainWindow::trayIconShown()
{
    if (trayIcon) return trayIcon->isVisible();

    return false;
}

void MainWindow::initButtonIcons()
{
    ui->toolButtonSaveTask->setIcon(QIcon(":/custom-icons/save.svg"));
    ui->toolButtonAddSource->setIcon(QIcon(":/custom-icons/folder-plus.svg"));
    ui->toolButtonRemoveSource->setIcon(QIcon(":/custom-icons/folder-minus.svg"));
    ui->toolButtonSourceUp->setIcon(QIcon(":/custom-icons/chevron-up.svg"));
    ui->toolButtonSourceDown->setIcon(QIcon(":/custom-icons/chevron-down.svg"));
    ui->toolButtonOpenDirExternal->setIcon(QIcon(":/custom-icons/to-external.svg"));
}

void MainWindow::trivialWiring()
{
    connect(ui->toolButtonSaveTask, &QToolButton::clicked, this, &MainWindow::applyChanges);
    connect(ui->toolButtonAddSource, &QToolButton::clicked, this, &MainWindow::askUserAndAddSources);
    connect(ui->toolButtonRemoveSource, &QToolButton::clicked, this, &MainWindow::removeSelectedSourceFromList);
    connect(ui->toolButtonSourceUp, &QToolButton::clicked, this, &MainWindow::moveSourceItemUp);
    connect(ui->toolButtonSourceDown, &QToolButton::clicked, this, &MainWindow::moveSourceItemDown);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::applyChanges);
    connect(ui->toolButtonOpenDirExternal, &QToolButton::clicked, this, &MainWindow::openDestinationDirExternal);
}

