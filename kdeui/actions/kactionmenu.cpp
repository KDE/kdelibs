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

#include "kactionmenu.h"

#include <QToolButton>
#include <QToolBar>

#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>

class KActionMenuPrivate
{
public:
  KActionMenuPrivate()
  {
    m_delayed = true;
    m_stickyMenu = true;
  }
  ~KActionMenuPrivate()
  {
  }
  bool m_delayed;
  bool m_stickyMenu;
};

KActionMenu::KActionMenu(QObject *parent)
  : KAction(parent)
  , d(new KActionMenuPrivate)
{
  setShortcutConfigurable( false );
}

KActionMenu::KActionMenu(const QString &text, QObject *parent)
  : KAction(parent)
  , d(new KActionMenuPrivate)
{
  setShortcutConfigurable( false );
  setText(text);
}

KActionMenu::KActionMenu(const KIcon & icon, const QString & text, QObject *parent)
  : KAction(icon, text, parent)
  , d(new KActionMenuPrivate)
{
  setShortcutConfigurable( false );
}

KActionMenu::~KActionMenu()
{
    delete d;
    delete menu();
}

QWidget * KActionMenu::createWidget( QWidget * _parent )
{
  QToolBar *parent = qobject_cast<QToolBar *>(_parent);
  if (!parent)
    return KAction::createWidget(_parent);
  QToolButton* button = new QToolButton(parent);
  button->setAutoRaise(true);
  button->setFocusPolicy(Qt::NoFocus);
  button->setIconSize(parent->iconSize());
  button->setToolButtonStyle(parent->toolButtonStyle());
  QObject::connect(parent, SIGNAL(iconSizeChanged(const QSize&)),
                   button, SLOT(setIconSize(const QSize&)));
  QObject::connect(parent, SIGNAL(toolButtonStyleChanged(Qt::ToolButtonStyle)),
                   button, SLOT(setToolButtonStyle(Qt::ToolButtonStyle)));
  button->setDefaultAction(this);
  QObject::connect(button, SIGNAL(triggered(QAction*)), parent, SIGNAL(actionTriggered(QAction*)));

  if (delayed())
    button->setPopupMode(QToolButton::DelayedPopup);
  else if (stickyMenu())
    button->setPopupMode(QToolButton::InstantPopup);
  else
    button->setPopupMode(QToolButton::MenuButtonPopup);

  return button;
}

void KActionMenu::remove( KAction* cmd )
{
  if ( cmd )
    menu()->removeAction(cmd);
}

void KActionMenu::addAction( QAction * action )
{
  menu()->addAction(action);
}

QAction* KActionMenu::addSeparator()
{
  QAction* separator = new QAction(this);
  separator->setSeparator(true);
  addAction(separator);
  return separator;
}

QAction* KActionMenu::insertSeparator(QAction* before)
{
  QAction* separator = new QAction(this);
  separator->setSeparator(true);
  insertAction(before, separator);
  return separator;
}

void KActionMenu::insertAction( QAction * before, QAction * action )
{
  menu()->insertAction(before, action);
}

void KActionMenu::removeAction( QAction * action )
{
  menu()->removeAction(action);
}

bool KActionMenu::delayed() const {
    return d->m_delayed;
}

void KActionMenu::setDelayed(bool _delayed) {
    d->m_delayed = _delayed;
}

bool KActionMenu::stickyMenu() const {
    return d->m_stickyMenu;
}

void KActionMenu::setStickyMenu(bool sticky) {
    d->m_stickyMenu = sticky;
}

KMenu* KActionMenu::menu()
{
  if (!KAction::menu())
    setMenu(new KMenu());

  return qobject_cast<KMenu*>(KAction::menu());
}

void KActionMenu::setMenu(KMenu *menu)
{
    KAction::setMenu( menu );
}

/* vim: et sw=2 ts=2
 */

#include "kactionmenu.moc"
