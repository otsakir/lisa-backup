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
#include "components/listviewsources.h"


MainWindow::MainWindow(bool startInTray,QString openingTaskName, AppContext* appContext, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , appContext(appContext)
    , settingsDialog(new SettingsDialog(this))

{
    QSettings settings;

    ui->setupUi(this);
    setWindowTitle(Lb::windowTitle("woof!"));
    setWindowIcon(QIcon(":/custom-icons/backup-icon.svg"));

    taskLoader = appContext->getTaskLoader();
    initButtonIcons();
    trivialWiring();

    // set up models for sources listview
    sourcesModel = new QStandardItemModel(0,2, this);
    sourcesListView = new ListViewSources(this);
    sourcesListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    static_cast<QHBoxLayout*>(ui->verticalLayout_sourceListWrap->layout())->insertWidget(0, sourcesListView);
    sourcesListView->setModel(sourcesModel);
    sourcesListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QItemSelectionModel* selectionModel = sourcesListView->selectionModel();

    // Task Manager - create component and do wiring
    taskManager = new TaskManager(appContext, this);
    static_cast<QVBoxLayout*>(ui->tasksWrap->layout())->insertWidget(0, taskManager);
    connect(taskManager, &TaskManager::editTask, this, &MainWindow::editTask);
    connect(taskManager, &TaskManager::runTask, this, &MainWindow::checkSaveAndRun);
    // run backup tasks from taskManager
    connect(taskManager, &TaskManager::newTask, this, &MainWindow::on_action_New_triggered);
    connect(this, &MainWindow::newTaskCreated, taskManager, &TaskManager::refreshView);
    connect(taskManager, &TaskManager::taskRemoved, [this](const QString taskid){
        qDebug() << "Task " << taskid << " removed";
        if (taskName() == taskid)
            ui->stackedWidget->setCurrentIndex(1); // hide "edit task" controls and show badge with user message
    });
    connect(this, &MainWindow::taskNowEdited, taskManager, &TaskManager::taskIsNowEdited);

    connect(selectionModel, &QItemSelectionModel::currentRowChanged, this, &MainWindow::sourceChanged);
    connect(this, &MainWindow::sourceChanged, this, &MainWindow::updateSourceDetails);
    connect(this, &MainWindow::PleaseQuit, QCoreApplication::instance(), QCoreApplication::quit, Qt::QueuedConnection); // 'PleaseQuit' signal bound to application quit
    connect(ui->actionE_xit, &QAction::triggered, this, &MainWindow::PleaseQuit);
    connect(this, &MainWindow::newTaskCreated, this, &MainWindow::editTask);
    connect(ui->lineEditDestinationPath, &QLineEdit::textChanged, this, &MainWindow::checkLineEditDestinationPath);
    connect(settingsDialog, &SettingsDialog::trayIconUpdate, [this] (bool show) {
        showTrayIcon(show);
    });
    connect(this, &MainWindow::gotClean, this, &MainWindow::onDirtyChanged);
    connect(this, &MainWindow::gotDirty, this, &MainWindow::onDirtyChanged);
    connect(settingsDialog, &SettingsDialog::autoStartUpdate, [this] (bool autoStart) {
        if (autoStart)
            Lb::createDesktopFile();
        else
            Lb::removeDesktopFile();
    });

    triggeringCombo = new TriggeringComboBox(this);
    triggeringCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    static_cast<QHBoxLayout*>(ui->horizontalLayout_Triggering->layout())->insertWidget(0, triggeringCombo);
    connect(triggeringCombo, &TriggeringComboBox::triggerEntrySelected, this, &MainWindow::_triggerEntrySelected);

    sourceDetails = new SourceDetailsView(this);
    ui->splitterSources->insertWidget(1, sourceDetails);
    connect(sourceDetails, &SourceDetailsView::gotDirty, this, &MainWindow::onModelUpdated);
    connect(this, &MainWindow::gotClean, sourceDetails, &SourceDetailsView::clearDirty);
    connect(sourcesListView->model(), &QAbstractItemModel::rowsInserted, this, &MainWindow::onModelUpdated);
    connect(sourcesListView->model(), &QAbstractItemModel::rowsRemoved, this, &MainWindow::onModelUpdated);
    connect(ui->labelNoTaskSelected, &QLabel::linkActivated, this, &MainWindow::on_action_New_triggered);
    connect(sourcesListView, &ListViewSources::deleteKeyPressed, this, &MainWindow::removeSelectedSourceFromList);

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
    QItemSelectionModel* selModel = sourcesListView->selectionModel();

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
    const QItemSelection selection = sourcesListView->selectionModel()->selection();
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
    this->setWindowTitle( Lb::windowTitle(backupModel.backupDetails.tmp.taskId ));  //friendlyName) );
    ui->lineEditDestinationPath->setText(backupModel.backupDetails.destinationPath);

    sourcesModel->clear();

    QStandardItem* lastSourceAdded = 0;
    for (int i=0; i<backupModel.allSourceDetails.size(); i++) {
        lastSourceAdded = appendSource(i);
    }
    if (lastSourceAdded)
    {
        QModelIndex index = sourcesModel->indexFromItem(lastSourceAdded);
        sourcesListView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
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


void MainWindow::updatetDestinationPathModel(const QString &arg1)
{
    activeBackup->backupDetails.destinationPath = ui->lineEditDestinationPath->text();
    emit onModelUpdated();
}

void MainWindow::checkLineEditDestinationPath(const QString& newText)
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

void MainWindow::checkSaveAndRun(const QString taskname, Common::TaskRunnerReason reason, bool show)
{
    if (taskname == taskName() && dirty) // if task to run is the one that is currently edited
    {
        int result = QMessageBox::warning(this, "Unsaved changes", "The backup task has been modified.\nDo you want to save your changes first?", QMessageBox::Save | QMessageBox::No | QMessageBox::Cancel);
        if  (result == QMessageBox::Save)
        {
            applyChanges();
        } else
        if (result == QMessageBox::Cancel)
            return;
    }
    appContext->taskRunnerManager->runTask(taskname, reason, show);
}


void MainWindow::editTask(const QString& taskid)
{
    if (checkSave() != QMessageBox::Cancel) {
        if (openTask(taskid))
        {
            ui->stackedWidget->setCurrentIndex(0);
            ui->labelTaskHeading->setText(QString("[%1]").arg(taskid));
            //taskManager->setBoldListEntry(taskid);
            emit taskNowEdited(taskid);
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
    checkLineEditDestinationPath(ui->lineEditDestinationPath->text());
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
    sourcesListView->selectionModel()->clearSelection();
    QModelIndex newCurrent = sourcesListView->model()->index(toIndex,0);
    sourcesListView->selectionModel()->select( newCurrent, QItemSelectionModel::Select );
    emit sourceChanged(newCurrent);
}


void MainWindow::moveSourceItemUp()
{
    const QItemSelection selection = sourcesListView->selectionModel()->selection();
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
    const QItemSelection selection = sourcesListView->selectionModel()->selection();
    if (!selection.isEmpty()) {
        QModelIndex i = selection.indexes().first();
        qDebug() << "will move down " << i.data();
        BackupModel::SourceDetailsIndex iSourceDetails = i.row();
        if (iSourceDetails < sourcesListView->model()->rowCount()-1) // is there any space above ?
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

// TODO - remove this. It should be dead code.
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
    QString suggestedDestDir = suggestDestinationPath();
    QString dir = QFileDialog::getExistingDirectory(this, "Choose destination directory", suggestedDestDir, QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty())
    {
        ui->lineEditDestinationPath->setText(dir);
    }
}

QString MainWindow::suggestDestinationPath         ()
{
    MountedDevice triggerEntry = triggeringCombo->currentEntry();
    QString validPath;
    QString path;
    if (triggerEntry.mountPoint.isEmpty())
        path = ui->lineEditDestinationPath->text();
    else
    {
        if (ui->lineEditDestinationPath->text().startsWith(triggerEntry.mountPoint))
            path = ui->lineEditDestinationPath->text();
        else
            path = triggerEntry.mountPoint;
    }
    validPath = Lb::bestValidDirectoryMatch(path);

    return validPath;
}

// opens the closest existing directory to destination path in an external file-manager window
void MainWindow::openDestinationDirExternal()
{
    QSettings settings;
    QString validPath = suggestDestinationPath();

    QString commandTemplate = settings.value(Settings::Keys::ExternalFileManagerCommand).toString();
    if (!commandTemplate.isEmpty())
    {
        QString command = commandTemplate.replace("%p", validPath);
        QProcess process;
        process.startDetached(command);
    } else
    {
        QUrl  url = QUrl::fromLocalFile(validPath);
        QDesktopServices::openUrl( url ); // path() returns the closest existing path to the destination directory
    }
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
            sourcesListView->selectionModel()->setCurrentIndex(sourcesModel->indexFromItem(appendSource(sourceDetailsIndex)), QItemSelectionModel::ClearAndSelect);
        }
    }
}

void MainWindow::refreshTriggerEntries()
{
    MountedDevice triggerEntry;
    Triggering::triggerEntryForTask(activeBackup->backupDetails.tmp.taskId, triggerEntry);
    QList<MountedDevice> triggerEntries;
    DbusUtils::getMountedDevices(triggerEntries);
    triggeringCombo->refresh(triggerEntries, triggerEntry);
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
    trayIcon->setIcon(QIcon(":/custom-icons/backup-icon-tray.png"));
    connect(trayIcon, &QSystemTrayIcon::activated, this, &QWidget::showNormal); // show main window when just clicking on the tray icon
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
    connect(ui->lineEditDestinationPath, &QLineEdit::textChanged, this, &MainWindow::updatetDestinationPathModel);
    connect(ui->toolButtonRefreshTriggering, &QToolButton::clicked, this, &MainWindow::refreshTriggerEntries);
    connect(this, &MainWindow::taskSaved, appContext->globalSignals, &Common::GlobalSignals::taskModified);
}

