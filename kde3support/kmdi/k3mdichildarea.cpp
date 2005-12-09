//----------------------------------------------------------------------------
//    filename             : k3mdichildarea.cpp
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

#include "k3mdichildarea.h"
#include "k3mdichildarea.moc"

#include "k3mdidefines.h"

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>

#include <math.h>

#include <QMouseEvent>
#include <QFocusEvent>
#include <QDesktopWidget>


///////////////////////////////////////////////////////////////////////////////
// K3MdiChildArea
///////////////////////////////////////////////////////////////////////////////

//============ K3MdiChildArea ============//

K3MdiChildArea::K3MdiChildArea( QWidget *parent )
		: QFrame( parent, "k3mdi_childarea" )
{
	setFrameStyle( QFrame::Panel | QFrame::Sunken );
	m_captionFont = QFont();
	QFontMetrics fm( m_captionFont );
	m_captionFontLineSpacing = fm.lineSpacing();
	m_captionActiveBackColor = KGlobalSettings::activeTitleColor();
	m_captionActiveForeColor = KGlobalSettings::activeTextColor();
	m_captionInactiveBackColor = KGlobalSettings::inactiveTitleColor();
	m_captionInactiveForeColor = KGlobalSettings::inactiveTextColor();
	m_pZ = new Q3PtrList<K3MdiChildFrm>;
	m_pZ->setAutoDelete( true );
	setFocusPolicy( Qt::ClickFocus );
	m_defaultChildFrmSize = QSize( 400, 300 );
}


K3MdiChildArea::~K3MdiChildArea()
{
	delete m_pZ; //This will destroy all the widgets inside.
}


void K3MdiChildArea::manageChild( K3MdiChildFrm* child, bool show, bool cascade )
{
	kdDebug( 760 ) << k_funcinfo << "Adding child " << child << " to be managed" << endl;
	K3MdiChildFrm* top = topChild();
	
	//remove old references. There can be more than one so we remove them all
	if ( m_pZ->findRef( child ) != -1 )
	{
		//QPtrList::find* moves current() to the found item
		m_pZ->take(); 
		while ( m_pZ->findNextRef( child ) != -1 )
			m_pZ->take();
	}

	if ( show )
		m_pZ->append( child ); //visible -> first in the Z order
	else
		m_pZ->insert( 0, child ); //hidden -> last in the Z order

	if ( cascade )
		child->move( getCascadePoint( m_pZ->count() - 1 ) );
	
	if ( show )
	{
		if ( top && top->state() == K3MdiChildFrm::Maximized )
		{
			kdDebug( 760 ) << k_funcinfo << "Maximizing the new child" << endl;
			emit sysButtonConnectionsMustChange( top, child );
			top->setState( K3MdiChildFrm::Normal, false /*animate*/ );
			child->setState( K3MdiChildFrm::Maximized, false /*animate*/ );
		}
		child->show();
		focusTopChild();
	}
}


void K3MdiChildArea::destroyChild( K3MdiChildFrm *child, bool focusTop )
{
	kdDebug( 760 ) << k_funcinfo << "Removing child " << child->caption() << endl;
	bool wasMaximized = ( child->state() == K3MdiChildFrm::Maximized );

	// destroy the old one
	QObject::disconnect( child );
	child->blockSignals( true );
	m_pZ->setAutoDelete( false );
	m_pZ->removeRef( child );

	// focus the next new childframe
	K3MdiChildFrm* newTopChild = topChild();
	if ( wasMaximized )
	{
		if ( newTopChild )
		{
			newTopChild->setState( K3MdiChildFrm::Maximized, false );
			emit sysButtonConnectionsMustChange( child, newTopChild );
		}
		else
			emit noMaximizedChildFrmLeft( child ); // last childframe removed
	}
	
	delete child;
	m_pZ->setAutoDelete( true );

	if ( focusTop )
		focusTopChild();
}


