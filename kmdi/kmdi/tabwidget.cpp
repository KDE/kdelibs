//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#include <ktabbar.h>
#include <kpopupmenu.h>

#include "tabwidget.h"
#include "tabwidget.moc"

namespace KMDI
{

TabWidget::TabWidget(QWidget* parent, const char* name):KTabWidget(parent,name)
{
	m_visibility = KMdi::ShowWhenMoreThanOneTab;
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
	if ( m_visibility == KMdi::AlwaysShowTabs )
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

	if ( m_visibility == KMdi::ShowWhenMoreThanOneTab )
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

	if ( m_visibility == KMdi::NeverShowTabs )
	{
		tabBar()->hide();
	}
}

void TabWidget::setTabWidgetVisibility( KMdi::TabWidgetVisibility visibility )
{
	m_visibility = visibility;
	maybeShow();
}

KMdi::TabWidgetVisibility TabWidget::tabWidgetVisibility( )
{
	return m_visibility;
}

}


