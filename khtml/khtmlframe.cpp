/* 
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

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
*/
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- Frames
// $Id$

#include "khtmlframe.h"

#include <qstringlist.h>

#include "khtml.h"
#include "khtmlview.h"
#include "khtmltoken.h"

#include <stdlib.h>

HTMLFrameSet::HTMLFrameSet( QWidget *_parent, 
			    const QString &_cols, const QString &_rows,
			    int _frameBorder, bool _bAllowResize)
    : QWidget( _parent ), size(0), rows(_rows), cols(_cols)
{
    lastPanner = 0L;
    
    frameBorder = _frameBorder;
    bAllowResize = _bAllowResize;
    
    widgetList.setAutoDelete( TRUE );
    
    cFrames = 0;
    valid = false;

    printf("FRAMESET: Cols = \"%s\" Rows = \"%s\"\n", 
	_cols.ascii() ? _cols.ascii() : "", 
	_rows.ascii() ? _rows.ascii() : "");
    
    // Calculate amount of frames
    int nrCols = 0;
    int nrRows = 0;
    QStringList *colSizes=0;    
    QStringList *rowSizes=0;
    elementSizes = 0;
    elements = 0;

    if ( !cols.isEmpty() )
    {
        colSizes = parseSizeStrings( _cols );
        nrCols = colSizes->count();
    }
    
    if ( (nrCols < 2) && (!rows.isEmpty()) )
    {
        rowSizes = parseSizeStrings( _rows );
        nrRows = rowSizes->count();
    }

    printf("FRAMESET: #Cols = %d #Rows = %d\n", nrCols, nrRows); 

    if (nrCols > nrRows)
    {
	orientation = Vertical;
        elementSizes = colSizes;
        if (rowSizes)
            delete rowSizes;
    }
    else
    {
	orientation = Horizontal;
        elementSizes = rowSizes;
        if (colSizes)
            delete colSizes;
    }
   
    if (elementSizes && elementSizes->count())
    {
        elements = elementSizes->count();
        size.resize( elements );   
        valid = true;
    }
}

HTMLFrameSet::~HTMLFrameSet()
{
    widgetList.clear();
    if (elementSizes)
        delete elementSizes;
}

QStringList *
HTMLFrameSet::parseSizeStrings( const QString &sizes)
{
   QString remaining = sizes;
   QStringList *sizeList = new QStringList();

   for(;;)
   {
      int pos = remaining.find(',');
      if (pos == -1)
      {
          sizeList->append( remaining );
          break;
      }
      sizeList->append( remaining.left(pos) );
      remaining = remaining.mid(pos+1);
   }
   return sizeList;
}


void HTMLFrameSet::append( QWidget *_w )
{
    if (cFrames == elements)
    {
        // More frames then expected!
        return;
    }

    // Add the 2. child of the last panner if there is one.
    if ( lastPanner != 0L )
    {
	lastPanner->setChild2( _w );
	lastPanner = 0L;
    }
    
    // Increase the number of processed frames
    cFrames++;  

    widgetList.append( _w );   
    
    int frameborder = -1;
    bool allowresize = TRUE;
    
    if ( _w->inherits( "KHTMLView" ) )
    {
	KHTMLView *v = (KHTMLView*)_w;
	allowresize = v->allowResize();
	frameborder = v->getFrameBorder();
    }
    else if ( _w->inherits( "HTMLFrameSet" ) )
    {      
	HTMLFrameSet *f = (HTMLFrameSet*)_w;
	allowresize = f->getAllowResize();
	frameborder = f->getFrameBorder();
    }
    else
	return;
  
    if ( cFrames < elements && frameborder != 0 )
    {
	HTMLFramePanner *panner = new HTMLFramePanner( orientation, this );
	if ( !allowresize )
	{
	    panner->setIsMoveable( FALSE );
	}
	
	panner->setChild1( _w );
	widgetList.append( panner );
	panner->show();
	lastPanner = panner;
    }
    else
    {
	show();
	resizeEvent( 0 );
    }
    
    _w->show();
}

