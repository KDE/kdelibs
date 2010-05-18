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
#include "kglobalaccel_p.h"
#include "klocale.h"
#include "kmessagebox.h"
#include "kauthaction.h"
#include "kauthactionwatcher.h"

#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QShortcutEvent>
#include <QtGui/QToolBar>

#include <kdebug.h>

#include "kguiitem.h"
#include "kicon.h"

//---------------------------------------------------------------------
// KActionPrivate
//---------------------------------------------------------------------

void KActionPrivate::init(KAction *q_ptr)
{
  q = q_ptr;
  globalShortcutEnabled = false;
  neverSetGlobalShortcut = true;

  QObject::connect(q, SIGNAL(triggered(bool)), q, SLOT(slotTriggered()));

  q->setProperty("isShortcutConfigurable", true);
}

void KActionPrivate::setActiveGlobalShortcutNoEnable(const KShortcut &cut)
{
    globalShortcut = cut;
    emit q->globalShortcutChanged(cut.primary());
}


void KActionPrivate::slotTriggered()
{
#ifdef KDE3_SUPPORT
  emit q->activated();
#endif
  emit q->triggered(QApplication::mouseButtons(), QApplication::keyboardModifiers());

  if (authAction) {
      KAuth::Action::AuthStatus s = authAction->earlyAuthorize();
      switch(s) {
      case KAuth::Action::Denied:
          q->setEnabled(false);
          break;
      case KAuth::Action::Authorized:
          emit q->authorized(authAction);
          break;
      default:
          break;
      }
  }
}

void KActionPrivate::authStatusChanged(int status)
{
    KAuth::Action::AuthStatus s = (KAuth::Action::AuthStatus)status;

    switch(s) {
        case KAuth::Action::Authorized:
            q->setEnabled(true);
            if(!oldIcon.isNull()) {
                q->setIcon(oldIcon);
                oldIcon = KIcon();
            }
            break;
        case KAuth::Action::AuthRequired:
            q->setEnabled(true);
            oldIcon = KIcon(q->icon());
            q->setIcon(KIcon("dialog-password"));
            break;
        default:
            q->setEnabled(false);
            if(!oldIcon.isNull()) {
                q->setIcon(oldIcon);
                oldIcon = KIcon();
            }
    }
}

