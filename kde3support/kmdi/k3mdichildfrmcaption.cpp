//----------------------------------------------------------------------------
//    filename             : k3mdichildfrmcaption.cpp
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//
//    copyright            : (C) 1999-2003 by Szymon Stefanek (stefanek@tin.it)
//                                         and
//                                         Falk Brettschneider
//    email                :  falkbr@kdevelop.org (Falk Brettschneider)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#include "k3mdichildfrmcaption.h"
#include "k3mdichildfrmcaption.moc"

#include <qpainter.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qtoolbutton.h>
#include <q3popupmenu.h>
#include <QMouseEvent>

#include "k3mdidefines.h"
#include "k3mdichildfrm.h"
#include "k3mdichildarea.h"
#include "k3mdimainfrm.h"
#include <klocale.h>
#include <iostream>

#ifdef Q_WS_WIN 
//TODO: one day gradient can be added for win98/winnt5+
// ask system properties on windows
#ifndef SPI_GETGRADIENTCAPTIONS
# define SPI_GETGRADIENTCAPTIONS 0x1008
#endif
#ifndef COLOR_GRADIENTACTIVECAPTION
# define COLOR_GRADIENTACTIVECAPTION 27
#endif
#ifndef COLOR_GRADIENTINACTIVECAPTION
# define COLOR_GRADIENTINACTIVECAPTION 28
#endif
#endif 
//#endif

//////////////////////////////////////////////////////////////////////////////
// Class   : K3MdiChildFrmCaption
// Purpose : An MDI label that draws the title
//
//
//////////////////////////////////////////////////////////////////////////////

//============== K3MdiChildFrmCaption =============//

K3MdiChildFrmCaption::K3MdiChildFrmCaption( K3MdiChildFrm *parent )
		: QWidget( parent )
{
	setObjectName( QLatin1String( "k3mdi_childfrmcaption" ) );
	m_szCaption = i18n( "Unnamed" );
	m_bActive = false;
	m_pParent = parent;
	setBackgroundMode( Qt::NoBackground );
	setFocusPolicy( Qt::NoFocus );
	m_bChildInDrag = false;
}

//============== ~K3MdiChildFrmCaption =============//

K3MdiChildFrmCaption::~K3MdiChildFrmCaption()
{}

//============= mousePressEvent ==============//

void K3MdiChildFrmCaption::mousePressEvent( QMouseEvent *e )
{
	if ( e->button() == Qt::LeftButton )
	{
		setMouseTracking( false );
		if ( K3MdiMainFrm::frameDecorOfAttachedViews() != K3Mdi::Win95Look )
		{
			QApplication::setOverrideCursor( Qt::SizeAllCursor, true );
		}
		m_pParent->m_bDragging = true;
		m_offset = mapToParent( e->pos() );
	}
	else if ( e->button() == Qt::RightButton )
	{
		m_pParent->systemMenu()->popup( mapToGlobal( e->pos() ) );
	}
}

//============= mouseReleaseEvent ============//

void K3MdiChildFrmCaption::mouseReleaseEvent( QMouseEvent *e )
{
	if ( e->button() == Qt::LeftButton )
	{
		if ( K3MdiMainFrm::frameDecorOfAttachedViews() != K3Mdi::Win95Look )
			QApplication::restoreOverrideCursor();
		
		releaseMouse();
		if ( m_pParent->m_bDragging )
		{
			m_pParent->m_bDragging = false;
			if ( m_bChildInDrag )
			{
				//notify child view
				K3MdiChildFrmDragEndEvent ue( e );
				if ( m_pParent->m_pClient != 0L )
					QApplication::sendEvent( m_pParent->m_pClient, &ue );
				
				m_bChildInDrag = false;
			}
		}
	}
}

//============== mouseMoveEvent =============//
void K3MdiChildFrmCaption::mouseMoveEvent( QMouseEvent *e )
{
	if ( !m_pParent->m_bDragging )
		return ;

	if ( !m_bChildInDrag )
	{
		//notify child view
		K3MdiChildFrmDragBeginEvent ue( e );
		if ( m_pParent->m_pClient != 0L )
			QApplication::sendEvent( m_pParent->m_pClient, &ue );
		
		m_bChildInDrag = true;
	}

	QPoint relMousePosInChildArea = m_pParent->m_pManager->mapFromGlobal( e->globalPos() );

	// mouse out of child area? stop child frame dragging
	if ( !m_pParent->m_pManager->rect().contains( relMousePosInChildArea ) )
	{
		if ( relMousePosInChildArea.x() < 0 )
			relMousePosInChildArea.rx() = 0;
		
		if ( relMousePosInChildArea.y() < 0 )
			relMousePosInChildArea.ry() = 0;
		
		if ( relMousePosInChildArea.x() > m_pParent->m_pManager->width() )
			relMousePosInChildArea.rx() = m_pParent->m_pManager->width();
		
		if ( relMousePosInChildArea.y() > m_pParent->m_pManager->height() )
			relMousePosInChildArea.ry() = m_pParent->m_pManager->height();
	}
	QPoint mousePosInChildArea = relMousePosInChildArea - m_offset;

	// set new child frame position
	parentWidget() ->move( mousePosInChildArea );
}

