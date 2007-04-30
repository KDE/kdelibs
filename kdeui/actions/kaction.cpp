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

#include "kapplication.h"
#include <kdebug.h>

#include "kglobalaccel.h"
#include "kguiitem.h"
#include "kgesturemap.h"
#include "kicon.h"
//---------------------------------------------------------------------
// KActionPrivate
//---------------------------------------------------------------------

class KActionPrivate
{
public:
  KActionPrivate()
    : globalShortcutAllowed(false), q(0)
  {
  }

  void slotTriggered();

  void init(KAction *q_ptr);

  KShortcut globalShortcut, defaultGlobalShortcut;
  KShapeGesture shapeGesture, defaultShapeGesture;
  KRockerGesture rockerGesture, defaultRockerGesture;

  bool globalShortcutAllowed;
  KAction *q;
};

void KActionPrivate::init(KAction *q_ptr)
{
  q = q_ptr;

  QObject::connect(q, SIGNAL(triggered(bool)), q, SLOT(slotTriggered()));

  q->setProperty("isShortcutConfigurable", true);
}

//---------------------------------------------------------------------
// KAction
//---------------------------------------------------------------------

KAction::KAction(QObject *parent)
  : QWidgetAction(parent), d(new KActionPrivate)
{
  d->init(this);
}

KAction::KAction(const QString &text, QObject *parent)
  : QWidgetAction(parent), d(new KActionPrivate)
{
  d->init(this);
  setText(text);
}

KAction::KAction(const KIcon &icon, const QString &text, QObject *parent)
  : QWidgetAction(parent), d(new KActionPrivate)
{
  d->init(this);
  setIcon(icon);
  setText(text);
}

KAction::~KAction()
{
    if( !d->globalShortcut.isEmpty()) {
        d->globalShortcutAllowed = false;
        KGlobalAccel::self()->updateGlobalShortcutAllowed(this); // unregister
    }
    KGestureMap::self()->removeGesture(d->shapeGesture, this);
    KGestureMap::self()->removeGesture(d->rockerGesture, this);
    delete d;
}

bool KAction::isShortcutConfigurable() const
{
    return property("isShortcutConfigurable").toBool();
}

void KAction::setShortcutConfigurable( bool b )
{
    setProperty("isShortcutConfigurable", b);
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
      setShortcuts(shortcut);
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

void KActionPrivate::slotTriggered()
{
#ifdef KDE3_SUPPORT
  emit q->activated();
#endif
  emit q->triggered(QApplication::mouseButtons(), QApplication::keyboardModifiers());
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

  if (!d->globalShortcutAllowed) {
    d->globalShortcutAllowed = true;
    KGlobalAccel::self()->updateGlobalShortcutAllowed(this);
  }

  if (type & DefaultShortcut)
    d->defaultGlobalShortcut = shortcut;

  if ((type & ActiveShortcut) && d->globalShortcut != shortcut) {
    KShortcut oldCut = d->globalShortcut;
    d->globalShortcut = shortcut;

    KGlobalAccel::self()->updateGlobalShortcut(this, oldCut);
  }
}

//private
void KAction::setActiveGlobalShortcutNoEnable(const KShortcut &cut)
{
  if (d->globalShortcut != cut) {
    KShortcut oldCut = d->globalShortcut;
    d->globalShortcut = cut;

    KGlobalAccel::self()->updateGlobalShortcut(this, oldCut);
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
    KGlobalAccel::self()->updateGlobalShortcutAllowed(this);
  }
}

KShapeGesture KAction::shapeGesture( ShortcutTypes type ) const
{
  Q_ASSERT(type);
  if ( type & DefaultShortcut )
    return d->defaultShapeGesture;

  return d->shapeGesture;
}

KRockerGesture KAction::rockerGesture( ShortcutTypes type ) const
{
  Q_ASSERT(type);
  if ( type & DefaultShortcut )
    return d->defaultRockerGesture;

  return d->rockerGesture;
}

void KAction::setShapeGesture( const KShapeGesture& gest,  ShortcutTypes type )
{
  Q_ASSERT(type);

  if( type & DefaultShortcut )
    d->defaultShapeGesture = gest;

  if ( type & ActiveShortcut ) {
    if ( KGestureMap::self()->findAction( gest ) ) {
      kDebug(283) << k_funcinfo << "New mouse gesture already in use, won't change gesture." << endl;
      return;
    }
    KGestureMap::self()->removeGesture( d->shapeGesture, this );
    KGestureMap::self()->addGesture( gest, this );
    d->shapeGesture = gest;
  }
}

void KAction::setRockerGesture( const KRockerGesture& gest,  ShortcutTypes type )
{
  Q_ASSERT(type);

  if( type & DefaultShortcut )
    d->defaultRockerGesture = gest;

  if ( type & ActiveShortcut ) {
    if ( KGestureMap::self()->findAction( gest ) ) {
      kDebug(283) << k_funcinfo << "New mouse gesture already in use, won't change gesture." << endl;
      return;
    }
    KGestureMap::self()->removeGesture( d->rockerGesture, this );
    KGestureMap::self()->addGesture( gest, this );
    d->rockerGesture = gest;
  }
}

/* vim: et sw=2 ts=2
 */

#include "kaction.moc"
