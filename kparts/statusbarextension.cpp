/* This file is part of the KDE project
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2003 David Faure <faure@kde.org>

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

#include "statusbarextension.h"

#include <QtCore/QObject>

#include <kstatusbar.h>
#include <kmainwindow.h>
#include <kdebug.h>
#include <kparts/part.h>
#include <kparts/event.h>

using namespace KParts;

///////////////////////////////////////////////////////////////////
// Helper Classes
///////////////////////////////////////////////////////////////////

class KParts::StatusBarItem {
  public:
    StatusBarItem() // for QValueList
      : m_widget(0), m_visible(false)
      {}
    StatusBarItem( QWidget * widget, int stretch, bool permanent )
      : m_widget(widget), m_stretch(stretch), m_permanent(permanent), m_visible(false)
      {}

    QWidget * widget() const { return m_widget; }

    void ensureItemShown( KStatusBar * sb )
    {
      if ( m_widget && !m_visible )
      {
        if ( m_permanent )
            sb->addPermanentWidget( m_widget, m_stretch );
        else
            sb->addWidget( m_widget, m_stretch );
        m_visible = true;
        m_widget->show();
      }
    }
    void ensureItemHidden( KStatusBar * sb )
    {
      if ( m_widget && m_visible )
      {
        sb->removeWidget( m_widget );
        m_visible = false;
        m_widget->hide();
      }
    }
  private:
    QPointer<QWidget> m_widget;
    int m_stretch;
    bool m_permanent;
    bool m_visible;  // true when the item has been added to the statusbar
};

class KParts::StatusBarExtensionPrivate
{
public:
  StatusBarExtensionPrivate(StatusBarExtension *q): q(q),
                                                    m_statusBar(0) {}

  StatusBarExtension *q;
  QList<StatusBarItem> m_statusBarItems; // Our statusbar items
  KStatusBar* m_statusBar;
};

///////////////////////////////////////////////////////////////////


StatusBarExtension::StatusBarExtension(KParts::ReadOnlyPart *parent)
  : QObject(parent), d(new StatusBarExtensionPrivate(this))
{
  parent->installEventFilter(this);
}

StatusBarExtension::~StatusBarExtension()
{
  delete d;
}


StatusBarExtension *StatusBarExtension::childObject( QObject *obj )
{
    if ( !obj )
        return 0;

    // we try to do it on our own, in hope that we are faster than
    // queryList, which looks kind of big :-)
    const QObjectList &children = obj->children();
    QObjectList::ConstIterator it = children.begin();
    for (; it != children.end(); ++it ) {
        KParts::StatusBarExtension* ext = ::qobject_cast<KParts::StatusBarExtension *>( *it );
        if ( ext )
            return ext;
    }

    return 0;
}

bool StatusBarExtension::eventFilter(QObject * watched, QEvent* ev)
{
  if ( !GUIActivateEvent::test( ev ) ||
       !::qobject_cast<KParts::ReadOnlyPart *>(watched)  )
      return QObject::eventFilter(watched, ev);

  KStatusBar * sb = statusBar();
  if ( !sb )
      return QObject::eventFilter(watched, ev);

  GUIActivateEvent *gae = static_cast<GUIActivateEvent*>(ev);

  if ( gae->activated() )
  {
    QList<StatusBarItem>::iterator it = d->m_statusBarItems.begin();
    for ( ; it != d->m_statusBarItems.end() ; ++it )
      (*it).ensureItemShown( sb );
  }
  else
  {
    QList<StatusBarItem>::iterator it = d->m_statusBarItems.begin();
    for ( ; it != d->m_statusBarItems.end() ; ++it )
      (*it).ensureItemHidden( sb );
  }

  return false;

}

KStatusBar * StatusBarExtension::statusBar() const
{
  if ( !d->m_statusBar )  {
    QWidget* w = static_cast<KParts::ReadOnlyPart*>(parent())->widget();
    KMainWindow* mw = w ? dynamic_cast<KMainWindow *>( w->topLevelWidget() ) : 0;
    if ( mw )
      d->m_statusBar = mw->statusBar();
  }
  return d->m_statusBar;
}

void StatusBarExtension::setStatusBar( KStatusBar* status )
{
  d->m_statusBar = status;
}

void StatusBarExtension::addStatusBarItem( QWidget * widget, int stretch, bool permanent )
{
  d->m_statusBarItems.append( StatusBarItem( widget, stretch, permanent ) );
  StatusBarItem& it = d->m_statusBarItems.last();
  KStatusBar * sb = statusBar();
  if (sb)
    it.ensureItemShown( sb );
}

void StatusBarExtension::removeStatusBarItem( QWidget * widget )
{
  KStatusBar * sb = statusBar();
  QList<StatusBarItem>::iterator it = d->m_statusBarItems.begin();
  for ( ; it != d->m_statusBarItems.end() ; ++it )
    if ( (*it).widget() == widget )
    {
      if ( sb )
        (*it).ensureItemHidden( sb );
      d->m_statusBarItems.erase( it );
      break;
    }
  if ( it == d->m_statusBarItems.end() )
    kWarning(1000) << "StatusBarExtension::removeStatusBarItem. Widget not found : " << widget;
}

#include "statusbarextension.moc"

// vim: ts=2 sw=2 et
