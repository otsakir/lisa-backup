#include "triggeringcombobox.h"

#include <QStandardItem>
#include <QList>
#include "../../triggering.h"

#include <QDebug>

TriggeringComboBox::TriggeringComboBox(QWidget *parent)
    : QComboBox(parent)
{
    setModel(new QStandardItemModel(0,1)); // caption, data(MountedDevice)
    setModelColumn(0); // first column holds the "caption"
    QFont italicFont(this->font());
    italicFont.setItalic(true);
    this->italicFont = italicFont;

    connect(this, QOverload<int>::of(&TriggeringComboBox::currentIndexChanged),this, &TriggeringComboBox::_currentIndexChanged);
}

void TriggeringComboBox::appendBaseBath(const QString caption, MountedDevice triggerEntry, bool italic = false)
{
    QList<QStandardItem*> rowItems;

    QVariant triggerEntryVariant;
    triggerEntryVariant.setValue(triggerEntry);

    QStandardItem* item = new QStandardItem(caption);
    item->setData(triggerEntryVariant, Qt::UserRole+1);
    item->setData(triggerEntry.uuid, Qt::ToolTipRole);

    if (italic)
        item->setData(italicFont, Qt::FontRole);

    rowItems << item;
    blockSignals(true);
    static_cast<QStandardItemModel*>(model())->appendRow(rowItems);
    blockSignals(false);
}

void TriggeringComboBox::refresh(const QList<MountedDevice>& availableTriggerEntries, const MountedDevice& currentTriggerEntry)
{
    QList<MountedDevice> triggerEntries;
    Triggering::filterTriggerEntries(availableTriggerEntries, triggerEntries);

    clear();
    MountedDevice blankTriggerEntry;
    appendBaseBath("No triggering", blankTriggerEntry, true);

    int indexFound = -1; // assume not found
    int i = 1; // counter
    foreach( const MountedDevice& mountedDevice, triggerEntries)
    {
        if (!currentTriggerEntry.uuid.isEmpty() && currentTriggerEntry.uuid == mountedDevice.uuid) {
            indexFound = i;
        }

        QString caption = mountedDevice.mountPoint.isEmpty() ? "(not mounted)" : mountedDevice.mountPoint;
        if (!mountedDevice.label.isEmpty())
            caption.append(QString(" | label: %1").arg(mountedDevice.label));
        appendBaseBath(caption, mountedDevice);
        i++;
    }
    if (indexFound == -1 && !currentTriggerEntry.uuid.isEmpty())
    {
        appendBaseBath(QString("last  time mounted under %1").arg(currentTriggerEntry.mountPoint), currentTriggerEntry);
        i++;
        setCurrentIndex(i-1);
    } else
    if (indexFound == -1)
    {
        setCurrentIndex(0);
    } else
        setCurrentIndex(indexFound);
}

void TriggeringComboBox::_currentIndexChanged(int index)
{
    if (index >= 0)
    {
        QVariant variant = model()->data(model()->index(index,0),Qt::UserRole+1);
        MountedDevice triggerEntry = variant.value<MountedDevice>();
        emit triggerEntrySelected(triggerEntry);
    } else
        emit triggerEntrySelected(MountedDevice());
}

MountedDevice TriggeringComboBox::currentEntry()
{
    if (currentIndex() < 0)
        return MountedDevice();

    QVariant variant = this->currentData(Qt::UserRole+1);
    return variant.value<MountedDevice>();
}

void TriggeringComboBox::printCombo()
{

    QStandardItemModel* model = static_cast<QStandardItemModel*>(this->model());
    for (int j = 0; j<model->rowCount(); j++)
    {
        QModelIndex index = model->index(j,0);
        qDebug() << "item" << j << ":" << index.data().toString();
    }
}


