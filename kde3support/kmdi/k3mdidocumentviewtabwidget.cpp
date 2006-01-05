//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#include <ktabbar.h>
#include "k3mdidocumentviewtabwidget.h"

K3MdiDocumentViewTabWidget::K3MdiDocumentViewTabWidget( QWidget* parent, const char* name ) : KTabWidget( parent )
{
	setObjectName( name );
	m_visibility = K3Mdi::ShowWhenMoreThanOneTab;
	tabBar() ->hide();
	setHoverCloseButton( true );
	connect( this, SIGNAL( closeRequest( QWidget* ) ), this, SLOT( closeTab( QWidget* ) ) );
}

K3MdiDocumentViewTabWidget::~K3MdiDocumentViewTabWidget()
{}

void K3MdiDocumentViewTabWidget::closeTab( QWidget* w )
{
	w->close();
}
void K3MdiDocumentViewTabWidget::addTab ( QWidget * child, const QString & label )
{
	KTabWidget::addTab( child, label );
	showPage( child );
	maybeShow();
}

void K3MdiDocumentViewTabWidget::addTab ( QWidget * child, const QIcon & iconset, const QString & label )
{
	KTabWidget::addTab( child, iconset, label );
	showPage( child );
	maybeShow();
}

#if 0
//FIXME: Qt4 does not have QTab's
void K3MdiDocumentViewTabWidget::addTab ( QWidget * child, QTab * tab )
{
	KTabWidget::addTab( child, tab );
	showPage( child );
	maybeShow();
}
#endif

void K3MdiDocumentViewTabWidget::insertTab ( QWidget * child, const QString & label, int index )
{
	KTabWidget::insertTab( child, label, index );
	showPage( child );
	maybeShow();
	tabBar() ->repaint();
}

void K3MdiDocumentViewTabWidget::insertTab ( QWidget * child, const QIcon & iconset, const QString & label, int index )
{
	KTabWidget::insertTab( child, iconset, label, index );
	showPage( child );
	maybeShow();
	tabBar() ->repaint();
}

#if 0
//FIXME: Qt4 does not have QTab's
void K3MdiDocumentViewTabWidget::insertTab ( QWidget * child, QTab * tab, int index )
{
	KTabWidget::insertTab( child, tab, index );
	showPage( child );
	maybeShow();
	tabBar() ->repaint();
}
#endif

void K3MdiDocumentViewTabWidget::removePage ( QWidget * w )
{
	KTabWidget::removePage( w );
	maybeShow();
}

void K3MdiDocumentViewTabWidget::updateIconInView( QWidget *w, QPixmap icon )
{
	changeTab( w, icon, tabLabel( w ) );
}

void K3MdiDocumentViewTabWidget::updateCaptionInView( QWidget *w, const QString &caption )
{
	changeTab( w, caption );
}

void K3MdiDocumentViewTabWidget::maybeShow()
{
	if ( m_visibility == K3Mdi::AlwaysShowTabs )
	{
		tabBar() ->show();
		if ( cornerWidget() )
		{
			if ( count() == 0 )
				cornerWidget() ->hide();
			else
				cornerWidget() ->show();
		}
	}

	if ( m_visibility == K3Mdi::ShowWhenMoreThanOneTab )
	{
		if ( count() < 2 )
			tabBar() ->hide();
		if ( count() > 1 )
			tabBar() ->show();
		if ( cornerWidget() )
		{
			if ( count() < 2 )
				cornerWidget() ->hide();
			else
				cornerWidget() ->show();
		}
	}

	if ( m_visibility == K3Mdi::NeverShowTabs )
	{
		tabBar() ->hide();
	}
}

void K3MdiDocumentViewTabWidget::setTabWidgetVisibility( K3Mdi::TabWidgetVisibility visibility )
{
	m_visibility = visibility;
	maybeShow();
}

void K3MdiDocumentViewTabWidget::moveTab( int from, int to )
{
  emit initiateTabMove( from, to );
  KTabWidget::moveTab( from, to );
}

K3Mdi::TabWidgetVisibility K3MdiDocumentViewTabWidget::tabWidgetVisibility( )
{
	return m_visibility;
}


#ifndef NO_INCLUDE_MOCFILES
#include "k3mdidocumentviewtabwidget.moc"
#endif

// kate: space-indent off; tab-width 4; replace-tabs off; indent-mode csands;

