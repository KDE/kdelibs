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
#include "kaction_p.h"
#include "klocalizedstring.h"
#include "kauthobjectdecorator.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QShortcutEvent>
#include <QToolBar>

#include <kdebug.h>

//---------------------------------------------------------------------
// KActionPrivate
//---------------------------------------------------------------------

void KActionPrivate::init(KAction *q_ptr)
{
  q = q_ptr;

  QObject::connect(q, SIGNAL(triggered(bool)), q, SLOT(slotTriggered()));

  q->setProperty("isShortcutConfigurable", true);

  decorator = new KAuth::ObjectDecorator(q);
  QObject::connect(decorator, SIGNAL(authorized(KAuth::Action)),
                   q, SIGNAL(authorized(KAuth::Action)));
  QObject::connect(KGlobalAccel::self(), SIGNAL(globalShortcutChanged(QAction*,const QKeySequence&)),
        q, SLOT(_k_emitActionGlobalShortcutChanged(QAction*,const QKeySequence&)));
}

void KActionPrivate::slotTriggered()
{
#ifdef KDE3_SUPPORT
  emit q->activated();
#endif
  emit q->triggered(QApplication::mouseButtons(), QApplication::keyboardModifiers());
}

void KActionPrivate::_k_emitActionGlobalShortcutChanged(QAction *action, const QKeySequence &seq)
{
    if (action == q) {
        // reemit the legacy KAction::globalShortcutChanged
        // TODO: completely remove this method when KAction::globalShortcutChanged signal will be removed
        emit q->globalShortcutChanged(seq);
    }
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

KAction::KAction(const QIcon &icon, const QString &text, QObject *parent)
  : QWidgetAction(parent), d(new KActionPrivate)
{
  d->init(this);
  setIcon(icon);
  setText(text);
}

KAction::~KAction()
{
    KGestureMap::self()->removeAllGestures(this);
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
      QList<QKeySequence> shortcuts = property("defaultShortcuts").value<QList<QKeySequence> >();
      return KShortcut(shortcuts);
  }

  QKeySequence primary = shortcuts().value(0);
  QKeySequence secondary = shortcuts().value(1);
  return KShortcut(primary, secondary);
}

void KAction::setShortcut( const KShortcut & shortcut, ShortcutTypes type )
{
  Q_ASSERT(type);

  if (type & DefaultShortcut) {
      setProperty("defaultShortcuts", QVariant::fromValue(shortcut.toList()));
  }

  if (type & ActiveShortcut) {
      QAction::setShortcuts(shortcut);
  }
}

void KAction::setShortcut( const QKeySequence & keySeq, ShortcutTypes type )
{
  Q_ASSERT(type);

  if (type & DefaultShortcut)
      setProperty("defaultShortcuts", QVariant::fromValue(QList<QKeySequence>() << keySeq));

  if (type & ActiveShortcut) {
      QAction::setShortcut(keySeq);
  }
}

void KAction::setShortcuts(const QList<QKeySequence>& shortcuts, ShortcutTypes type)
{
  setShortcut(KShortcut(shortcuts), type);
}

KShortcut KAction::globalShortcut(ShortcutTypes type) const
{
  Q_ASSERT(type);

  if (type == DefaultShortcut)
    return KGlobalAccel::self()->defaultShortcut(this);

  return KGlobalAccel::self()->shortcut(this);
}

void KAction::setGlobalShortcut( const KShortcut & shortcut, ShortcutTypes type,
                                 GlobalShortcutLoading load )
{
  Q_ASSERT(type);

  if ((type & DefaultShortcut) && globalShortcut(DefaultShortcut) != shortcut) {
    KGlobalAccel::self()->setDefaultShortcut(this, shortcut,
                                             static_cast<KGlobalAccel::GlobalShortcutLoading>(load));
  }

  if ((type & ActiveShortcut) && globalShortcut(ActiveShortcut) != shortcut) {
    KGlobalAccel::self()->setShortcut(this, shortcut,
                                      static_cast<KGlobalAccel::GlobalShortcutLoading>(load));
  }
}

#ifndef KDE_NO_DEPRECATED
bool KAction::globalShortcutAllowed() const
{
  return isGlobalShortcutEnabled();
}
#endif

bool KAction::isGlobalShortcutEnabled() const
{
  return KGlobalAccel::self()->hasShortcut(this);
}

#ifndef KDE_NO_DEPRECATED
void KAction::setGlobalShortcutAllowed( bool allowed, GlobalShortcutLoading /* load */ )
{
  if (allowed) {
      //### no-op
  } else {
      forgetGlobalShortcut();
  }
}
#endif

void KAction::forgetGlobalShortcut()
{
    if (isGlobalShortcutEnabled()) {
        KGlobalAccel::self()->removeAllShortcuts(this);
    }
}

KShapeGesture KAction::shapeGesture( ShortcutTypes type ) const
{
    Q_ASSERT(type);
    if ( type & DefaultShortcut ) {
        return KGestureMap::self()->defaultShapeGesture(this);
    } else {
        return KGestureMap::self()->shapeGesture(this);
    }
}

KRockerGesture KAction::rockerGesture( ShortcutTypes type ) const
{
    Q_ASSERT(type);
    if (type & DefaultShortcut) {
        return KGestureMap::self()->defaultRockerGesture(this);
    } else {
        return KGestureMap::self()->rockerGesture(this);
    }
}

void KAction::setShapeGesture( const KShapeGesture& gest,  ShortcutTypes type )
{
  Q_ASSERT(type);

  if (type & DefaultShortcut) {
    KGestureMap::self()->setDefaultShapeGesture(this, gest);
  }

  if (type & ActiveShortcut) {
    if ( KGestureMap::self()->findAction( gest ) ) {
      kDebug(283) << "New mouse gesture already in use, won't change gesture.";
      return;
    }
    KGestureMap::self()->setShapeGesture(this, gest);
  }
}

void KAction::setRockerGesture( const KRockerGesture& gest,  ShortcutTypes type )
{
  Q_ASSERT(type);

  if( type & DefaultShortcut ) {
    KGestureMap::self()->setDefaultRockerGesture(this, gest);
  }

  if ( type & ActiveShortcut ) {
    if ( KGestureMap::self()->findAction( gest ) ) {
      kDebug(283) << "New mouse gesture already in use, won't change gesture.";
      return;
    }
    KGestureMap::self()->setRockerGesture(this, gest);
  }
}

void KAction::setHelpText(const QString& text)
{
    setStatusTip(text);
    setToolTip(text);
    if (whatsThis().isEmpty())
        setWhatsThis(text);
}

KAuth::Action KAction::authAction() const
{
    return d->decorator->authAction();
}

void KAction::setAuthAction(const QString &actionName)
{
    d->decorator->setAuthAction(actionName);
}

void KAction::setAuthAction(const KAuth::Action &action)
{
    d->decorator->setAuthAction(action);
}

/* vim: et sw=2 ts=2
 */

#include "moc_kaction.cpp"
