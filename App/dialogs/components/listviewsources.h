#ifndef LISTVIEWSOURCES_H
#define LISTVIEWSOURCES_H

#include <QListView>

class ListViewSources : public QListView
{
    Q_OBJECT
public:
    ListViewSources(QWidget* parent);

signals:
    void deleteKeyPressed();


protected:
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // LISTVIEWSOURCES_H
