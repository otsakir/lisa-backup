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

private:
    Ui::SourceDetailsView *ui;

    SourceDetails* sourceDetails = nullptr; // Data model. Not owned by this class.
    void initControls();
    void signalWiring();

private slots:
    // low level

    // higher level
    void on_methodChanged(SourceDetails::BackupType method);

signals:
    void methodChanged(SourceDetails::BackupType method);
    void actionChanged(SourceDetails::ActionType actionType);
    void containsFilenameChanged(const QString& newtext);
    void nameMatchedChanged(const QString& newtext);
    void backupDepthChanged(SourceDetails::BackupDepth depth);


};

#endif // SOURCEDETAILSVIEW_H