void K3MdiChildArea::destroyChildButNotItsView( K3MdiChildFrm* child, bool focusTop )
{
	kdDebug( 760 ) << k_funcinfo << "Removing child " << child->caption() << endl;
	bool wasMaximized = ( child->state() == K3MdiChildFrm::Maximized );

	// destroy the old one
	QObject::disconnect( child );
	child->unsetClient();
	m_pZ->setAutoDelete( false );
	m_pZ->removeRef( child );

	// focus the next new childframe
	K3MdiChildFrm* newTopChild = topChild();
	if ( wasMaximized )
	{
		if ( newTopChild )
		{
			newTopChild->setState( K3MdiChildFrm::Maximized, false );
			emit sysButtonConnectionsMustChange( child, newTopChild );
		}
		else
			emit noMaximizedChildFrmLeft( child ); // last childframe removed
	}
	delete child;
	m_pZ->setAutoDelete( true );

	if ( focusTop )
		focusTopChild();
}

void K3MdiChildArea::setTopChild( K3MdiChildFrm* child, bool /* bSetFocus */ )
{
	if ( !child )
		return;
	
	if ( topChild() != child )
	{
		kdDebug( 760 ) << k_funcinfo << "Setting " << child->caption() << " as the new top child" << endl;
		m_pZ->setAutoDelete( false );
		if ( child )
			m_pZ->removeRef( child );
		m_pZ->setAutoDelete( true );
		
		//disable the labels of all the other children
		Q3PtrListIterator<K3MdiChildFrm> it( *m_pZ );
		for ( ; ( *it ); ++it )
			( *it )->m_pCaption->setActive( false );
		
		K3MdiChildFrm* maximizedChild = topChild();
		bool topChildMaximized = false;
		if ( maximizedChild && maximizedChild->state() == K3MdiChildFrm::Maximized )
			topChildMaximized = true;
		
		m_pZ->append( child );
		
		int nChildAreaMinW = 0, nChildAreaMinH = 0;
		int nChildAreaMaxW = QWIDGETSIZE_MAX, nChildAreaMaxH = QWIDGETSIZE_MAX;
		if ( topChildMaximized && child->m_pClient )
		{
			//the former top child is maximized, so maximize the new one
			nChildAreaMinW = child->m_pClient->minimumWidth();
			nChildAreaMinH = child->m_pClient->minimumHeight();
			/// @todo: setting the maximum size doesn't work properly - fix this later
			// nChildAreaMaxW = child->m_pClient->maximumWidth();
			// nChildAreaMaxH = child->m_pClient->maximumHeight();
		}
		
		//set the min and max sizes of this child area to the new top child
		setMinimumSize( nChildAreaMinW, nChildAreaMinH );
		setMaximumSize( nChildAreaMaxW, nChildAreaMaxH );
		
		if ( topChildMaximized )
		{   //maximize the new view and restore the old
			child->setState( K3MdiChildFrm::Maximized, false /*animate*/);
			maximizedChild->setState( K3MdiChildFrm::Normal, false /*animate*/ );
			emit sysButtonConnectionsMustChange( maximizedChild, child );
		}
		else
			child->raise();
		
		child->m_pClient->setFocus();
	}
}


void K3MdiChildArea::resizeEvent( QResizeEvent* e )
{
	//If we have a maximized children at the top , adjust its size
	K3MdiChildFrm* child = topChild();
	if ( child && child->state() == K3MdiChildFrm::Maximized  )
	{
		int clientw = 0, clienth = 0;
		if ( child->m_pClient != 0L )
		{
			clientw = child->m_pClient->width();
			clienth = child->m_pClient->height();
		}
		child->resize( width() + K3MDI_CHILDFRM_DOUBLE_BORDER,
		               height() + child->m_pCaption->heightHint() + K3MDI_CHILDFRM_SEPARATOR + K3MDI_CHILDFRM_DOUBLE_BORDER );

	}
	layoutMinimizedChildren();
	QWidget::resizeEvent( e );
}

//=============== mousePressEvent =============//

void K3MdiChildArea::mousePressEvent( QMouseEvent *e )
{
	//Popup the window menu
	if ( e->button() & Qt::RightButton )
		emit popupWindowMenu( mapToGlobal( e->pos() ) );
}

//=============== getCascadePoint ============//

