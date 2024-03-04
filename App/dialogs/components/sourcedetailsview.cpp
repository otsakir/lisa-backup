#include "sourcedetailsview.h"
#include "ui_sourcedetailsview.h"


#include <QFileInfo>


SourceDetailsView::SourceDetailsView(QWidget *parent) :
    ui(new Ui::SourceDetailsView)
{
    ui->setupUi(this);
    signalWiring();
    setDetails(nullptr);
}

SourceDetailsView::~SourceDetailsView()
{
    delete ui;
}

void SourceDetailsView::signalWiring()
{
    connect(ui->radioButtonAll, &QRadioButton::toggled, [this] (bool checked) {
        if (checked) emit methodChanged(SourceDetails::all);
    });
    connect(ui->radioButtonSelective, &QRadioButton::toggled, [this] (bool checked) {
        if (checked) emit methodChanged(SourceDetails::selective);
    });
    connect(ui->radioButtonRsync, &QRadioButton::toggled, [this] (bool checked) {
        if (checked) emit actionChanged(SourceDetails::rsync);
    });
    connect(ui->radioButtonGitBundle, &QRadioButton::toggled, [this] (bool checked) {
        if (checked) emit actionChanged(SourceDetails::gitBundle);
    });
    connect(ui->radioButtonAuto, &QRadioButton::toggled, [this] (bool checked) {
        if (checked) emit actionChanged(SourceDetails::automatic);
    });
    connect(ui->lineEditContainsFilename, &QLineEdit::textEdited, [this] (const QString& newtext) {
        this->sourceDetails->containsFilename = newtext;
        emit containsFilenameChanged(newtext);
    });
    connect(ui->lineEditNameMatches, &QLineEdit::textEdited, [this] (const QString& newtext) {
        this->sourceDetails->nameMatches = newtext;
        emit nameMatchedChanged(newtext);
    });
    connect(ui->comboBoxDepth, QOverload<int>::of(&QComboBox::currentIndexChanged), [this] (int comboIndex) {
        this->sourceDetails->backupDepth = (SourceDetails::BackupDepth) comboIndex;
        emit backupDepthChanged(this->sourceDetails->backupDepth);
    });
    connect(ui->comboBoxPredicate, QOverload<int>::of(&QComboBox::currentIndexChanged), ui->stackedWidgetPredicate, &QStackedWidget::setCurrentIndex);
    connect(ui->comboBoxPredicate, QOverload<int>::of(&QComboBox::currentIndexChanged), [this] (int index) {
        this->sourceDetails->predicateType = (SourceDetails::PredicateType) index;
    });
    connect(this, &SourceDetailsView::methodChanged, this, &SourceDetailsView::on_methodChanged);
}



void SourceDetailsView::setDetails(SourceDetails *details)
{
    if (details == nullptr)
    {
        this->setEnabled(false);
        sourceDetails = details;
    } else
    {
        if (details != sourceDetails)
        {
            this->setEnabled(true);
            sourceDetails = details;
            initControls();
        }
    }
}


/**
 * When a source list item is clicked, populate UI (the right hand side of it) with sources details
 * param rowIndex: Points to the first item of the selected row or is an empty (invalid) index
 *
 */
void SourceDetailsView::initControls() {
    ui->comboBoxDepth->setCurrentIndex(sourceDetails->backupDepth);
    if (sourceDetails->backupType == SourceDetails::all) {
        ui->radioButtonAll->setChecked(true);
        //emit methodChanged(SourceDetails::all);
    }
    else if (sourceDetails->backupType == SourceDetails::selective) {
        ui->radioButtonSelective->setChecked(true);
        //emit methodChanged(SourceDetails::selective);
    }

    ui->radioButtonRsync->setChecked(sourceDetails->actionType == SourceDetails::rsync);
    ui->radioButtonGitBundle->setChecked(sourceDetails->actionType == SourceDetails::gitBundle);
    ui->radioButtonAuto->setChecked(sourceDetails->actionType == SourceDetails::automatic);
    //emit actionChanged(sourceDetails->actionType);

    ui->lineEditContainsFilename->setText(sourceDetails->containsFilename);
    ui->lineEditNameMatches->setText(sourceDetails->nameMatches);

    ui->comboBoxPredicate->setCurrentIndex(sourceDetails->predicateType);
    ui->stackedWidgetPredicate->setCurrentIndex(sourceDetails->predicateType);

    //ui->widgetSourceDetails->setHidden(false);

}



/*
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
*/

void SourceDetailsView::on_methodChanged(SourceDetails::BackupType method)
{
    this->sourceDetails->backupType = method;

    ui->groupBoxCriteria->setEnabled(method != SourceDetails::all);
    ui->groupBoxAction->setEnabled(method != SourceDetails::all);
}



