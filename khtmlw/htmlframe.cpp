/* This file is part of the KDE libraries
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
#include "htmlframe.h"
#include "html.h"
#include "htmlview.h"

#include <kcursor.h>

#include <strings.h>
#include <stdlib.h>

HTMLFrameSet::HTMLFrameSet( QWidget *_parent, const char *_src )
    : QWidget( _parent )
{
    lastPanner = 0L;
    
    frameBorder = 1;
    bAllowResize = TRUE;
    
    widgetList.setAutoDelete( TRUE );
    
    size = 0L;
    cFrames = 0;
    
    QString s = _src + 9;
    StringTokenizer st;
    st.tokenize( s, " >" );
    while ( st.hasMoreTokens() )
    {
	const char* token = st.nextToken();
	if ( strncasecmp( token, "COLS=", 5 ) == 0 )
	{
	    cols = token + 5;
	}
	else if ( strncasecmp( token, "ROWS=", 5 ) == 0 )
	{
	    rows = token + 5;
	}
	else if ( strncasecmp( token, "FRAMEBORDER=", 12 ) == 0 )
	{
	    frameBorder = atoi( token + 12 );
	}
	else if ( strncasecmp( token, "NORESIZE", 8 ) == 0 )
	{
            bAllowResize = FALSE;
	}
    }

    if ( !cols.isNull() )
	orientation = HTMLFramePanner::VERTICAL;
    else
	orientation = HTMLFramePanner::HORIZONTAL;

    // Calculate amount of frames
    elements = 1;
    char *p = "";
    if ( !cols.isNull() )
	p = cols.data();
    else if ( !rows.isNull() )
	p = rows.data();
    while ( ( p = strchr( p, ',' ) ) != 0 ) { p++; elements++; }
 
    size = new int[ elements ];   
}

HTMLFrameSet::~HTMLFrameSet()
{
    widgetList.clear();
    if ( size )
	delete []size;
}

void HTMLFrameSet::append( QWidget *_w )
{
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

    if ( !cols.isNull() )
    {
	elements = calcSize( cols.data(), size, width() );
    }
    else if ( !rows.isNull() )
    {
	elements = calcSize( rows.data(), size, height() );
    }

    int j = 0;
    int i = 0;
    int pannerSize = frameBorder > 0 ? 5 : 0;
    QWidget *w;
    QWidget* next;
    for ( w = widgetList.first(); w != 0L; w = next )
    {
	next = widgetList.next();
	
	if ( orientation == HTMLFramePanner::HORIZONTAL )
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

int HTMLFrameSet::calcSize( const char *_str, int *size, int _max )
{	
    debug("Calculating size");
  
    int value[1024];
    int mode[1024];
    int i = 0;
    
    QString s = _str;
    StringTokenizer st;
    st.tokenize( s, "," );
    while ( st.hasMoreTokens() )
    {
	if ( i == 1024 )
	    return i;
	
	const char* token = st.nextToken();
	if ( token[0] != 0 )
	{
	    //printf("WIDTH='%s'\n",token);
	    value[i] = atoi( token );
	    if ( strchr( token, '%' ) != 0 )
	    {
		mode[i] = 1;
		value[i] = ( value[i] * _max ) / 100;
	    }
	    else if ( strchr( token, '*' ) != 0 )
	    {
		if ( value[i] == 0 )
		    value[i] = 1;
		mode[i] = 2;
	    }
	    else
		mode[i] = 0;
	    
	    i++;
	}
    }

    debug("*************** CALC SIZE elements = %i ******************",i);
    
    debug("max. width=%i   max. height=%i",width(),height() );

    bool joker = FALSE;
    
    int s1 = 0;
    for ( int j = 0; j < i; j++ )
    {
	if ( mode[j] == 0 || mode[j] == 1 )
	    s1 += value[j];
	else if ( mode[j] == 2 )
	    joker = TRUE;
    }
    
    //printf("s1 = %i\n", s1);
    int k;
    if ( s1 <= _max && !joker )
    {
        if (!s1)
            s1 = 1;
	for ( k = 0; k < i; k++ )
	{
	    size[k] = ( value[k] * _max ) / s1;
	    //printf("%i %i %i -> %i\n",value[k],_max,s1,size[k] );
	}

	// Calculate the error
	int s2 = 0;
	for ( k = 0; k < i; k++ )
	{
	    s2 += size[k];
	}
	//printf("Error is %i\n",_max - s2);
	// Add the error to the last frame
	size[ i - 1 ] += _max - s2;
    }
    else if ( s1 <= _max && joker )
    {
	int s2 = 0;
	int s3 = 0;
	for ( k = 0; k < i; k++ )
	{
	    if ( mode[k] == 0 || mode[k] == 1 )
	    {
		size[k] = value[k];
		s2 += value[k];
	    }
	    else
		s3 += value[k];
	}
	for ( k = 0; k < i; k++ )
	{
	    if ( mode[k] == 2 )
		size[k] = ( _max - s2 ) * value[k] / s3;
	}
    }
    else
    {
	for ( k = 0; k < i; k++ )
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
	for ( k = 0; k < i; k++ )
	{
	    if ( mode[k] == 0 || mode[k] == 1 )
		s2 += size[k];
	}
	//printf("Error is %i\n",_max - s2);
	// Add the error to the last frame
	size[ i - 1 ] += _max - s2;	
    }

//    for ( k = 0; k < i; k++ )
	//printf("SIZE=%i\n",size[k]);
	
    return i;
}

KHTMLView* HTMLFrameSet::getSelectedFrame()
{
  //printf(">>>>>>>>>>>>>>>>>>>>>>>>>>> Frame 1 <<<<<<<<<<<<<<<<<<<<<<<<\n");
    QWidget *w;
    for ( w = widgetList.first(); w != 0L; w = widgetList.next() )
    {
      //printf(">>>>>>>>>>>>>>>>>>>>>>>>>>> Frame 2 '%s'<<<<<<<<<<<<<<<<<<<<<<<<\n",w->className());
      if ( w->inherits( "KHTMLView" ) )
      {
	//printf(">>>>>>>>>>>>>>>>>>>>>>>>>>> Frame 3 <<<<<<<<<<<<<<<<<<<<<<<<\n");
	KHTMLView *v = (KHTMLView*)w;
	KHTMLView *ret = v->getSelectedView();
	if ( ret )
	  return ret;
      }
    }
    
    //printf(">>>>>>>>>>>>>>>>>>>>>>>>>>> Frame 4 <<<<<<<<<<<<<<<<<<<<<<<<\n");
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

    if ( orientation == HTMLFramePanner::HORIZONTAL )
      setCursor( KCursor::sizeVerCursor() );
    else
      setCursor( KCursor::sizeHorCursor() );
    
    moveable = TRUE;
}

HTMLFramePanner::~HTMLFramePanner()
{
}

void HTMLFramePanner::setIsMoveable( bool _move )
{
  moveable = _move;
  if ( !moveable )
    setCursor( KCursor::arrowCursor() );
  else if ( orientation == HTMLFramePanner::HORIZONTAL )
    setCursor( KCursor::sizeVerCursor() );
  else
    setCursor( KCursor::sizeHorCursor() );
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
    
    if ( orientation == HTMLFramePanner::HORIZONTAL )
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
    
    releaseMouse();

    if ( orientation == HTMLFramePanner::HORIZONTAL )
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

#include "htmlframe.moc"

