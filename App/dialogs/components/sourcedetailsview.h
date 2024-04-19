#ifndef SOURCEDETAILSVIEW_H
#define SOURCEDETAILSVIEW_H

#include <QWidget>
#include "../core.h"

namespace Ui {
class SourceDetailsView;
}


class SourceDetailsView : public QWidget
{
    Q_OBJECT

public:
    explicit SourceDetailsView(QWidget *parent = nullptr);
    ~SourceDetailsView();

public slots:
    void setDetails(SourceDetails* details); // call when data model changes. Pass nullptr is there is no data.
    void clearDirty(); // clear dirty status. Call when state has been persisted in parent

signals:
    void gotDirty(); // data model changed since initialization (setDetails())


private:
    Ui::SourceDetailsView *ui;

    SourceDetails* sourceDetails = nullptr; // Data model. Not owned by this class.
    bool dirty = false; // are there any changes in the data model since setDetails() was called ?
    void initControls();
    void signalWiring();

private slots:
    // higher level
    void on_methodChanged(SourceDetails::BackupType method);
    void updateActionType(SourceDetails::ActionType actionType);
    void checkSignalDirty(); // call when there is any change in the data model (sourceDetails)

signals:
    void methodChanged(SourceDetails::BackupType method);
    void actionChanged(SourceDetails::ActionType actionType);
    void containsFilenameChanged(const QString& newtext);
    void nameMatchedChanged(const QString& newtext);
    void backupDepthChanged(SourceDetails::BackupDepth depth);

};

#endif // SOURCEDETAILSVIEW_H
