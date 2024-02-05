#include "qtoolbuttonanimated.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

#include <QDebug>

QToolButtonAnimated::QToolButtonAnimated(QWidget *parent)
    : QToolButton(parent)
{
    animation = new QSequentialAnimationGroup(this);

    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(this);
    QPropertyAnimation* fadeoutAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeoutAnimation->setDuration(1000);
    fadeoutAnimation->setStartValue(1);
    fadeoutAnimation->setEndValue(0.2);
    fadeoutAnimation->setEasingCurve(QEasingCurve::InQuad);

    QPropertyAnimation* fadeinAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    fadeinAnimation->setDuration(1000);
    fadeinAnimation->setStartValue(0.2);
    fadeinAnimation->setEndValue(1);
    fadeinAnimation->setEasingCurve(QEasingCurve::OutQuad);

    // fade-out and fade-in in a row
    // TODO check memory handling of child animations. Who will release ?
    animation->addAnimation(fadeoutAnimation);
    animation->addAnimation(fadeinAnimation);

    animation->setLoopCount(-1); // recycle
    connect(animation, &QSequentialAnimationGroup::finished, [this, opacityEffect]() {
        opacityEffect->setOpacity(1.0);
    });

    setGraphicsEffect(opacityEffect);
    setMinimumSize(32,32);
}

QToolButtonAnimated::~QToolButtonAnimated()
{}