//=============== setActive ===============//

void K3MdiChildFrmCaption::setActive( bool bActive )
{
	if ( m_bActive == bActive )
		return ;

	//    Ensure the icon's pixmap has the correct bg color
	m_pParent->m_pWinIcon->setBackgroundColor( bActive ?
	                                           m_pParent->m_pManager->m_captionActiveBackColor :
	                                           m_pParent->m_pManager->m_captionInactiveBackColor );
	m_pParent->m_pUnixIcon->setBackgroundColor( bActive ?
	                                            m_pParent->m_pManager->m_captionActiveBackColor :
	                                            m_pParent->m_pManager->m_captionInactiveBackColor );

	m_bActive = bActive;
	repaint( false );
}

//=============== setCaption ===============//

void K3MdiChildFrmCaption::setCaption( const QString& text )
{
	m_szCaption = text;
	repaint( false );
}

//============== heightHint ===============//

int K3MdiChildFrmCaption::heightHint()
{
	int hint = m_pParent->m_pManager->m_captionFontLineSpacing + 3;
	if ( K3MdiMainFrm::frameDecorOfAttachedViews() == K3Mdi::Win95Look )
	{
		if ( hint < 18 )
			hint = 18;
	}
	else if ( K3MdiMainFrm::frameDecorOfAttachedViews() == K3Mdi::KDE1Look )
	{
		if ( hint < 20 )
			hint = 20;
	}
	else if ( K3MdiMainFrm::frameDecorOfAttachedViews() == K3Mdi::KDELook )
	{
		if ( hint < 16 )
			hint = 16;
	}
	else
	{   // kde2laptop look
		hint -= 4;
		if ( hint < 14 )
			hint = 14;
	}
	return hint;
}

//=============== paintEvent ==============//

void K3MdiChildFrmCaption::paintEvent( QPaintEvent * )
{
	QPainter p( this );
	QRect r = rect();
	p.setFont( m_pParent->m_pManager->m_captionFont );
	
	if ( m_bActive )
	{
		p.fillRect( r, m_pParent->m_pManager->m_captionActiveBackColor );
		p.setPen( m_pParent->m_pManager->m_captionActiveForeColor );
	}
	else
	{
		p.fillRect( r, m_pParent->m_pManager->m_captionInactiveBackColor );
		p.setPen( m_pParent->m_pManager->m_captionInactiveForeColor );
	}
	
	//Shift the text after the icon
	if ( K3MdiMainFrm::frameDecorOfAttachedViews() == K3Mdi::Win95Look )
		r.setLeft( r.left() + m_pParent->icon() ->width() + 3 );
	else if ( K3MdiMainFrm::frameDecorOfAttachedViews() == K3Mdi::KDE1Look )
		r.setLeft( r.left() + 22 );
	else if ( K3MdiMainFrm::frameDecorOfAttachedViews() == K3Mdi::KDELook )
		r.setLeft( r.left() + m_pParent->icon() ->width() + 3 );
	else  // kde2laptop look
		r.setLeft( r.left() + 30 );

	int captionWidthForText = width() - 4 * m_pParent->m_pClose->width() - m_pParent->icon() ->width() - 5;
	QString text = abbreviateText( m_szCaption, captionWidthForText );
	p.drawText( r, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine, text );

}


QString K3MdiChildFrmCaption::abbreviateText( QString origStr, int maxWidth )
{
	QFontMetrics fm = fontMetrics();
	int actualWidth = fm.width( origStr );

	int realLetterCount = origStr.length();
	int newLetterCount;
	
	if ( actualWidth != 0 )
		newLetterCount = ( maxWidth * realLetterCount ) / actualWidth;
	else
		newLetterCount = realLetterCount; // should be 0 anyway

	int w = maxWidth + 1;
	QString s = origStr;
	
	if ( newLetterCount <= 0 )
		s = "";
	
	while ( ( w > maxWidth ) && ( newLetterCount >= 1 ) )
	{
		if ( newLetterCount < realLetterCount )
		{
			if ( newLetterCount > 3 )
				s = origStr.left( newLetterCount / 2 ) + "..." + origStr.right( newLetterCount / 2 );
			else
			{
				if ( newLetterCount > 1 )
					s = origStr.left( newLetterCount ) + "..";
				else
					s = origStr.left( 1 );
			}
		}
		QFontMetrics fm = fontMetrics();
		w = fm.width( s );
		newLetterCount--;
	}
	return s;
}

//============= mouseDoubleClickEvent ===========//

void K3MdiChildFrmCaption::mouseDoubleClickEvent( QMouseEvent * )
{
	m_pParent->maximizePressed();
}

//============= slot_moveViaSystemMenu ===========//

void K3MdiChildFrmCaption::slot_moveViaSystemMenu()
{
	setMouseTracking( true );
	grabMouse();
	
	if ( K3MdiMainFrm::frameDecorOfAttachedViews() != K3Mdi::Win95Look )
		QApplication::setOverrideCursor( Qt::SizeAllCursor, true );
	
	m_pParent->m_bDragging = true;
	m_offset = mapFromGlobal( QCursor::pos() );
}

// kate: space-indent off; replace-tabs off; indent-mode csands; tab-width 4;
