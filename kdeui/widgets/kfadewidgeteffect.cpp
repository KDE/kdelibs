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

#include "kfadewidgeteffect.h"
#include "kfadewidgeteffect_p.h"
#include <QtCore/QEvent>
#include <QtGui/QPainter>

#include <kglobalsettings.h>

KFadeWidgetEffectPrivate::KFadeWidgetEffectPrivate(QWidget *_destWidget)
    : destWidget(_destWidget), disabled(false)
{
}

KFadeWidgetEffect::KFadeWidgetEffect(QWidget *destWidget)
    : QWidget(destWidget ? destWidget->parentWidget() : 0),
    d_ptr(new KFadeWidgetEffectPrivate(destWidget))
{
    Q_D(KFadeWidgetEffect);
    d->q_ptr = this;
    Q_ASSERT(destWidget && destWidget->parentWidget());
    if (!destWidget || !destWidget->parentWidget() || !destWidget->isVisible() ||
        !(KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects)) {
        d->disabled = true;
        hide();
        return;
    }
    setGeometry(QRect(destWidget->mapTo(parentWidget(), QPoint(0, 0)), destWidget->size()));
    d->oldPixmap = QPixmap::grabWidget(destWidget);
    d->timeLine.setFrameRange(0, 255);
    connect(&d->timeLine, SIGNAL(finished()), SLOT(finished()));
    connect(&d->timeLine, SIGNAL(frameChanged(int)), SLOT(repaint()));
    show();
}

KFadeWidgetEffect::~KFadeWidgetEffect()
{
    delete d_ptr;
}

void KFadeWidgetEffectPrivate::finished()
{
    Q_Q(KFadeWidgetEffect);
    destWidget->setUpdatesEnabled(false);
    q->hide();
    q->deleteLater();
    destWidget->setUpdatesEnabled(true);
}

void KFadeWidgetEffect::start(int duration)
{
    Q_D(KFadeWidgetEffect);
    if (d->disabled) {
        deleteLater();
        return;
    }
    d->timeLine.setDuration(duration);
    d->timeLine.start();
}

void KFadeWidgetEffect::paintEvent(QPaintEvent *)
{
    Q_D(KFadeWidgetEffect);
    QPainter p(this);
    p.setOpacity(1.0 - d->timeLine.currentValue());
    p.drawPixmap(rect(), d->oldPixmap);
}

#include "moc_kfadewidgeteffect.cpp"