QPoint K3MdiChildArea::getCascadePoint( int indexOfWindow )
{
	if ( indexOfWindow < 0 )
	{
		indexOfWindow = m_pZ->count(); //use the window count
		kdDebug( 760 ) << k_funcinfo << "indexOfWindow was less than zero, using "
			<< indexOfWindow << " as new index" << endl;
	}

	QPoint pnt( 0, 0 );
	if ( indexOfWindow == 0 )
	{
		kdDebug( 760 ) << k_funcinfo << "No windows. Returning QPoint( 0, 0 ) as the cascade point" << endl;
		return pnt;
	}

	bool topLevelMode = false;
	if ( height() == 1 ) 	// hacky?!
		topLevelMode = true;

	kdDebug( 760 ) << k_funcinfo << "Getting the cascade point for window index " << indexOfWindow << endl;
	kdDebug( 760 ) << k_funcinfo << "Do we think we're in top level mode? " << topLevelMode << endl;
	
	K3MdiChildFrm* child = m_pZ->first();
	
	//default values
	int step = 20;
	int h = ( topLevelMode ? QApplication::desktop()->height() : height() );
	int w = ( topLevelMode ? QApplication::desktop()->width() : width() );
	
	int availableHeight = h - m_defaultChildFrmSize.height();
	int availableWidth = w - m_defaultChildFrmSize.width();
	int ax = 0;
	int ay = 0;
	
	if ( child )
	{
		kdDebug( 760 ) << k_funcinfo << "child frame exists. resetting height and width values" << endl;
		step = child->m_pCaption->heightHint() + K3MDI_CHILDFRM_BORDER;
		availableHeight = h - child->minimumHeight();
		availableWidth = w - child->minimumWidth();
	}
	
	for ( int i = 0; i < indexOfWindow; i++ )
	{
		ax += step;
		ay += step;
		
		//do some bounds checking, because to not do it would be bad.
		if ( ax > availableWidth )
			ax = 0;

		if ( ay > availableHeight )
			ay = 0;
	}
	pnt.setX( ax );
	pnt.setY( ay );
	return pnt;
}


void K3MdiChildArea::childMinimized( K3MdiChildFrm *minimizedChild, bool wasMaximized )
{
	//can't find the child in our list, so we don't care.
	if ( m_pZ->findRef( minimizedChild ) == -1 )
	{
		kdDebug( 760 ) << k_funcinfo << "child was minimized but wasn't in our list!" << endl;
		return;
	}
	
	kdDebug( 760 ) << k_funcinfo << endl;
	if ( m_pZ->count() > 1 )
	{
		//move the minimized child to the bottom
		m_pZ->setAutoDelete( false );
		m_pZ->removeRef( minimizedChild );
		m_pZ->setAutoDelete( true );
		m_pZ->insert( 0, minimizedChild );
		
		if ( wasMaximized )
		{ // Need to maximize the new top child
			kdDebug( 760 ) << k_funcinfo << "child just minimized from maximized state. maximize new top child" << endl;
			minimizedChild = topChild();
			if ( !minimizedChild )
				return; //??
			
			if ( minimizedChild->state() == K3MdiChildFrm::Maximized )
				return; //it's already maximized
			
			minimizedChild->setState( K3MdiChildFrm::Maximized, false ); //do not animate the change
		}
		focusTopChild();
	}
	else
		setFocus(); //Remove focus from the child. We only have one window
}

void K3MdiChildArea::focusTopChild()
{
	K3MdiChildFrm* lastChild = topChild();
	if ( !lastChild )
	{
		kdDebug( 760 ) << k_funcinfo << "No more child windows left" << endl;
		emit lastChildFrmClosed();
		return;
	}

	if ( !lastChild->m_pClient->hasFocus() )
	{
		//disable the labels of all the other children
		Q3PtrListIterator<K3MdiChildFrm> it ( *m_pZ );
		for ( ; ( *it ); ++it )
		{
			if ( ( *it ) != lastChild )
				( *it )->m_pCaption->setActive( false );
		}

		kdDebug( 760 ) << k_funcinfo << "Giving focus to " << lastChild->caption() << endl;
		lastChild->raise();
		lastChild->m_pClient->activate();
	}

}

void K3MdiChildArea::cascadeWindows()
{
	kdDebug( 760 ) << k_funcinfo << "cascading windows but not changing their size" << endl;
	int idx = 0;
	Q3PtrList<K3MdiChildFrm> list( *m_pZ );
	list.setAutoDelete( false );
	while ( !list.isEmpty() )
	{
		K3MdiChildFrm* childFrm = list.first();
		if ( childFrm->state() != K3MdiChildFrm::Minimized )
		{
			if ( childFrm->state() == K3MdiChildFrm::Maximized )
				childFrm->restorePressed();
			
			childFrm->move( getCascadePoint( idx ) );
			idx++;
		}
		list.removeFirst();
	}
	focusTopChild();
}

