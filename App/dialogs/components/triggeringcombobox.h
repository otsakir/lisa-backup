#ifndef TRIGGERINGCOMBOBOX_H
#define TRIGGERINGCOMBOBOX_H

#include <QComboBox>
#include "../../core.h"


class TriggeringComboBox : public QComboBox
{
    Q_OBJECT
private:
    void appendBaseBath(const QString caption, MountedDevice triggerEntry, bool italic);
    QFont italicFont;

public:
    TriggeringComboBox(QWidget *parent = nullptr);
    void refresh(const QList<MountedDevice>& availableTriggerEntries, const MountedDevice& currentTriggerEntry);
    MountedDevice currentEntry();
    void printCombo();

signals:
    void triggerEntrySelected(MountedDevice triggerEntry);

private slots:
    void _currentIndexChanged(int index);

};

#endif // TRIGGERINGCOMBOBOX_H
