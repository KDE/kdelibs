/* This file is part of the KDE libraries
   Copyright (C) 2004 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

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

   Based on:

   //----------------------------------------------------------------------------
   //    Project              : KDE MDI extension
   //
   //    begin                : 07/1999       by Szymon Stefanek as part of kvirc
   //                                         (an IRC application)
   //    changes              : 09/1999       by Falk Brettschneider to create an
   //                           - 06/2000     stand-alone Qt extension set of
   //                                         classes and a Qt-based library
   //                         : 02/2000       by Massimo Morin (mmorin@schedsys.com)
   //                           2000-2003     maintained by the KDevelop project
   //    patches              : -/2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
   //                         : 01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
   //
   //    copyright            : (C) 1999-2003 by Falk Brettschneider
   //                                         and
   //                                         Szymon Stefanek (stefanek@tin.it)
   //    email                :  falkbr@kdevelop.org (Falk Brettschneider)
   //----------------------------------------------------------------------------
*/

#include <ktabbar.h>
#include <kpopupmenu.h>
#include <kdebug.h>

#include <qobjectlist.h>

#include "tabwidget.h"
#include "tabwidget.moc"

namespace KMDIPrivate
{
  class TabWidgetPrivate
  {

  };
}

namespace KMDI
{

TabWidget::TabWidget(QWidget* parent, const char* name)
 : KTabWidget(parent,name)
 , m_visibility (KMDI::ShowWhenMoreThanOneTab)
 , d (new KMDIPrivate::TabWidgetPrivate())
{
  installEventFilter (this);

  tabBar()->hide();

  setHoverCloseButton(true);

  connect(this, SIGNAL(closeRequest(QWidget*)), this, SLOT(closeTab(QWidget*)));
}

TabWidget::~TabWidget()
{
  delete d;
  d = 0;
}

bool TabWidget::eventFilter(QObject *obj, QEvent *e )
{
  if(e->type() == QEvent::FocusIn)
  {
    emit focusInEvent ();
  }
  else if (e->type() == QEvent::ChildRemoved)
  {
    // if we lost a child we uninstall ourself as event filter for the lost
    // child and its children
    QObject* pLostChild = ((QChildEvent*)e)->child();
    if ((pLostChild != 0L) && (pLostChild->isWidgetType())) {
       QObjectList *list = pLostChild->queryList( "QWidget" );
       list->insert(0, pLostChild);        // add the lost child to the list too, just to save code
       QObjectListIt it( *list );          // iterate over all lost child widgets
       QObject * o;
       while ( (o=it.current()) != 0 ) { // for each found object...
          QWidget* widg = (QWidget*)o;
          ++it;
          widg->removeEventFilter(this);
       }
       delete list;                        // delete the list, not the objects
    }
  }
  else if (e->type() == QEvent::ChildInserted)
  {
    // if we got a new child and we are attached to the MDI system we
    // install ourself as event filter for the new child and its children
    // (as we did when we were added to the MDI system).
    QObject* pNewChild = ((QChildEvent*)e)->child();
    if ((pNewChild != 0L) && (pNewChild->isWidgetType()))
    {
       QWidget* pNewWidget = (QWidget*)pNewChild;
       if (pNewWidget->testWFlags(Qt::WType_Dialog | Qt::WShowModal))
           return false;
       QObjectList *list = pNewWidget->queryList( "QWidget" );
       list->insert(0, pNewChild);         // add the new child to the list too, just to save code
       QObjectListIt it( *list );          // iterate over all new child widgets
       QObject * o;
       while ( (o=it.current()) != 0 ) { // for each found object...
          QWidget* widg = (QWidget*)o;
          ++it;
          widg->installEventFilter(this);
          connect(widg, SIGNAL(destroyed()), this, SLOT(childDestroyed()));
       }
       delete list;                        // delete the list, not the objects
    }
  }

  return KTabWidget::eventFilter (obj, e);
}

void TabWidget::childDestroyed()
{
  // if we lost a child we uninstall ourself as event filter for the lost
  // child and its children
  const QObject* pLostChild = QObject::sender();
  if ((pLostChild != 0L) && (pLostChild->isWidgetType()))
  {
     QObjectList *list = ((QObject*)(pLostChild))->queryList("QWidget");
     list->insert(0, pLostChild);        // add the lost child to the list too, just to save code
     QObjectListIt it( *list );          // iterate over all lost child widgets
     QObject * obj;
     while ( (obj=it.current()) != 0 ) { // for each found object...
       QWidget* widg = (QWidget*)obj;
       ++it;
       widg->removeEventFilter(this);
     }
     delete list;                        // delete the list, not the objects
  }
}

void TabWidget::closeTab(QWidget* w)
{
  w->close();
}

void TabWidget::addTab ( QWidget * child, const QString & label )
{
  KTabWidget::addTab(child,label);
  showPage(child);
  maybeShow();
}

void TabWidget::addTab ( QWidget * child, const QIconSet & iconset, const QString & label )
{
  KTabWidget::addTab(child,iconset,label);
  showPage(child);
  maybeShow();
}

void TabWidget::addTab ( QWidget * child, QTab * tab )
{
  KTabWidget::addTab(child,tab);
  showPage(child);
  maybeShow();
}

void TabWidget::insertTab ( QWidget * child, const QString & label, int index)
{
  KTabWidget::insertTab(child,label,index);
  showPage(child);
  maybeShow();
  tabBar()->repaint();
}

void TabWidget::insertTab ( QWidget * child, const QIconSet & iconset, const QString & label, int index )
{
  KTabWidget::insertTab(child,iconset,label,index);
  showPage(child);
  maybeShow();
  tabBar()->repaint();
}

void TabWidget::insertTab ( QWidget * child, QTab * tab, int index)
{
  KTabWidget::insertTab(child,tab,index);
  showPage(child);
  maybeShow();
  tabBar()->repaint();
}

void TabWidget::removePage ( QWidget * w )
{
  KTabWidget::removePage(w);
  maybeShow();
}

void TabWidget::updateIconInView( QWidget *w, QPixmap icon )
{
  changeTab(w,icon,tabLabel(w));
}

void TabWidget::updateCaptionInView( QWidget *w, const QString &caption )
{
  changeTab(w, caption);
}

void TabWidget::maybeShow()
{
  switch (m_visibility)
  {
    case KMDI::AlwaysShowTabs:
      tabBar()->show();

      // show/hide corner widgets
      if (count() == 0)
        setCornerWidgetVisibility(false);
      else
        setCornerWidgetVisibility(true);

      break;

    case KMDI::ShowWhenMoreThanOneTab:
      if (count()<2) tabBar()->hide();
      else tabBar()->show();

      // show/hide corner widgets
      if (count() < 2)
        setCornerWidgetVisibility(false);
      else
        setCornerWidgetVisibility(true);

      break;

    case KMDI::NeverShowTabs:
      tabBar()->hide();
      break;
  }
}

void TabWidget::setCornerWidgetVisibility(bool visible) {
  // there are two corner widgets: on TopLeft and on TopTight!

  if (cornerWidget(Qt::TopLeft) ) {
    if (visible)
      cornerWidget(Qt::TopLeft)->show();
    else
      cornerWidget(Qt::TopLeft)->hide();
  }

  if (cornerWidget(Qt::TopRight) ) {
    if (visible)
      cornerWidget(Qt::TopRight)->show();
    else
      cornerWidget(Qt::TopRight)->hide();
  }
}

void TabWidget::setTabWidgetVisibility( KMDI::TabWidgetVisibility visibility )
{
  m_visibility = visibility;
  maybeShow();
}

KMDI::TabWidgetVisibility TabWidget::tabWidgetVisibility( ) const
{
  return m_visibility;
}

}

// kate: space-indent on; indent-width 2; replace-tabs on;
