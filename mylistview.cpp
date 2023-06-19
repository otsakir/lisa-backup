#include "mylistview.h"

#include <QDebug>

MyListView::MyListView(QWidget *parent) : QListView(parent)
{}

void MyListView::mouseDoubleClickEvent(QMouseEvent *event) {
    //qInfo() << "MyListView:: on double click!";
    emit doubleClicked( QModelIndex() );
}

MyListView::~MyListView() {
//    qInfo() << "destroy MyListView";
}