void K3MdiChildArea::cascadeMaximized()
{
	kdDebug( 760 ) << k_funcinfo << "cascading windows. will make sure they are minimum sized" << endl;
	int idx = 0;
	Q3PtrList<K3MdiChildFrm> list( *m_pZ );

	list.setAutoDelete( false );
	while ( !list.isEmpty() )
	{
		K3MdiChildFrm* childFrm = list.first();
		if (childFrm->state() != K3MdiChildFrm::Minimized )
		{
			if (childFrm->state() == K3MdiChildFrm::Maximized )
				childFrm->restorePressed();
			
			QPoint pnt( getCascadePoint( idx ) );
			childFrm->move( pnt );
			QSize curSize( width() - pnt.x(), height() - pnt.y() );
			
			if ( ( childFrm->minimumSize().width() > curSize.width() ) ||
			     ( childFrm->minimumSize().height() > curSize.height() ) )
			{
				childFrm->resize( childFrm->minimumSize() );
			}
			else
				childFrm->resize( curSize );
			
			idx++;
		}
		list.removeFirst();
	}
	focusTopChild();
}

void K3MdiChildArea::expandVertical()
{
	kdDebug( 760 ) << k_funcinfo << "expanding all child frames vertically" << endl;
	int idx = 0;
	Q3PtrList<K3MdiChildFrm> list( *m_pZ );
	list.setAutoDelete( false );
	while ( !list.isEmpty() )
	{
		K3MdiChildFrm* childFrm = list.first();
		if ( childFrm->state() != K3MdiChildFrm::Minimized )
		{
			if ( childFrm->state() == K3MdiChildFrm::Maximized )
				childFrm->restorePressed();
			
			childFrm->setGeometry( childFrm->x(), 0, childFrm->width(), height() );
			idx++;
		}
		list.removeFirst();
	}
	focusTopChild();
}

void K3MdiChildArea::expandHorizontal()
{
	kdDebug( 760 ) << k_funcinfo << "expanding all child frames horizontally" << endl;
	int idx = 0;
	Q3PtrList<K3MdiChildFrm> list( *m_pZ );
	list.setAutoDelete( false );
	while ( !list.isEmpty() )
	{
		K3MdiChildFrm* childFrm = list.first();
		if ( childFrm->state() != K3MdiChildFrm::Minimized )
		{
			if ( childFrm->state() == K3MdiChildFrm::Maximized )
				childFrm->restorePressed();
			
			childFrm->setGeometry( 0, childFrm->y(), width(), childFrm->height() );
			idx++;
		}
		list.removeFirst();
	}
	focusTopChild();
}

int K3MdiChildArea::getVisibleChildCount() const
{
	int visibleChildCount = 0;
	Q3PtrListIterator<K3MdiChildFrm> it( *m_pZ );
	for ( ; ( *it ); ++it )
	{
		if ( ( *it )->state() != K3MdiChildFrm::Minimized && ( *it )->isVisible() )
			visibleChildCount++;
	}
	return visibleChildCount;
}

void K3MdiChildArea::tilePragma()
{
	kdDebug( 760 ) << k_funcinfo << endl;
	tileAllInternal( 9 );
}

