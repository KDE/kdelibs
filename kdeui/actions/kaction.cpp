/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
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

#include "kaction.h"

#include <kauthorized.h>
#include "kapplication.h"
#include <kdebug.h>

#include "kactioncollection.h"
#include "kglobalaccel.h"
#include "kguiitem.h"
#include "kicon.h"
//---------------------------------------------------------------------
// KActionPrivate
//---------------------------------------------------------------------

class KActionPrivate
{
public:
  KActionPrivate()
    :globalShortcutAllowed(false)
  {
  }

  KShortcut globalShortcut, defaultGlobalShortcut;

  bool globalShortcutAllowed;
};

//---------------------------------------------------------------------
// KAction
//---------------------------------------------------------------------

KAction::KAction( KActionCollection * parent, const QString& name )
  : QWidgetAction(parent), d(new KActionPrivate)
{
  initPrivate(name);
}

KAction::KAction( const QString & text, KActionCollection * parent, const QString& name )
  : QWidgetAction(parent), d(new KActionPrivate)
{
  setText(text);
  initPrivate(name);
}

KAction::KAction( const KIcon & icon, const QString & text, KActionCollection * parent, const QString& name )
  : QWidgetAction(parent), d(new KActionPrivate)
{
  setIcon(icon);
  setText(text);
  initPrivate(name);
}

KAction::~KAction()
{
    if (KActionCollection* ac = parentCollection())
        ac->take( this );

    delete d;
}

void KAction::initPrivate(const QString& name)
{
    QAction::setObjectName(name);

    if (!KAuthorized::authorizeKAction(name)) {
      // Disable this action
      setEnabled(false);
      setVisible(false);
      blockSignals(true);
    }

    if (KActionCollection* ac = parentCollection())
        ac->insert( this );

    connect(this, SIGNAL(triggered(bool)), SLOT(slotTriggered()));

    setProperty("isShortcutConfigurable", true);
}

void KAction::initPrivate( const KShortcut& cut,
                  const QObject* receiver, const char* slot, const QString& name )
{
    initPrivate(name);

    if (receiver && slot)
        connect(this, SIGNAL(triggered(bool)), receiver, slot);

    setShortcut(cut);
}

bool KAction::isShortcutConfigurable() const
{
    return property("isShortcutConfigurable").toBool();
}

void KAction::setShortcutConfigurable( bool b )
{
    setProperty("isShortcutConfigurable", b);
}

KActionCollection *KAction::parentCollection() const
{
    return qobject_cast<KActionCollection*>(const_cast<QObject*>(parent()));
}

KShortcut KAction::shortcut(ShortcutTypes type) const
{
  Q_ASSERT(type);

  if (type == DefaultShortcut) {
      QKeySequence primary = property("defaultPrimaryShortcut").value<QKeySequence>();
      QKeySequence secondary = property("defaultAlternateShortcut").value<QKeySequence>();
      return KShortcut(primary, secondary);
  }

  QKeySequence primary = shortcuts().value(0);
  QKeySequence secondary = shortcuts().value(1);
  return KShortcut(primary, secondary);
}

void KAction::setShortcut( const KShortcut & shortcut, ShortcutTypes type )
{
  Q_ASSERT(type);

  if (type & DefaultShortcut) {
      setProperty("defaultPrimaryShortcut", shortcut.primary());
      setProperty("defaultAlternateShortcut", shortcut.alternate());
  }

  if (type & ActiveShortcut) {
      setShortcuts(shortcut.toList());
  }
}

void KAction::setShortcut( const QKeySequence & keySeq, ShortcutTypes type )
{
  Q_ASSERT(type);

  if (type & DefaultShortcut)
      setProperty("defaultPrimaryShortcut", keySeq);

  if (type & ActiveShortcut) {
      QAction::setShortcut(keySeq);
  }
}

void KAction::slotTriggered()
{
#ifdef KDE3_SUPPORT
  emit activated();
#endif
  emit triggered(QApplication::mouseButtons(), QApplication::keyboardModifiers());
}

void KAction::setName ( const char * )
{
  // Not allowed
  Q_ASSERT(false);
}

void KAction::setObjectName(const QString&)
{
  // Not allowed
  Q_ASSERT(false);
}

void KAction::setIcon( const KIcon & icon )
{
  QAction::setIcon(icon);
}

const KShortcut & KAction::globalShortcut(ShortcutTypes type) const
{
  Q_ASSERT(type);

  if (type == DefaultShortcut)
    return d->defaultGlobalShortcut;

  return d->globalShortcut;
}

void KAction::setGlobalShortcut( const KShortcut & shortcut, ShortcutTypes type )
{
  Q_ASSERT(type);

  d->globalShortcutAllowed = true;

  if (type & DefaultShortcut)
    d->defaultGlobalShortcut = shortcut;

  if ((type & ActiveShortcut) && d->globalShortcut != shortcut) {
    d->globalShortcut = shortcut;

    KGlobalAccel::self()->checkAction(this);
  }
}

bool KAction::globalShortcutAllowed( ) const
{
  return d->globalShortcutAllowed;
}

void KAction::setGlobalShortcutAllowed( bool allowed )
{
  if (d->globalShortcutAllowed != allowed) {
    d->globalShortcutAllowed = allowed;
    KGlobalAccel::self()->checkAction(this);
  }
}

KIcon KAction::icon( ) const
{
  return KIcon(QAction::icon());
}

/* vim: et sw=2 ts=2
 */

#include "kaction.moc"
