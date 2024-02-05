#ifndef QTOOLBUTTONANIMATED_H
#define QTOOLBUTTONANIMATED_H

#include <QToolButton>
#include <QObject>
#include <QGraphicsOpacityEffect>


class QSequentialAnimationGroup;

class QToolButtonAnimated : public QToolButton
{
    Q_OBJECT
public:
    explicit QToolButtonAnimated(QWidget *parent = nullptr);
    ~QToolButtonAnimated();

public:
    QSequentialAnimationGroup* animation;
};

#endif // QTOOLBUTTONANIMATED_H