void K3MdiChildArea::tileAllInternal( int maxWnds )
{
	kdDebug( 760 ) << k_funcinfo << endl;
	//NUM WINDOWS =           1,2,3,4,5,6,7,8,9
	static int colstable[ 9 ] = { 1, 1, 1, 2, 2, 2, 3, 3, 3 }; //num columns
	static int rowstable[ 9 ] = { 1, 2, 3, 2, 3, 3, 3, 3, 3 }; //num rows
	static int lastwindw[ 9 ] = { 1, 1, 1, 1, 2, 1, 3, 2, 1 }; //last window multiplier
	static int colrecall[ 9 ] = { 0, 0, 0, 3, 3, 3, 6, 6, 6 }; //adjust self
	static int rowrecall[ 9 ] = { 0, 0, 0, 0, 4, 4, 4, 4, 4 }; //adjust self

	int numVisible = getVisibleChildCount();
	if ( numVisible < 1 )
	{
		kdDebug( 760 ) << k_funcinfo << "No visible child windows to tile" << endl;
		return;
	}

	K3MdiChildFrm *tcw = topChild();
	int numToHandle = ( ( numVisible > maxWnds ) ? maxWnds : numVisible );
	
	int xQuantum = width() / colstable[ numToHandle - 1 ];
	int widthToCompare;
	
	if ( tcw->minimumWidth() > m_defaultChildFrmSize.width() )
		widthToCompare = tcw->minimumWidth();
	else
		widthToCompare = m_defaultChildFrmSize.width();
	
	if ( xQuantum < widthToCompare )
	{
		if ( colrecall[ numToHandle - 1 ] != 0 )
		{
			tileAllInternal( colrecall[ numToHandle - 1 ] );
			return ;
		}
	}
	
	int yQuantum = height() / rowstable[ numToHandle - 1 ];
	int heightToCompare;
	if ( tcw->minimumHeight() > m_defaultChildFrmSize.height() )
		heightToCompare = tcw->minimumHeight();
	else
		heightToCompare = m_defaultChildFrmSize.height();
		
	if ( yQuantum < heightToCompare )
	{
		if ( rowrecall[ numToHandle - 1 ] != 0 )
		{
			tileAllInternal( rowrecall[ numToHandle - 1 ] );
			return ;
		}
	}
	int curX = 0;
	int curY = 0;
	int curRow = 1;
	int curCol = 1;
	int curWin = 1;
	
	Q3PtrListIterator<K3MdiChildFrm> it( *m_pZ );
	for ( ; ( *it ); ++it )
	{
		K3MdiChildFrm* child = ( *it );
		if ( child->state() != K3MdiChildFrm::Minimized )
		{
			//restore the window
			if ( child->state() == K3MdiChildFrm::Maximized )
				child->restorePressed();
			
			if ( ( curWin % numToHandle ) == 0 )
				child->setGeometry( curX, curY, xQuantum * lastwindw[ numToHandle - 1 ], yQuantum );
			else
				child->setGeometry( curX, curY, xQuantum, yQuantum );
			
			//example : 12 windows : 3 cols 3 rows
			if ( curCol < colstable[ numToHandle - 1 ] )
			{ //curCol<3
				curX += xQuantum; //add a column in the same row
				curCol++;         //increase current column
			}
			else
			{
				curX = 0;         //new row
				curCol = 1;       //column 1
				if ( curRow < rowstable[ numToHandle - 1 ] )
				{ //curRow<3
					curY += yQuantum; //add a row
					curRow++;         //increase current row
				}
				else
				{
					curY = 0;         //restart from beginning
					curRow = 1;       //reset current row
				}
			}
			curWin++;
		}
	}
	
	if ( tcw )
		tcw->m_pClient->activate();
}

void K3MdiChildArea::tileAnodine()
{
	K3MdiChildFrm * topChildWindow = topChild();
	int numVisible = getVisibleChildCount(); // count visible windows
	if ( numVisible < 1 )
		return ;
	
	int numCols = int( sqrt( ( double ) numVisible ) ); // set columns to square root of visible count
	// create an array to form grid layout
	int *numRows = new int[ numCols ];
	int numCurCol = 0;
	
	while ( numCurCol < numCols )
	{
		numRows[numCurCol] = numCols; // create primary grid values
		numCurCol++;
	}
	
	int numDiff = numVisible - ( numCols * numCols ); // count extra rows
	int numCurDiffCol = numCols; // set column limiting for grid updates
	
	while ( numDiff > 0 )
	{
		numCurDiffCol--;
		numRows[numCurDiffCol]++; // add extra rows to column grid
		
		if ( numCurDiffCol < 1 )
			numCurDiffCol = numCols; // rotate through the grid
		
		numDiff--;
	}
	
	numCurCol = 0;
	int numCurRow = 0;
	int curX = 0;
	int curY = 0;
	
	// the following code will size everything based on my grid above
	// there is no limit to the number of windows it will handle
	// it's great when a kick-ass theory works!!!                      // Pragma :)
	int xQuantum = width() / numCols;
	int yQuantum = height() / numRows[numCurCol];
	Q3PtrListIterator<K3MdiChildFrm> it( *m_pZ );
	for ( ; ( *it ); ++it )
	{
		K3MdiChildFrm* child = ( *it );
		if ( child->state() != K3MdiChildFrm::Minimized )
		{
			if ( child->state() == K3MdiChildFrm::Maximized )
				child->restorePressed();
			
			child->setGeometry( curX, curY, xQuantum, yQuantum );
			numCurRow++;
			curY += yQuantum;
			
			if ( numCurRow == numRows[numCurCol] )
			{
				numCurRow = 0;
				numCurCol++;
				curY = 0;
				curX += xQuantum;
				if ( numCurCol != numCols )
					yQuantum = height() / numRows[ numCurCol ];
			}
		}
	}
	
	delete[] numRows;
	
	if ( topChildWindow )
		topChildWindow->m_pClient->activate();
}