void HTMLFrameSet::parse()
{
  resizeEvent( 0L );
  
  /*  QWidget *w = 0L;
  for ( w = widgetList.first(); w != 0L; w = widgetList.next() )
  {
    w->show();
  } */
}

void HTMLFrameSet::resizeEvent( QResizeEvent* )
{
    if ( !isVisible() )
	return;      

    if ( orientation == Vertical )
    {
      elements = calcSize( width() );
    }
    else
    {
      elements = calcSize( height() );
    }
     

    int j = 0;
    int i = 0;
    int pannerSize = frameBorder > 0 ? 5 : 0;
    QWidget *w;
    QWidget* next;
    for ( w = widgetList.first(); w != 0L; w = next )
    {
	next = widgetList.next();
	
	if ( orientation == Horizontal )
	{
	    if ( w->isA( "HTMLFramePanner" ) )
	    {
		w->setGeometry( 0, j, width(), pannerSize );
		j += pannerSize;
	    }
	    else
	    {
		if ( next && next->isA( "HTMLFramePanner" ) )
		    size[i] -= pannerSize;
		w->setGeometry( 0, j, width(), size[ i ] );
		j += size[ i ];
		i++;
	    }
	}
	else
	{
	    if ( w->isA( "HTMLFramePanner" ) )
	    {
		w->setGeometry( j, 0, pannerSize, height() );
		j += pannerSize;
	    }
	    else
	    {
		if ( next && next->isA( "HTMLFramePanner" ) )
		    size[i] -= pannerSize;
		w->setGeometry( j, 0, size[ i ], height() );
		j += size[ i ];
		i++;	
	    }
	}
    }
    debug("Done Set");
}

int HTMLFrameSet::calcSize( int _max )
{	
    if (!elements)
        return 0;

    QArray<int> value(elements);
    QArray<int> mode(elements);

    int i = 0;
    for( QStringList::ConstIterator it = elementSizes->begin();
         (it != elementSizes->end());
         it++, i++)
    {
        QString token = *it;
        int percent;
	if ( token.contains('%') )
	{
            int pos = token.find('%');
            percent = token.left(pos).toInt();
	    mode[i] = 1;
	    value[i] = ( percent * _max ) / 100;
	}
	else if ( token.contains('*') )
	{
            int pos = token.find('*');
            value[i] = token.left(pos).toInt();
	    if ( value[i] == 0 )
		value[i] = 1;
	    mode[i] = 2;
	}
	else 
        {
            value[i] = token.toInt();
	    mode[i] = 0;
        }
    }

    bool joker = false;
    
    int s1 = 0;
    for ( int j = 0; j < elements; j++ )
    {
	if ( mode[j] == 0 || mode[j] == 1 )
	    s1 += value[j];
	else if ( mode[j] == 2 )
	    joker = true;
    }
    
    int k;
    if ( s1 <= _max && !joker )
    {
        if (!s1)
            s1 = 1;
	for ( k = 0; k < elements; k++ )
	{
	    size[k] = ( value[k] * _max ) / s1;
	}

	// Calculate the error
	int s2 = 0;
	for ( k = 0; k < elements; k++ )
	{
	    s2 += size[k];
	}
	// Add the error to the last frame
	size[ elements - 1 ] += _max - s2;
    }
    else if ( s1 <= _max && joker )
    {
	int s2 = 0;
	int s3 = 0;
	for ( k = 0; k < elements; k++ )
	{
	    if ( mode[k] == 0 || mode[k] == 1 )
	    {
		size[k] = value[k];
		s2 += value[k];
	    }
	    else
		s3 += value[k];
	}
	for ( k = 0; k < elements; k++ )
	{
	    if ( mode[k] == 2 )
	    {
	        if (s3)
	        {
                    size[k] = ( _max - s2 ) * value[k] / s3;
                    s2 += size[k];
		    s3 -= value[k];
                }
                else
                {
                    debug("%s:%d s3==0 assertion failed!", __FILE__, __LINE__);
                    size[k] = 40;
                }
	    }
	}
    }
    else
    {
	for ( k = 0; k < elements; k++ )
	{
	    if ( mode[k] == 0 || mode[k] == 1 )
	    {
		size[k] = value[k] * _max / s1;
	    }
	    else
		size[k] = 50;
	}

	// Calculate the error
	int s2 = 0;
	for ( k = 0; k < elements; k++ )
	{
	    if ( mode[k] == 0 || mode[k] == 1 )
		s2 += size[k];
	}
	// Add the error to the last frame
	size[ elements - 1 ] += _max - s2;	
    }

    return i;
}

