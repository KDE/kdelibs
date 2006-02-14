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

#include <qobject.h>

#include <kstatusbar.h>
#include <kmainwindow.h>
#include <kdebug.h>
#include <kdelibs_export.h>
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
      if ( !m_visible )
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
      if ( m_visible )
      {
        sb->removeWidget( m_widget );
        m_visible = false;
        m_widget->hide();
      }
    }
  private:
    QWidget * m_widget;
    int m_stretch;
    bool m_permanent;
    bool m_visible;  // true when the item has been added to the statusbar
};

///////////////////////////////////////////////////////////////////


StatusBarExtension::StatusBarExtension(KParts::ReadOnlyPart *parent)
  : QObject(parent), m_statusBar(0), d(0)
{
  parent->installEventFilter(this);
}

StatusBarExtension::~StatusBarExtension()
{
}


StatusBarExtension *StatusBarExtension::childObject( QObject *obj )
{
    if ( !obj || obj->children().isEmpty() )
        return 0L;

    // we try to do it on our own, in hope that we are faster than
    // queryList, which looks kind of big :-)
    const QObjectList &children = obj->children();
    QObjectList::ConstIterator it = children.begin();
    for (; it != children.end(); ++it )
        if ( (*it)->inherits( "KParts::StatusBarExtension" ) )
            return static_cast<KParts::StatusBarExtension *>( *it );

    return 0L;
}

bool StatusBarExtension::eventFilter(QObject * watched, QEvent* ev)
{
  if ( !GUIActivateEvent::test( ev ) ||
      !watched->inherits("KParts::ReadOnlyPart")  )
      return QObject::eventFilter(watched, ev);

  KStatusBar * sb = statusBar();
  if ( !sb )
      return QObject::eventFilter(watched, ev);

  GUIActivateEvent *gae = static_cast<GUIActivateEvent*>(ev);

  if ( gae->activated() )
  {
    QList<StatusBarItem>::iterator it = m_statusBarItems.begin();
    for ( ; it != m_statusBarItems.end() ; ++it )
      (*it).ensureItemShown( sb );
  }
  else
  {
    QList<StatusBarItem>::iterator it = m_statusBarItems.begin();
    for ( ; it != m_statusBarItems.end() ; ++it )
      (*it).ensureItemHidden( sb );
  }

  return false;

}

KStatusBar * StatusBarExtension::statusBar() const
{
  if ( !m_statusBar )  {
    QWidget* w = static_cast<KParts::ReadOnlyPart*>(parent())->widget();
    KMainWindow* mw = dynamic_cast<KMainWindow *>( w->topLevelWidget() );
    if ( mw )
      m_statusBar = mw->statusBar();
  }
  return m_statusBar;
}

void StatusBarExtension::setStatusBar( KStatusBar* status )
{
  m_statusBar = status;
}

void StatusBarExtension::addStatusBarItem( QWidget * widget, int stretch, bool permanent )
{
  m_statusBarItems.append( StatusBarItem( widget, stretch, permanent ) );
  StatusBarItem& it = m_statusBarItems.last();
  KStatusBar * sb = statusBar();
  Q_ASSERT(sb);
  if (sb)
    it.ensureItemShown( sb );
}

void StatusBarExtension::removeStatusBarItem( QWidget * widget )
{
  KStatusBar * sb = statusBar();
  QList<StatusBarItem>::iterator it = m_statusBarItems.begin();
  for ( ; it != m_statusBarItems.end() ; ++it )
    if ( (*it).widget() == widget )
    {
      if ( sb )
        (*it).ensureItemHidden( sb );
      m_statusBarItems.erase( it );
      break;
    }
  if ( it == m_statusBarItems.end() )
    kWarning(1000) << "StatusBarExtension::removeStatusBarItem. Widget not found : " << widget << endl;
}

#include "statusbarextension.moc"

// vim: ts=2 sw=2 et
