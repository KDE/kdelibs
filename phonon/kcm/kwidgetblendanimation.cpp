/*  This file is part of the KDE project
    Copyright (C) 2008 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "kwidgetblendanimation.h"
#include "kwidgetblendanimation_p.h"
#include <QtCore/QEvent>
#include <QtGui/QPainter>

KWidgetBlendAnimationPrivate::KWidgetBlendAnimationPrivate(QWidget *_destWidget)
    : destWidget(_destWidget)
{
}

KWidgetBlendAnimation::KWidgetBlendAnimation(QWidget *destWidget)
    : QWidget(destWidget ? destWidget->parentWidget() : 0),
    d_ptr(new KWidgetBlendAnimationPrivate(destWidget))
{
    Q_D(KWidgetBlendAnimation);
    d->q_ptr = this;
    Q_ASSERT(destWidget && destWidget->parentWidget());
    setGeometry(QRect(destWidget->mapTo(parentWidget(), QPoint(0, 0)), destWidget->size()));
    d->oldPixmap = QPixmap::grabWidget(destWidget);
    d->timeLine.setFrameRange(0, 255);
    connect(&d->timeLine, SIGNAL(finished()), SLOT(finished()));
    connect(&d->timeLine, SIGNAL(frameChanged(int)), SLOT(repaint()));
    show();
}

KWidgetBlendAnimation::~KWidgetBlendAnimation()
{
    delete d_ptr;
}

void KWidgetBlendAnimationPrivate::finished()
{
    Q_Q(KWidgetBlendAnimation);
    destWidget->setUpdatesEnabled(false);
    q->hide();
    q->deleteLater();
    destWidget->setUpdatesEnabled(true);
}

void KWidgetBlendAnimation::start(int duration)
{
    Q_D(KWidgetBlendAnimation);
    d->timeLine.setDuration(duration);
    d->timeLine.start();
}

void KWidgetBlendAnimation::paintEvent(QPaintEvent *)
{
    Q_D(KWidgetBlendAnimation);
    QPainter p(this);
    p.setOpacity(1.0 - d->timeLine.currentValue());
    p.drawPixmap(rect(), d->oldPixmap);
}

#include "moc_kwidgetblendanimation.cpp"
