#ifndef MYLISTVIEW_H
#define MYLISTVIEW_H

#include <QListView>

class MyListView : public QListView
{
    Q_OBJECT
public:
    MyListView(QWidget *parent = nullptr);

    virtual void mouseDoubleClickEvent(QMouseEvent *event);


    ~MyListView();

};

#endif // MYLISTVIEW_H
