#include "listviewsources.h"

#include <QKeyEvent>
#include <QDebug>

ListViewSources::ListViewSources(QWidget *parent) : QListView(parent)
{}

void ListViewSources::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete)
    {
        emit deleteKeyPressed();
    } else {
        QListView::keyPressEvent(event);
    }
}



