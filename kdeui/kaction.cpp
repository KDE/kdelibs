/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2005 Hamish Rodda <rodda@kde.org>

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

#include <assert.h>

#include <kauthorized.h>
#include <qapplication.h>
#include <kdebug.h>
#include <kguiitem.h>

#include "kmainwindow.h"
#include "kmenubar.h"
#include "kmenu.h"
#include "ktoolbar.h"
#include "kicon.h"

/**
* How it works.
* KActionCollection is an organizing container for KActions.
* KActionCollection keeps track of the information necessary to handle
* configuration and shortcuts.
*
* Focus Widget pointer:
* This is the widget which is the focus for action shortcuts.
* It is set either by passing a QWidget* to the KActionCollection constructor
* or by calling setWidget() if the widget wasn't known when the object was
* initially constructed (as in KXMLGUIClient and KParts::PartBase)
*
* Shortcuts:
* An action's shortcut will not not be connected unless a focus widget has
* been specified in KActionCollection.
*
* XML Filename:
* This is used to save user-modified settings back to the *ui.rc file.
* It is set by KXMLGUIFactory.
*/

//---------------------------------------------------------------------
// KAction::KActionPrivate
//---------------------------------------------------------------------

class KAction::KActionPrivate
{
public:
  KActionPrivate()
  {
    m_configurable = true;
  }

  KShortcut m_shortcut;
  KShortcut m_defaultShortcut;

  bool m_configurable;
};

//---------------------------------------------------------------------
// KAction
//---------------------------------------------------------------------

KAction::KAction( KActionCollection * parent, const char* name )
  : QAction(parent), d(new KActionPrivate)
{
  initPrivate(name);
}

KAction::KAction( const QString & text, KActionCollection * parent, const char* name )
  : QAction(text, parent), d(new KActionPrivate)
{
  initPrivate(name);
}

KAction::KAction( const KIcon & icon, const QString & text, KActionCollection * parent, const char* name )
  : QAction(icon, text, parent), d(new KActionPrivate)
{
  initPrivate(name);
}

KAction::KAction( const QString & icon, const QString & text, KActionCollection * parent, const char* name )
  : QAction(text, parent), d(new KActionPrivate)
{
  initPrivate(name);
  setIconName( icon );
}

KAction::KAction( const QString& text, const KShortcut& cut,
             const QObject* receiver, const char* slot,
             KActionCollection* parent, const char* name )
: QAction( text, parent ), d(new KActionPrivate)
{
  initPrivate( cut, receiver, slot, name );
}

KAction::KAction( const QString& text, const QString& sIconName, const KShortcut& cut,
  const QObject* receiver, const char* slot,
  KActionCollection* parent, const char* name )
: QAction(text, parent), d(new KActionPrivate)
{
  initPrivate( cut, receiver, slot, name );
  setIconName( sIconName );
}

KAction::KAction( const QString& text, const QIcon& icon, const KShortcut& cut,
  const QObject* receiver, const char* slot,
  KActionCollection* parent, const char* name )
: QAction(icon, text, parent), d(new KActionPrivate)
{
  initPrivate( cut, receiver, slot, name );
}

KAction::KAction( const KGuiItem& item, const KShortcut& cut,
  const QObject* receiver, const char* slot,
  KActionCollection* parent, const char* name )
: QAction(item.text(), parent), d(new KActionPrivate)
{
  initPrivate( cut, receiver, slot, name );
  if( item.hasIcon() )
    setIconName( item.iconName() );
  setToolTip( item.toolTip() );
  setWhatsThis( item.whatsThis() );
}

KAction::~KAction()
{
    if (KActionCollection* ac = parentCollection())
        ac->take( this );

    delete d;
}

void KAction::initPrivate(const char* name)
{
    QAction::setObjectName( QLatin1String( name ) );

    if (!KAuthorized::authorizeKAction(name)) {
      // Disable this action
      setEnabled(false);
      setVisible(false);
      blockSignals(true);
    }

    if (KActionCollection* ac = parentCollection())
        ac->insert( this );

    connect(this, SIGNAL(triggered(bool)), SLOT(slotTriggered()));
}

void KAction::initPrivate( const KShortcut& cut,
                  const QObject* receiver, const char* slot, const char* name )
{
    initPrivate(name);

    d->m_defaultShortcut = cut;

    if ( receiver && slot )
        connect( this, SIGNAL( activated() ), receiver, slot );

    if( !cut.isNull() && objectName().isEmpty() )
        kWarning(129) << "KAction::initPrivate(): trying to assign a shortcut (" << cut.toStringInternal() << ") to an unnamed action." << endl;

    KAction::setShortcut( cut );
}

const KShortcut& KAction::defaultShortcut() const
{
  return d->m_defaultShortcut;
}

bool KAction::isShortcutConfigurable() const
{
  return d->m_configurable;
}

void KAction::setShortcutConfigurable( bool b )
{
    d->m_configurable = b;
}

bool KAction::hasIcon() const
{
  return !icon().isNull();
}

KActionCollection *KAction::parentCollection() const
{
    return qobject_cast<KActionCollection*>(const_cast<QObject*>(parent()));
}

void KAction::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KAction::setIconName( const QString & icon )
{
  setIcon(KIcon(icon));
}

const KShortcut & KAction::shortcut( ) const
{
  return d->m_shortcut;
}

QString KAction::shortcutText() const
{
  return shortcut().toString();
}

void KAction::setShortcut( const KShortcut & shortcut )
{
  d->m_shortcut = shortcut;
  QAction::setShortcut(shortcut.seq(0).qt());

  if (shortcut.count() > 1) {
    QList<QKeySequence> alternateShortcuts;
    for (uint i = 1; i < shortcut.count(); ++i)
      alternateShortcuts.append(shortcut.seq(i).qt());
    setAlternateShortcuts(alternateShortcuts);
  }
}

void KAction::setShortcutText(const QString& shortcutText)
{
  return setShortcut(KShortcut(shortcutText));
}

void KAction::slotTriggered()
{
#ifdef KDE3_SUPPORT
  emit activated();
#endif
  emit triggered(QApplication::mouseButtons(), QApplication::keyboardModifiers());
}

int KAction::plug( QWidget * widget, int index )
{
  QAction* before = 0L;
  if (index > 0 && index < widget->actions().count())
    before = widget->actions().at(index - 1);
  widget->insertAction(before, this);
  return widget->actions().lastIndexOf(this);
}

void KAction::unplug( QWidget * w )
{
  w->removeAction(this);
}

bool KAction::isPlugged( ) const
{
  return associatedWidgets().count();
}

bool KAction::isPlugged( QWidget * widget ) const
{
  return associatedWidgets().contains(widget);
}

void KAction::unplugAll()
{
  QList<QWidget*> aw = associatedWidgets();

  foreach (QWidget* widget, aw)
    widget->removeAction(this);
}

int KAction::containerCount() const
{
  return associatedWidgets().count();
}

QWidget* KAction::container( int index ) const
{
  if (index >= 0 && index < associatedWidgets().count())
    return associatedWidgets().value(index);

  return 0L;
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

/* vim: et sw=2 ts=2
 */

#include "kaction.moc"
