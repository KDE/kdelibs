/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>
              (C) 2005-2006 Hamish Rodda <rodda@kde.org>

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

#include "ktoggleaction.h"

#include <kdebug.h>
#include <kguiitem.h>
#include <klocale.h>

class KToggleAction::KToggleActionPrivate
{
public:
  KToggleActionPrivate()
    : m_checkedGuiItem(0L)
  {}

  ~KToggleActionPrivate()
  {
    delete m_checkedGuiItem;
  }

  KGuiItem* m_checkedGuiItem;
};


KToggleAction::KToggleAction( KActionCollection * parent, const QString& name, QActionGroup * exclusiveGroup )
    : KAction(parent, name)
{
  init();
  setActionGroup(exclusiveGroup);
}

KToggleAction::KToggleAction( const QString & text, KActionCollection * parent, const QString& name, QActionGroup * exclusiveGroup )
    : KAction(text, parent, name)
{
  init();
  setActionGroup(exclusiveGroup);
}

KToggleAction::KToggleAction( const KIcon & icon, const QString & text, KActionCollection * parent, const QString& name, QActionGroup * exclusiveGroup )
    : KAction(icon, text, parent, name)
{
  init();
  setActionGroup(exclusiveGroup);
}

KToggleAction::KToggleAction( const QString & icon, const QString & text, KActionCollection * parent, const QString& name, QActionGroup * exclusiveGroup )
    : KAction(icon, text, parent, name)
{
  init();
  setActionGroup(exclusiveGroup);
}

KToggleAction::KToggleAction( const QString& text, const KShortcut& cut,
                              KActionCollection* parent,
                              const QString& name )
    : KAction( text, cut, 0,0,parent, name )
{
  init();
}

KToggleAction::KToggleAction( const QString& text, const KShortcut& cut,
                              const QObject* receiver, const char* slot,
                              KActionCollection* parent, const QString& name )
  : KAction( text, cut, receiver, slot, parent, name )
{
  init();
}

KToggleAction::KToggleAction( const QString& text, const QIcon& pix,
                              const KShortcut& cut,
                              KActionCollection* parent, const QString& name )
  : KAction( text, pix, cut, 0,0,parent, name )
{
  init();
}

KToggleAction::KToggleAction( const QString& text, const QString& pix,
                              const KShortcut& cut,
                              KActionCollection* parent, const QString& name )
 : KAction( text, pix, cut, 0,0,parent, name )
{
  init();
}

KToggleAction::KToggleAction( const QString& text, const QIcon& pix,
                              const KShortcut& cut,
                              const QObject* receiver,
                              const char* slot, KActionCollection* parent,
                              const QString& name )
  : KAction( text, pix, cut, receiver, slot, parent, name )
{
  init();
}

KToggleAction::KToggleAction( const QString& text, const QString& pix,
                              const KShortcut& cut,
                              const QObject* receiver,
                              const char* slot, KActionCollection* parent,
                              const QString& name )
  : KAction( text, pix, cut, receiver, slot, parent, name )
{
  init();
}

KToggleAction::~KToggleAction()
{
  delete d;
}

void KToggleAction::init()
{
  d = new KToggleActionPrivate;
  setCheckable(true);
  connect(this, SIGNAL(toggled(bool)), SLOT(slotToggled(bool)));
}

void KToggleAction::setCheckedState( const KGuiItem& checkedItem )
{
  delete d->m_checkedGuiItem;
  d->m_checkedGuiItem = new KGuiItem( checkedItem );
}

void KToggleAction::slotToggled( bool checked )
{
  if (d->m_checkedGuiItem) {
    QString string = d->m_checkedGuiItem->text();
    d->m_checkedGuiItem->setText(text());
    setText(string);

    string = d->m_checkedGuiItem->toolTip();
    d->m_checkedGuiItem->setToolTip(toolTip());
    setToolTip(string);

    if (d->m_checkedGuiItem->hasIconSet()) {
      QIcon icon = d->m_checkedGuiItem->iconSet();
      d->m_checkedGuiItem->setIcon(QAction::icon());
      QAction::setIcon(icon);
    }
  }
}

/* vim: et sw=2 ts=2
 */

#include "ktoggleaction.moc"