void K3MdiChildArea::tileVertically()
{
	K3MdiChildFrm * topChildWindow = topChild();
	int numVisible = getVisibleChildCount(); // count visible windows
	if ( numVisible < 1 )
		return ;

	int w = width() / numVisible;
	int lastWidth = 0;
	
	if ( numVisible > 1 )
		lastWidth = width() - ( w * ( numVisible - 1 ) );
	else
		lastWidth = w;
	
	int h = height();
	int posX = 0;
	int countVisible = 0;

	Q3PtrListIterator<K3MdiChildFrm> it( *m_pZ );
	for ( ; ( *it ); ++it )
	{
		K3MdiChildFrm* child = ( *it );
		if ( child->state() != K3MdiChildFrm::Minimized )
		{
			if ( child->state() == K3MdiChildFrm::Maximized )
				child->restorePressed();
			
			countVisible++;
			
			if ( countVisible < numVisible )
			{
				child->setGeometry( posX, 0, w, h );
				posX += w;
			}
			else
			{ // last visible childframe
				child->setGeometry( posX, 0, lastWidth, h );
			}
		}
	}
	
	if ( topChildWindow )
		topChildWindow->m_pClient->activate();
}


void K3MdiChildArea::layoutMinimizedChildren()
{
	int posX = 0;
	int posY = height();
	Q3PtrListIterator<K3MdiChildFrm> it( *m_pZ );
	for ( ; ( *it ); ++it )
	{
		K3MdiChildFrm* child = *( it );
		if ( child->state() == K3MdiChildFrm::Minimized )
		{
			
			if ( ( posX > 0 ) && ( posX + child->width() > width() ) )
			{
				posX = 0;
				posY -= child->height();
			}
			
			child->move( posX, posY - child->height() );
			posX = child->geometry().right();
		}
	}
}


void K3MdiChildArea::setMdiCaptionFont( const QFont& fnt )
{
	m_captionFont = fnt;
	QFontMetrics fm( m_captionFont );
	m_captionFontLineSpacing = fm.lineSpacing();

	Q3PtrListIterator<K3MdiChildFrm> it( *m_pZ );
	for ( ; ( *it ); ++it )
		( *it )->doResize();

}

void K3MdiChildArea::setMdiCaptionActiveForeColor( const QColor& clr )
{
	m_captionActiveForeColor = clr;
}

void K3MdiChildArea::setMdiCaptionActiveBackColor( const QColor& clr )
{
	m_captionActiveBackColor = clr;
}

void K3MdiChildArea::setMdiCaptionInactiveForeColor( const QColor& clr )
{
	m_captionInactiveForeColor = clr;
}

void K3MdiChildArea::setMdiCaptionInactiveBackColor( const QColor& clr )
{
	m_captionInactiveBackColor = clr;
}

//KDE4: remove
void K3MdiChildArea::getCaptionColors( const QPalette& /*pal*/, QColor& activeBG,
                                      QColor& activeFG, QColor& inactiveBG, QColor& inactiveFG )
{
	activeBG = KGlobalSettings::activeTitleColor();
	activeFG = KGlobalSettings::activeTextColor();
	inactiveBG = KGlobalSettings::inactiveTitleColor();
	inactiveFG = KGlobalSettings::inactiveTextColor();
}

// kate: space-indent off; replace-tabs off; tab-width 4; indent-mode csands;
