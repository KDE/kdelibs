/* This file is part of the KDE libraries
    Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kshortcutwidget.h"
#include "kiconloader.h"
#include "ui_kshortcutwidget.h"

class KShortcutWidgetPrivate
{
public:
    KShortcutWidgetPrivate(KShortcutWidget *q) : q(q) {}

//private slots
    void priKeySequenceChanged(const QKeySequence &);
    void altKeySequenceChanged(const QKeySequence &);

//members
    KShortcutWidget *const q;
    Ui::KShortcutWidget ui;
    KShortcut cut;
    bool holdChangedSignal;
};


KShortcutWidget::KShortcutWidget(QWidget *parent)
 : QWidget(parent),
   d(new KShortcutWidgetPrivate(this))
{
    d->holdChangedSignal = false;
    d->ui.setupUi(this);
    connect(d->ui.priEditor, SIGNAL(keySequenceChanged(const QKeySequence &)),
            this, SLOT(priKeySequenceChanged(const QKeySequence &)));
    connect(d->ui.altEditor, SIGNAL(keySequenceChanged(const QKeySequence &)),
            this, SLOT(altKeySequenceChanged(const QKeySequence &)));
}


KShortcutWidget::~KShortcutWidget()
{
  delete d;
}


void KShortcutWidget::setModifierlessAllowed(bool allow)
{
    d->ui.priEditor->setModifierlessAllowed(allow);
    d->ui.altEditor->setModifierlessAllowed(allow);
}


bool KShortcutWidget::isModifierlessAllowed()
{
    return d->ui.priEditor->isModifierlessAllowed();
}


void KShortcutWidget::setClearButtonsShown(bool show)
{
    d->ui.priEditor->setClearButtonShown(show);
    d->ui.altEditor->setClearButtonShown(show);
}


KShortcut KShortcutWidget::shortcut() const
{
    KShortcut ret;
    ret.setPrimary(d->ui.priEditor->keySequence());
    ret.setAlternate(d->ui.altEditor->keySequence());
    return ret;
}

void KShortcutWidget::setCheckActionList(const QList<QAction*> &checkList)
{
    d->ui.priEditor->setCheckActionList(checkList);
    d->ui.altEditor->setCheckActionList(checkList);
}

void KShortcutWidget::setCheckActionCollections(const QList<KActionCollection *>& actionCollections)
{
    d->ui.priEditor->setCheckActionCollections(actionCollections);
    d->ui.altEditor->setCheckActionCollections(actionCollections);
}

//slot
void KShortcutWidget::applyStealShortcut()
{
    d->ui.priEditor->applyStealShortcut();
    d->ui.altEditor->applyStealShortcut();
}


//slot
void KShortcutWidget::setShortcut(const KShortcut &newSc)
{
    if (newSc == d->cut)
        return;

    d->holdChangedSignal = true;
    d->ui.priEditor->setKeySequence(newSc.primary());
    d->ui.altEditor->setKeySequence(newSc.alternate());
    d->holdChangedSignal = false;

    emit shortcutChanged(d->cut);
}


//slot
void KShortcutWidget::clearShortcut()
{
    setShortcut(KShortcut());
}


//private slot
void KShortcutWidgetPrivate::priKeySequenceChanged(const QKeySequence &seq)
{
    cut.setPrimary(seq);
    if (!holdChangedSignal)
        emit q->shortcutChanged(cut);
}


//private slot
void KShortcutWidgetPrivate::altKeySequenceChanged(const QKeySequence &seq)
{
    cut.setAlternate(seq);
    if (!holdChangedSignal)
        emit q->shortcutChanged(cut);
}

#include "kshortcutwidget.moc"