bool KAction::event(QEvent *event)
{
    if (event->type() == QEvent::Shortcut) {
        QShortcutEvent *se = static_cast<QShortcutEvent*>(event);
        if(se->isAmbiguous()) {
            KMessageBox::information(
                    NULL,  // No widget to be seen around here
                    i18n( "The key sequence '%1' is ambiguous. Use 'Configure Shortcuts'\n"
                          "from the 'Settings' menu to solve the ambiguity.\n"
                          "No action will be triggered.",
                                se->key().toString()),
                    i18n("Ambiguous shortcut detected"));
            return true;
        }
    }

    return QAction::event(event);
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
    if (d->globalShortcutEnabled) {
        // - remove the action from KGlobalAccel
        d->globalShortcutEnabled = false;
        KGlobalAccel::self()->d->remove(this, KGlobalAccelPrivate::SetInactive);
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
      QAction::setShortcuts(shortcut);
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

void KAction::setShortcuts(const QList<QKeySequence>& shortcuts, ShortcutTypes type)
{
  setShortcut(KShortcut(shortcuts), type);
}

const KShortcut & KAction::globalShortcut(ShortcutTypes type) const
{
  Q_ASSERT(type);

  if (type == DefaultShortcut)
    return d->defaultGlobalShortcut;

  return d->globalShortcut;
}

void KAction::setGlobalShortcut( const KShortcut & shortcut, ShortcutTypes type,
                                 GlobalShortcutLoading load )
{
  Q_ASSERT(type);
  bool changed = false;

  // protect against garbage keycode -1 that Qt sometimes produces for exotic keys;
  // at the moment (~mid 2008) Multimedia PlayPause is one of those keys.
  int shortcutKeys[8];
  for (int i = 0; i < 4; i++) {
    shortcutKeys[i] = shortcut.primary()[i];
    shortcutKeys[i + 4] = shortcut.alternate()[i];
  }
  for (int i = 0; i < 8; i++) {
    if (shortcutKeys[i] == -1) {
      kWarning(283) << "Encountered garbage keycode (keycode = -1) in input, not doing anything.";
      return;
    }
  }

  if (!d->globalShortcutEnabled) {
    changed = true;
    if (objectName().isEmpty() || objectName().startsWith(QLatin1String("unnamed-"))) {
      kWarning(283) << "Attempt to set global shortcut for action without objectName()."
                       " Read the setGlobalShortcut() documentation.";
      return;
    }
    d->globalShortcutEnabled = true;
    KGlobalAccel::self()->d->doRegister(this);
  }

  if ((type & DefaultShortcut) && d->defaultGlobalShortcut != shortcut) {
    d->defaultGlobalShortcut = shortcut;
    changed = true;
  }

  if ((type & ActiveShortcut) && d->globalShortcut != shortcut) {
    d->globalShortcut = shortcut;
    changed = true;
  }

  //We want to have updateGlobalShortcuts called on a new action in any case so that
  //it will be registered properly. In the case of the first setShortcut() call getting an
  //empty shortcut parameter this would not happen...
  if (changed || d->neverSetGlobalShortcut) {
    KGlobalAccel::self()->d->updateGlobalShortcut(this, type | load);
    d->neverSetGlobalShortcut = false;
  }
}

bool KAction::globalShortcutAllowed() const
{
  return d->globalShortcutEnabled;
}

bool KAction::isGlobalShortcutEnabled() const
{
  return d->globalShortcutEnabled;
}

void KAction::setGlobalShortcutAllowed( bool allowed, GlobalShortcutLoading /* load */ )
{
  if (allowed) {
      //### no-op
  } else {
      forgetGlobalShortcut();
  }
}

void KAction::forgetGlobalShortcut()
{
    d->globalShortcut = KShortcut();
    d->defaultGlobalShortcut = KShortcut();
    if (d->globalShortcutEnabled) {
        d->globalShortcutEnabled = false;
        d->neverSetGlobalShortcut = true;   //it's a fresh start :)
        KGlobalAccel::self()->d->remove(this, KGlobalAccelPrivate::UnRegister);
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
      kDebug(283) << "New mouse gesture already in use, won't change gesture.";
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
      kDebug(283) << "New mouse gesture already in use, won't change gesture.";
      return;
    }
    KGestureMap::self()->removeGesture( d->rockerGesture, this );
    KGestureMap::self()->addGesture( gest, this );
    d->rockerGesture = gest;
  }
}

void KAction::setHelpText(const QString& text)
{
    setStatusTip(text);
    setToolTip(text);
    if (whatsThis().isEmpty())
        setWhatsThis(text);
}

KAuth::Action *KAction::authAction() const
{
    return d->authAction;
}

void KAction::setAuthAction(const QString &actionName)
{
    if (actionName.isEmpty()) {
        setAuthAction(0);
    } else {
        setAuthAction(new KAuth::Action(actionName));
    }
}

void KAction::setAuthAction(KAuth::Action *action)
{
    if (d->authAction == action) {
        return;
    }
  
    if (d->authAction) {
        disconnect(d->authAction->watcher(), SIGNAL(statusChanged(int)),
                this, SLOT(authStatusChanged(int)));
        //delete d->authAction;
        d->authAction = 0;
        if (!d->oldIcon.isNull()) {
            setIcon(d->oldIcon);
            d->oldIcon = KIcon();
        }
    }

    if (action != 0) {
        d->authAction = action;
        connect(d->authAction->watcher(), SIGNAL(statusChanged(int)),
                this, SLOT(authStatusChanged(int)));
        d->authStatusChanged(d->authAction->status());
    }
}

/* vim: et sw=2 ts=2
 */

#include "kaction.moc"
