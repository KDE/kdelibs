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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

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

#include "tabwidget.h"
#include "tabwidget.moc"

namespace KMDI
{

TabWidget::TabWidget(QWidget* parent, const char* name):KTabWidget(parent,name)
{
	m_visibility = KMDI::ShowWhenMoreThanOneTab;
	tabBar()->hide();
	setHoverCloseButton(true);
        connect(this, SIGNAL(closeRequest(QWidget*)), this, SLOT(closeTab(QWidget*)));
}

TabWidget::~TabWidget() {
}

void TabWidget::closeTab(QWidget* w) {
	w->close();
}
void TabWidget::addTab ( QWidget * child, const QString & label ) {
	KTabWidget::addTab(child,label);
    showPage(child);
	maybeShow();
}

void TabWidget::addTab ( QWidget * child, const QIconSet & iconset, const QString & label ) {
	KTabWidget::addTab(child,iconset,label);
    showPage(child);
	maybeShow();
}

void TabWidget::addTab ( QWidget * child, QTab * tab ) {
	KTabWidget::addTab(child,tab);
    showPage(child);
	maybeShow();
}

void TabWidget::insertTab ( QWidget * child, const QString & label, int index) {
	KTabWidget::insertTab(child,label,index);
    showPage(child);
	maybeShow();
	tabBar()->repaint();
}

void TabWidget::insertTab ( QWidget * child, const QIconSet & iconset, const QString & label, int index ) {
	KTabWidget::insertTab(child,iconset,label,index);
    showPage(child);
	maybeShow();
	tabBar()->repaint();
}

void TabWidget::insertTab ( QWidget * child, QTab * tab, int index) {
	KTabWidget::insertTab(child,tab,index);
    showPage(child);
	maybeShow();
	tabBar()->repaint();
}

void TabWidget::removePage ( QWidget * w ) {
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
	if ( m_visibility == KMDI::AlwaysShowTabs )
	{
		tabBar()->show();
                if (cornerWidget())
                {
                    if (count() == 0)
                        cornerWidget()->hide();
                    else
                        cornerWidget()->show();
                }
	}

	if ( m_visibility == KMDI::ShowWhenMoreThanOneTab )
	{
		if (count()<2) tabBar()->hide();
		if (count()>1) tabBar()->show();
                if (cornerWidget())
                {
                    if (count() < 2)
                        cornerWidget()->hide();
                    else
                        cornerWidget()->show();
                }
	}

	if ( m_visibility == KMDI::NeverShowTabs )
	{
		tabBar()->hide();
	}
}

void TabWidget::setTabWidgetVisibility( KMDI::TabWidgetVisibility visibility )
{
	m_visibility = visibility;
	maybeShow();
}

KMDI::TabWidgetVisibility TabWidget::tabWidgetVisibility( )
{
	return m_visibility;
}

}


