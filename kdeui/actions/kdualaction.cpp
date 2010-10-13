/* This file is part of the KDE libraries
    Copyright (c) 2010 Aurélien Gâteau <agateau@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "kdualaction.h"

#include <QIcon>

#include <kdebug.h>

#include "kdualaction_p.h"

inline bool boolForState(KDualAction::State state)
{
    return state == KDualAction::ActiveState;
}

//---------------------------------------------------------------------
// KDualActionPrivate
//---------------------------------------------------------------------
void KDualActionPrivate::init(KDualAction *q_ptr)
{
    q = q_ptr;
    autoToggle = true;
    isActive = false;

    QObject::connect(q, SIGNAL(triggered()), q, SLOT(slotTriggered()));
}

void KDualActionPrivate::updateFromCurrentState()
{
    KGuiItem& currentItem = item(isActive);
    QAction* qq = static_cast<QAction*>(q);
    qq->setIcon(currentItem.icon());
    qq->setText(currentItem.text());
    qq->setToolTip(currentItem.toolTip());
}

void KDualActionPrivate::slotTriggered()
{
    if (!autoToggle) {
        return;
    }
    q->setActive(!isActive);
    q->activeChangedByUser(isActive);
}

//---------------------------------------------------------------------
// KDualAction
//---------------------------------------------------------------------
KDualAction::KDualAction(const QString &inactiveText, const QString &activeText, QObject *parent)
: KAction(parent)
, d(new KDualActionPrivate)
{
    d->init(this);
    d->item(false).setText(inactiveText);
    d->item(true).setText(activeText);
    d->updateFromCurrentState();
}

KDualAction::KDualAction(QObject *parent)
: KAction(parent)
, d(new KDualActionPrivate)
{
    d->init(this);
}

KDualAction::~KDualAction()
{
    delete d;
}

void KDualAction::setGuiItemForState(KDualAction::State state, const KGuiItem &item)
{
    d->item(state) = item;
    if (boolForState(state) == d->isActive) {
        d->updateFromCurrentState();
    }
}

KGuiItem KDualAction::guiItemForState(KDualAction::State state) const
{
    return d->item(state);
}

void KDualAction::setTextForState(KDualAction::State state, const QString &text)
{
    d->item(state).setText(text);
    if (boolForState(state) == d->isActive) {
        d->updateFromCurrentState();
    }
}

QString KDualAction::textForState(KDualAction::State state) const
{
    return d->item(state).text();
}

void KDualAction::setIconForState(KDualAction::State state, const QIcon &icon)
{
    d->item(state).setIcon(KIcon(icon));
    if (boolForState(state) == d->isActive) {
        d->updateFromCurrentState();
    }
}

void KDualAction::setIconForStates(const QIcon &icon)
{
    setIconForState(KDualAction::InactiveState, icon);
    setIconForState(KDualAction::ActiveState, icon);
}

QIcon KDualAction::iconForState(KDualAction::State state) const
{
    return d->item(state).icon();
}

void KDualAction::setToolTipForState(KDualAction::State state, const QString &toolTip)
{
    d->item(state).setToolTip(toolTip);
    if (boolForState(state) == d->isActive) {
        d->updateFromCurrentState();
    }
}

QString KDualAction::toolTipForState(KDualAction::State state) const
{
    return d->item(state).toolTip();
}

void KDualAction::setAutoToggle(bool value)
{
    d->autoToggle = value;
}

bool KDualAction::autoToggle() const
{
    return d->autoToggle;
}

void KDualAction::setActive(bool active)
{
    if (active == d->isActive) {
        return;
    }
    d->isActive = active;
    d->updateFromCurrentState();
    activeChanged(active);
}

bool KDualAction::isActive() const
{
    return d->isActive;
}

#include "kdualaction.moc"