KHTMLView* HTMLFrameSet::getSelectedFrame()
{
    QWidget *w;
    for ( w = widgetList.first(); w != 0L; w = widgetList.next() )
    {
      if ( w->inherits( "KHTMLView" ) )
      {
	KHTMLView *v = (KHTMLView*)w;
	KHTMLView *ret = v->getSelectedView();
	if ( ret )
	  return ret;
      }
    }
    
    return 0L;
}

HTMLFramePanner::HTMLFramePanner( HTMLFramePanner::Orientation _orientation, QWidget *_parent, const char *_name ) :
    QFrame( _parent, _name )
{
    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(1);

    child1 = 0L;
    child2 = 0L;
    orientation = _orientation;

    if ( orientation == Horizontal )
      setCursor( sizeVerCursor );
    else
      setCursor( sizeHorCursor );
    
    moveable = TRUE;
}

HTMLFramePanner::~HTMLFramePanner()
{
}

void HTMLFramePanner::setIsMoveable( bool _move )
{
  moveable = _move;
  if ( !moveable )
    setCursor( arrowCursor );
  else if ( orientation == Horizontal )
    setCursor( sizeVerCursor );
  else
    setCursor( sizeHorCursor );
}

void HTMLFramePanner::mousePressEvent( QMouseEvent *_ev )
{   
    if ( !moveable )
        return;
    
    if ( child1 == 0L || child2 == 0L )
	return;
    
    raise();
    
    grabMouse();
    
    initialX = x();
    initialY = y();
    initialGlobal = mapToGlobal( _ev->pos() );
}

void HTMLFramePanner::mouseMoveEvent( QMouseEvent *_ev )
{
    if ( !moveable )
        return;
    
    if ( child1 == 0L || child2 == 0L )
	return;

    if ( orientation == Horizontal )
    {
	QPoint p = mapToGlobal( _ev->pos() );
	QPoint p2 = child1->mapToGlobal( QPoint( 0, 0 + 10 ) );
	QPoint p3 = child2->mapToGlobal( QPoint( 0, child2->height() - 10 - 5 ) );
	int pos = p.y();
	if ( pos < p2.y() )
	    pos = p2.y();
	else if ( pos > p3.y() )
	    pos = p3.y();

	move( initialX, initialY + ( pos - initialGlobal.y() ) );
    }
    else
    {
	QPoint p = mapToGlobal( _ev->pos() );
	QPoint p2 = child1->mapToGlobal( QPoint( 0 + 10, 0 ) );
	QPoint p3 = child2->mapToGlobal( QPoint( child2->width() - 10 - 5 , 0 ) );
	int pos = p.x();
	if ( pos < p2.x() )
	    pos = p2.x();
	else if ( pos > p3.x() )
	    pos = p3.x();

	move( initialX + ( pos - initialGlobal.x() ), initialY );
    }
}

void HTMLFramePanner::mouseReleaseEvent( QMouseEvent * )
{
    if ( !moveable )
        return;
    
    if ( child1 == 0L || child2 == 0L )
	return;

    releaseMouse();

    if ( orientation == Horizontal )
    {
	child1->setGeometry( child1->x(), child1->y(), child1->width(), y() - child1->y() );
	
	child2->setGeometry( child2->x(), y() + 5, child2->width(),
			     child2->y() + child2->height() - ( y() + 5 ) );
    }
    else
    {
	child1->setGeometry( child1->x(), child1->y(), x() - child1->x(), child1->height() );
	
	child2->setGeometry( x() + 5, child2->y(), child2->x() + child2->width() - ( x() + 5 ),
			     child2->height() );
    }

    raise();
}

#include "khtmlframe.moc"

