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
//
// KDE HTML Widget
//

#include <kurl.h>

#include "htmlchain.h"
#include "htmlobj.h"
#include "htmlclue.h"
#include "html.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <qimage.h>
#include <qdrawutil.h>

//#define CLUE_DEBUG
//#define DEBUG_ALIGN

//-----------------------------------------------------------------------------

HTMLClue::HTMLClue( int _x, int _y, int _max_width, int _percent )
	 : HTMLObject()
{
    x = _x;
    y = _y;
    max_width = _max_width;
    percent = _percent;
    valign = Bottom;
    halign = Left;
    head = tail = curr = 0;

    if ( percent > 0 )
    {
	width = max_width * percent / 100;
	setFixedWidth( false );
    }
    else if ( percent < 0 )
    {
	width = max_width;
	setFixedWidth( false );
    }
    else
	width = max_width;
}

HTMLClue::~HTMLClue()
{
    while ( head )
    {
	curr = head->next();
	delete head;
	head = curr;
    }
}

void HTMLClue::reset()
{
    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->reset();

    curr = 0;
}

void HTMLClue::calcAbsolutePos( int _x, int _y )
{
    HTMLObject *obj;

    int lx = _x + x;
    int ly = _y + y - ascent;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->calcAbsolutePos( lx, ly );
}

HTMLAnchor* HTMLClue::findAnchor( const char *_name, QPoint *_p )
{
    HTMLObject *obj;
    HTMLAnchor *ret;

    _p->setX( _p->x() + x );
    _p->setY( _p->y() + y - ascent );
    
    for ( obj = head; obj != 0; obj = obj->next() )
    {
	ret = obj->findAnchor( _name, _p );
	if ( ret != 0 )
	    return ret;
    }
    
    _p->setX( _p->x() - x );
    _p->setY( _p->y() - y + ascent );

    return 0;
}

bool HTMLClue::getObjectPosition( const HTMLObject *objp, int &xp, int &yp )
{
    HTMLObject *obj;

    xp += x;
    yp += (y - ascent);
    
    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( obj->getObjectPosition( objp, xp, yp ) )
	    return true;
    }
    
    xp -= x;
    yp -= (y - ascent);

    return false;
}

void HTMLClue::getSelected( QStrList &_list )
{
    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->getSelected( _list );
}

void HTMLClue::getSelectedText( QString &_str )
{
    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->getSelectedText( _str );
}

void HTMLClue::select( bool _select )
{
    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	obj->select( _select );
    }
}

void HTMLClue::select( KHTMLWidget *_htmlw, HTMLChain *_chain,
    QRegExp& _pattern, bool _select, int _tx, int _ty )
{
    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

    for ( HTMLObject *obj = head; obj != 0; obj = obj->next() )
    {
	obj->select( _htmlw, _chain, _pattern, _select, _tx, _ty );
    }

    _chain->pop();
}

void HTMLClue::select( KHTMLWidget *_htmlw, HTMLChain *_chain,
    bool _select, int _tx, int _ty )
{
    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

    for ( HTMLObject *obj = head; obj != 0; obj = obj->next() )
    {
	obj->select( _htmlw, _chain, _select, _tx, _ty );
    }

    _chain->pop();
}

void HTMLClue::select( KHTMLWidget *_htmlw, HTMLChain *_chain,
    QRect & _rect, int _tx, int _ty )
{
    HTMLObject *obj;

    QRect r( x + _tx, y - ascent + _ty, width, ascent + descent );

    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

    if ( _rect.contains( r ) )
    {
	for ( obj = head; obj != 0; obj = obj->next() )
	    obj->select( _htmlw, _chain, TRUE, _tx, _ty );
    }
    else if ( !_rect.intersects( r ) )
    {
	for ( obj = head; obj != 0; obj = obj->next() )
	    obj->select( _htmlw, _chain, FALSE, _tx, _ty );
    }
    else
    {
	for ( obj = head; obj != 0; obj = obj->next() )
	    obj->select( _htmlw, _chain, _rect, _tx, _ty );
    }

    _chain->pop();
}

void HTMLClue::selectByURL( KHTMLWidget *_htmlw, HTMLChain *_chain,
    const char *_url, bool _select, int _tx, int _ty )
{
    HTMLObject *obj;

    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->selectByURL( _htmlw, _chain, _url, _select, _tx, _ty );

    _chain->pop();
}

void HTMLClue::findCells( int _tx, int _ty, QList<HTMLCellInfo> &_list )
{
    HTMLObject *obj;

    _tx += x;
    _ty += y - ascent;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->findCells( _tx, _ty, _list );
}

bool HTMLClue::selectText( KHTMLWidget *_htmlw, HTMLChain *_chain, int _x1,
	int _y1, int _x2, int _y2, int _tx, int _ty )
{
    HTMLObject *obj;
    bool isSel = false;

    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	isSel = obj->selectText( _htmlw, _chain, _x1 - x, _y1 - ( y - ascent ),
		_x2 - x, _y2 - ( y - ascent ), _tx, _ty ) || isSel;
    }

    _chain->pop();

    return isSel;
}

HTMLObject* HTMLClue::checkPoint( int _x, int _y )
{
    HTMLObject *obj;
    HTMLObject *obj2;

    if ( _x < x || _x > x + width || _y > y + descent || _y < y - ascent)
	return 0L;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ((obj2 = obj->checkPoint( _x - x, _y - (y - ascent) )) != 0L)
	    return obj2;
    }

    return 0;
}

HTMLObject* HTMLClue::mouseEvent( int _x, int _y, int button, int state )
{
    HTMLObject *obj;
    HTMLObject *obj2;

    if ( _x < x || _x > x + width || _y > y + descent || _y < y - ascent)
	return 0;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ((obj2 = obj->mouseEvent( _x - x, _y - (y - ascent), button,
		state )) != 0 )
	    return obj2;
    }

    return 0;
}

void HTMLClue::calcSize( HTMLClue * )
{
    // If we have already called calcSize for the children, then just
    // continue from the last object done in previous call.
    if ( !curr )
    {
	ascent = 0;
	curr = head;
    }

    while ( curr != 0 )
    {
	curr->calcSize( this );
	curr = curr->next();
    }

    // remember the last object so that we can start from here next time
    // we are called.
    curr = tail;
}

void HTMLClue::recalcBaseSize( QPainter *_painter )
{
    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->recalcBaseSize( _painter );
}

int HTMLClue::calcMinWidth()
{
    HTMLObject *obj;
    int minWidth = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	int w = obj->calcMinWidth();
	if ( w > minWidth )
	    minWidth = w;
    }

    if ( isFixedWidth() )
    {
        if (width > minWidth)
            minWidth = width;
    }

    return minWidth;
}

int HTMLClue::calcPreferredWidth()
{
    if ( isFixedWidth() )
	return width;

    HTMLObject *obj;
    int prefWidth = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	int w = obj->calcPreferredWidth();
	if ( w > prefWidth )
	    prefWidth = w;
    }

    return prefWidth;
}

void HTMLClue::setMaxAscent( int _a )
{
    HTMLObject *obj;

    if ( valign == VCenter )
    {
	for ( obj = head; obj != 0; obj = obj->next() )
	{	
	    obj->setYPos( obj->getYPos() + ( _a - ascent )/2 );
	}
    }
    else if ( valign == Bottom )
    {
	for ( obj = head; obj != 0; obj = obj->next() )
	{	
	    obj->setYPos( obj->getYPos() + _a - ascent );
	}
    }

    ascent = _a;
}

void HTMLClue::setMaxDescent( int _d )
{
    HTMLObject *obj;

    if ( valign == VCenter )
    {
	for ( obj = head; obj != 0; obj = obj->next() )
	{	
	    obj->setYPos( obj->getYPos() + ( _d - descent )/2 );
	}
    }
    else if ( valign == Bottom )
    {
	for ( obj = head; obj != 0; obj = obj->next() )
	{	
	    obj->setYPos( obj->getYPos() + _d - descent );
	}
    }

    descent = _d;
}

int HTMLClue::findPageBreak( int _y )
{
    if ( _y > y )
	return -1;

    HTMLObject *obj;
    int pos;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( !obj->isAligned() )
	{
	    pos = obj->findPageBreak( _y - ( y - ascent ) );
	    if ( pos >= 0 )
		return ( pos + y - ascent );
	}
    }

    return -1;
}

bool HTMLClue::print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter )
{
    if ( _y + _height < y - getAscent() || _y > y + descent )
	return false;
    
    HTMLObject *obj;

    _tx += x;
    _ty += y - ascent;

#ifdef CLUE_DEBUG
    // draw rectangles around clues - for debugging
    QBrush brush;
    _painter->setBrush( brush );
    _painter->drawRect( _tx, _ty, width, getHeight() );
#endif

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( !obj->isAligned() )
	{
	    if ( obj->print( _painter, _x - x, _y - (y - ascent),
			_width, _height, _tx, _ty, toPrinter ) && toPrinter )
	    return true;
	}
    }

    return false;
}

void HTMLClue::print( QPainter *_painter, HTMLChain *_chain, int _x, int _y,
    int _width, int _height, int _tx, int _ty )
{
    _tx += x;
    _ty += y - ascent;

    _chain->next();

    if ( _chain->current() )
    {
        if ( _chain->isLast() )
        {
            // if a clue is the last in the chain, then draw its children too.
            print( _painter, _tx, _ty );
        }
        else
        {
            _chain->current()->print( _painter, _chain, _x - x,
                _y - (y - ascent), _width, _height, _tx, _ty );
        }
    }
}

void HTMLClue::print( QPainter *_painter, int _tx, int _ty )
{
    print( _painter, 0, 0, 0xFFFF, 0xFFFF, _tx, _ty, false );
}

void HTMLClue::print( QPainter *_painter, HTMLObject *_obj, int _x, int _y, int _width, int _height, int _tx, int _ty )
{
    if ( _y + _height < y - getAscent() || _y > y + descent )
	return;
    
    HTMLObject *obj;

    _tx += x;
    _ty += y - ascent;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( obj == _obj )
	{
	    obj->print( _painter, _x - x, _y - (y - ascent), _width,	
		_height, _tx, _ty, false );
	    return;
	}
	else
	    obj->print( _painter, _obj, _x - x, _y - (y - ascent),
		_width, _height, _tx, _ty );
    }
}

void 
HTMLClue::setBgColor( QColor c )
{
    HTMLObject *obj;
    for ( obj = head; obj != 0; obj = obj->next() )
	obj->setBgColor( c );
}

//-----------------------------------------------------------------------------

HTMLClueV::HTMLClueV( int _x, int _y, int _max_width, int _percent )
    : HTMLClue( _x, _y, _max_width, _percent )
{
    alignLeftList  = 0;
    alignRightList = 0;

    padding = 0;
}

void HTMLClueV::reset()
{
    HTMLClue::reset();

    alignLeftList  = 0;
    alignRightList = 0;
}

void HTMLClueV::setMaxWidth( int _max_width )
{
    HTMLObject *obj;

    if ( !isFixedWidth() )
    {
	max_width = _max_width;
	if ( percent > 0 )
	    width = _max_width * percent / 100;
	else
	    width = max_width;
    }

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->setMaxWidth( width );
}

HTMLObject* HTMLClueV::checkPoint( int _x, int _y )
{
    HTMLObject *obj2;

    if ( ( obj2 = HTMLClue::checkPoint( _x, _y ) ) != 0L )
	    return obj2;

    if ( _x < x || _x > x + width || _y > y + descent || _y < y - ascent)
	return 0L;

    HTMLClueAligned *clue;
    for ( clue = alignLeftList; clue != 0; clue = clue->nextClue() )
    {
	if ((obj2 = clue->checkPoint( _x - x - clue->parent()->getXPos(),
		 _y - (y - ascent) - ( clue->parent()->getYPos() -
		 clue->parent()->getAscent() ) )) != 0L)
	    return obj2;
    }
    for ( clue = alignRightList; clue != 0; clue = clue->nextClue() )
    {
	if ((obj2 = clue->checkPoint( _x - x - clue->parent()->getXPos(),
		 _y - (y - ascent) - ( clue->parent()->getYPos() -
		 clue->parent()->getAscent() ) )) != 0L)
	    return obj2;
    }

    return 0L;
}

HTMLObject* HTMLClueV::mouseEvent( int _x, int _y, int button, int state )
{
    HTMLObject *obj2;

    if ( ( obj2 = HTMLClue::mouseEvent( _x, _y, button, state ) ) != 0L )
	    return obj2;

    if ( _x < x || _x > x + width || _y > y + descent || _y < y - ascent)
	return 0;

    HTMLClueAligned *clue;
    for ( clue = alignLeftList; clue != 0; clue = clue->nextClue() )
    {
	if ((obj2 = clue->mouseEvent( _x - x - clue->parent()->getXPos(),
		 _y - (y - ascent) - ( clue->parent()->getYPos() -
		 clue->parent()->getAscent() ), button, state )) != 0 )
	    return obj2;
    }
    for ( clue = alignRightList; clue != 0; clue = clue->nextClue() )
    {
	if ((obj2 = clue->mouseEvent( _x - x - clue->parent()->getXPos(),
		 _y - (y - ascent) - ( clue->parent()->getYPos() -
		 clue->parent()->getAscent() ), button, state )) != 0 )
	    return obj2;
    }

    return 0;
}

void HTMLClueV::calcSize( HTMLClue *parent )
{
	int lmargin = parent ? parent->getLeftMargin( getYPos() ) + padding : padding;

	// If we have already called calcSize for the children, then just
	// continue from the last object done in previous call.
	if ( curr )
	{
	    ascent = padding;
	    // get the current ascent not including curr
	    HTMLObject *obj = head;
	    while ( obj != curr )
	    {
		ascent += obj->getHeight();
		obj = obj->next();
	    }
	    // remove any aligned objects previously added by the current
	    // object.
	    removeAlignedByParent( curr );
	}
	else
	{
	    ascent = padding;
        descent = 0;
	    curr = head;
	}

	while ( curr != 0 )
	{
	    // Set an initial ypos so that the alignment stuff knows where
	    // the top of this object is
	    curr->setYPos( ascent );
	    curr->calcSize( this );
	    if ( curr->getWidth() > width - ( padding << 1) )
		    width = curr->getWidth() + ( padding << 1 );
	    ascent += curr->getHeight();
	    curr->setPos( lmargin, ascent - curr->getDescent() );
	    curr = curr->next();
	}

    ascent += padding;

	// remember the last object so that we can start from here next time
	// we are called.
	curr = tail;

	if ((max_width != 0) && (width > max_width))
		width = max_width;

	HTMLObject *obj;

	if ( halign == HCenter )
	{
	    for ( obj = head; obj != 0; obj = obj->next() )
		obj->setXPos( lmargin + (width - obj->getWidth()) / 2 );
	}
	else if ( halign == Right )
	{
	    for ( obj = head; obj != 0; obj = obj->next() )
		obj->setXPos( lmargin + width - obj->getWidth() );
	}

	HTMLClueAligned *clue;
	for ( clue = alignLeftList; clue != 0; clue = clue->nextClue() )
	{
	    if ( clue->getYPos() + clue->parent()->getYPos() -
		    clue->parent()->getAscent() > ascent )
		ascent = clue->getYPos() + clue->parent()->getYPos() -
			    clue->parent()->getAscent();
	}
	for ( clue = alignRightList; clue != 0; clue = clue->nextClue() )
	{
	    if ( clue->getYPos() + clue->parent()->getYPos() -
		    clue->parent()->getAscent() > ascent )
		ascent = clue->getYPos() + clue->parent()->getYPos() -
			    clue->parent()->getAscent();
	}
}

bool HTMLClueV::print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter )
{
    bool rv = HTMLClue::print( _painter, _x, _y, _width, _height, _tx, _ty, toPrinter );

    // print aligned objects
    if ( _y + _height < y - getAscent() || _y > y + descent )
	return rv;
    
    _tx += x;
    _ty += y - ascent;

    HTMLClueAligned *clue;

    for ( clue = alignLeftList; clue != 0; clue = clue->nextClue() )
    {
	clue->print( _painter, _tx + clue->parent()->getXPos(),
		_ty + clue->parent()->getYPos() - clue->parent()->getAscent() );
    }

    for ( clue = alignRightList; clue != 0; clue = clue->nextClue() )
    {
	clue->print( _painter, _tx + clue->parent()->getXPos(),
		_ty + clue->parent()->getYPos() - clue->parent()->getAscent() );
    }
    
    return rv;
}

void HTMLClueV::findFreeArea( int _y, int _width, int _height, int _indent,
                              int *_y_pos, int *_lmargin, int *_rmargin)
// This method tries to find a free rectangular area of _width x _height
// from position _y on. The start of this area is written in *y_pos.
// The actual left and right margins of the area are returned in
// *lmargin and *rmargin.
{                             
    int try_y = _y;
    int lmargin;
    int rmargin;
    HTMLClueAligned *clue;
    int next_y, top_y, base_y;

#ifdef DEBUG_ALIGN
// Debug alignment lists
    //printf("Find w,h = %d,%d from Y-pos %d on\n", _width, _height, _y);
    //printf("aligLeftList:\n");

    for ( clue = alignLeftList; clue != 0; clue = clue->nextClue() )
    {
        base_y = clue->getYPos() + clue->parent()->getYPos() -
         		 clue->parent()->getAscent();
	top_y = base_y - clue->getAscent();
        //printf("    x,y [x,y] / w,h = %d,%d [%d,%d] / %d, %d\n",
        	clue->getXPos()+clue->parent()->getXPos(), 
        	top_y,
        	clue->getXPos(), clue->getYPos(),
        	clue->getWidth(), clue->getHeight());
    }
    //printf("aligRightList:\n");

    for ( clue = alignRightList; clue != 0; clue = clue->nextClue() )
    {
        base_y = clue->getYPos() + clue->parent()->getYPos() -
         		 clue->parent()->getAscent();
	top_y = base_y - clue->getAscent();
        //printf("    x,y [x,y] / w,h = %d,%d [%d,%d] / %d, %d\n",
        	clue->getXPos()+clue->parent()->getXPos(), 
        	top_y,
        	clue->getXPos(), clue->getYPos(),
        	clue->getWidth(), clue->getHeight());
    }
#endif    
    // Find a suitable position
    while(1) 
    {
    	// try position try_y
    	lmargin = _indent;
	rmargin = max_width;
        next_y = 0;
    
	// Calculate left margin
        for ( clue = alignLeftList; clue != 0; clue = clue->nextClue() )
        {
           base_y = clue->getYPos() + clue->parent()->getYPos() -
         		 clue->parent()->getAscent();
	   top_y = base_y - clue->getAscent();

	   if ((top_y <= try_y+_height) && (base_y > try_y))
	   {
	      int lm = clue->getXPos() + clue->getWidth();
	      if (lm > lmargin)
	         lmargin = lm;
	         
	      if ((next_y == 0) || (base_y < next_y))
	      {
	         next_y = base_y;
	      }
           }
        }

	// Calculate right margin
        for ( clue = alignRightList; clue != 0; clue = clue->nextClue() )
        {
	
	   base_y = clue->getYPos() + clue->parent()->getYPos() -
	        	 clue->parent()->getAscent();
	   top_y = base_y - clue->getAscent();

	   if ((top_y <= try_y+_height) && (base_y > try_y))
	   {
	      int rm = clue->getXPos();
	      if (rm < rmargin)
	         rmargin = rm;

	      if ((next_y == 0) || (base_y < next_y))
	      {
	          next_y = base_y;
              }
	   }
        }
        
        /* no margins here.. just put it in */
        if ((lmargin == _indent) && (rmargin == max_width))
            break;

        /* object fits within current margins */
	if ((rmargin-lmargin) >= _width)
	    break;

	/* Object does not fit here.... increase Y */
	try_y = next_y;
    }
    *_y_pos = try_y;
    *_rmargin = rmargin;
    *_lmargin = lmargin;

#ifdef DEBUG_ALIGN
    //printf("Got y=%d, lmargin = %d, rmargin =%d\n", try_y, lmargin, rmargin);
#endif
}


void HTMLClueV::appendLeftAligned( HTMLClueAligned *_clue )
// This method adds _clue to the left aligned list.
// Its X-position is calculated based on the left margin
// at the Y-position of _clue. If _clue does not fit,
// its Y-position will be increased until it fits.
{
    int y_pos;
    int start_y = 0;
    int lmargin;
    int rmargin;
    HTMLClueAligned *clue;
    
    clue = alignLeftList;
    if (clue)
    {
        while ( clue->nextClue() )
        {
           clue = clue->nextClue();
        }
        y_pos = clue->getYPos() + 
        	clue->parent()->getYPos() - clue->parent()->getAscent();
	if (y_pos > start_y)
	    start_y = y_pos;
    }
    
    y_pos = _clue->getYPos() + 
    	    _clue->parent()->getYPos() - _clue->parent()->getAscent();

    if (y_pos > start_y)
        start_y = y_pos;

    // Start looking for space from the position of the last object in 
    // the left-aligned list on, or from the current position of the
    // object.
    findFreeArea(start_y - _clue->getAscent(), 
    		_clue->getWidth(), _clue->getHeight(), 0,
    		&y_pos, &lmargin, &rmargin);
	
    // Set position
    _clue->setPos(lmargin, 
    	          y_pos - 
    	          _clue->parent()->getYPos() + _clue->parent()->getAscent() + 
    	          _clue->getAscent());	

    // Insert clue in align list
    if ( !alignLeftList )
    {
	alignLeftList = _clue;
	_clue->setNextClue( 0 );
    }
    else
    {
	HTMLClueAligned *obj = alignLeftList;
	while ( obj->nextClue() )
	{
	    if ( obj == _clue )
	    {
	    	//printf("%s:%d Clue already in alignLeftList\n", __FILE__, __LINE__);
		return;
	    }
	    obj = obj->nextClue();
	}
	if ( obj == _clue )
	{
	    //printf("%s:%d Clue already in alignLeftList\n", __FILE__, __LINE__);
	    return;
	}
	obj->setNextClue( _clue );
	_clue->setNextClue( 0 );
    }
}

void HTMLClueV::appendRightAligned( HTMLClueAligned *_clue )
// This method adds _clue to the right aligned list.
// Its X-position is calculated based on the right margin
// at the Y-position of _clue. If _clue does not fit,
// its Y-position will be increased until it fits.
{
    int y_pos;
    int start_y = 0;
    int lmargin;
    int rmargin;
    HTMLClueAligned *clue;
    
    clue = alignRightList;
    if (clue)
    {
        while ( clue->nextClue() )
        {
           clue = clue->nextClue();
        }
        y_pos = clue->getYPos() + clue->parent()->getYPos();
	if (y_pos > start_y)
	    start_y = y_pos;
    }
    
    y_pos = _clue->getYPos() + _clue->parent()->getYPos();
    if (y_pos > start_y)
        start_y = y_pos;

    // Start looking for space from the position of the last object in 
    // the left-aligned list on, or from the current position of the
    // object.
    findFreeArea(start_y - _clue->getAscent(), 
    		_clue->getWidth(), _clue->getHeight(), 0,
    		&y_pos, &lmargin, &rmargin);
	
    // Set position
    _clue->setPos(rmargin - _clue->getWidth(), 
    	          y_pos - _clue->parent()->getYPos() + _clue->getAscent());	

    // Insert clue in align list
    if ( !alignRightList )
    {
	alignRightList = _clue;
	_clue->setNextClue( 0 );
    }
    else
    {
	HTMLClueAligned *obj = alignRightList;
	while ( obj->nextClue() )
	{
	    if ( obj == _clue )
	    {
		//printf("%s:%d Clue already in alignRightList\n", __FILE__, __LINE__);
		return;
	    }
	    obj = obj->nextClue();
	}
	if ( obj == _clue )
	{
	    //printf("%s:%d Clue already in alignRightList\n", __FILE__, __LINE__);
	    return;
	}
	obj->setNextClue( _clue );
	_clue->setNextClue( 0 );
    }
}

int HTMLClueV::appended( HTMLClueAligned *_clue )
// Returns whether _clue is already in the alignList
{
    HTMLClueAligned *clue;
    
    if (_clue->getHAlign() == Left)
    {
	clue = alignLeftList;
    }
    else
    {
    	clue = alignRightList;
    }
    while ( clue)
    {
    	if (clue == _clue) 
    	    return 1;
        clue = clue->nextClue();
    }
    return 0;
}

// This is a horrible hack so that the progressive size calculation is
// not stuffed up by aligned clues added in a previous pass on the
// current clue
//
void HTMLClueV::removeAlignedByParent( HTMLObject *p )
{
    HTMLClueAligned *tmp, *obj;

    tmp = 0;
    obj = alignLeftList;

    while ( obj )
    {
	if ( obj->parent() == p )
	{
	    if ( tmp )
	    {
		tmp->setNextClue( obj->nextClue() );
		tmp = obj;
	    }
	    else
	    {
		alignLeftList = obj->nextClue();
		tmp = 0;
	    }
	}
	else
	    tmp = obj;
	obj = obj->nextClue();
    }

    tmp = 0;
    obj = alignRightList;

    while ( obj )
    {
	if ( obj->parent() == p )
	{
	    if ( tmp )
	    {
		tmp->setNextClue( obj->nextClue() );
		tmp = obj;
	    }
	    else
	    {
		alignRightList = obj->nextClue();
		tmp = 0;
	    }
	}
	else
	    tmp = obj;
	obj = obj->nextClue();
    }
}

int HTMLClueV::getLeftMargin( int _y )
{
    int margin = 0;
    HTMLClueAligned *clue;

    for ( clue = alignLeftList; clue != 0; clue = clue->nextClue() )
    {
	if ( clue->getYPos() - clue->getAscent() + clue->parent()->getYPos() -
		    clue->parent()->getAscent() <= _y &&
		    clue->getYPos() + clue->parent()->getYPos() -
		    clue->parent()->getAscent() > _y )
	    margin = clue->getXPos() + clue->getWidth();
    }

    return margin;
}

int HTMLClueV::getRightMargin( int _y )
{
    int margin = max_width;
    HTMLClueAligned *clue;

    for ( clue = alignRightList; clue != 0; clue = clue->nextClue() )
    {
	if ( clue->getYPos()-clue->getAscent()+clue->parent()->getYPos() -
		clue->parent()->getAscent() <= _y &&
		clue->getYPos() + clue->parent()->getYPos() -
		clue->parent()->getAscent() > _y )
	    margin = clue->getXPos();
    }

    return margin;
}

int HTMLClueV::getLeftClear( int _y )
{
    HTMLClueAligned *clue;
    int top_y, base_y;
    	
    // try position _y
    for ( clue = alignLeftList; clue != 0; clue = clue->nextClue() )
    {
	base_y = clue->getYPos() + clue->parent()->getYPos() -
                clue->parent()->getAscent();
	top_y = base_y - clue->getAscent();

	if ((top_y <= _y) && (base_y > _y))
	{
	    _y = base_y;
        }
    }
    return _y;
}

int HTMLClueV::getRightClear( int _y )
{
    HTMLClueAligned *clue;
    int top_y, base_y;
    	
    // try position _y
    for ( clue = alignRightList; clue != 0; clue = clue->nextClue() )
    {
	base_y = clue->getYPos() + clue->parent()->getYPos() -
                clue->parent()->getAscent();
	top_y = base_y - clue->getAscent();

	if ((top_y <= _y) && (base_y > _y))
	{
	    _y = base_y;
        }
    }
    return _y;
}

//-----------------------------------------------------------------------------

HTMLCell::HTMLCell( int _x, int _y, int _max_width, int _percent, const char *_url, const char *_target ) :
  HTMLClueV( _x, _y, _max_width, _percent )
{
  url = _url;
  target = _target;
  bIsMarked = false;
  padding = 1;      // leave a little room for the selection rectangle.
}

void HTMLCell::select( KHTMLWidget *_htmlw, HTMLChain *_chain, QRect & _rect, int _tx, int _ty )
{
    HTMLObject *obj;

    QRect r( x + _tx, y - ascent + _ty, width, ascent + descent );

    _tx += x;
    _ty += y - ascent;

    bool sel = false;

    if ( _rect.contains( r ) )
    {
	sel = true;
    }
    else if ( !_rect.intersects( r ) )
    {
	sel = false;
    }
    else
    {
	QRect isect = _rect.intersect( r );
	if ( isect.width() > r.width()/2 && isect.height() > r.height()/2 )
	    sel = true;
    }

    _chain->push( this );

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->select( _htmlw, _chain, sel, _tx, _ty );

    _chain->pop();
}

bool HTMLCell::print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter )
{
  bool rv = HTMLClueV::print( _painter, _x, _y, _width, _height, _tx, _ty, toPrinter );
  
  // print aligned objects
  if ( _y + _height < y - getAscent() || _y > y + descent )
    return rv;
  
  if ( !toPrinter && bIsMarked )
  {
    QPen pen( _painter->pen() );
    QPen newPen( black );
    _painter->setPen( newPen );
    _painter->drawRect( _tx + x, _ty + y - ascent, width, ascent + descent );
    newPen.setColor( white );
    newPen.setStyle( DotLine );
    _painter->setPen( newPen );
    _painter->drawRect( _tx + x, _ty + y - ascent, width, ascent + descent );
    _painter->setPen( pen );
  }
    
  return rv;
}

void HTMLCell::findCells( int _tx, int _ty, QList<HTMLCellInfo> &_list )
{
    HTMLCellInfo *p = new HTMLCellInfo;
    p->pCell = this;
    p->xAbs = _tx + x;
    p->baseAbs = _ty + y;
    p->tx = _tx;
    p->ty = _ty;
    
    _list.append( p );
}

//-----------------------------------------------------------------------------

bool HTMLClueH::selectText( KHTMLWidget *_htmlw, HTMLChain *_chain,
	int _x1, int _y1, int _x2, int _y2, int _tx, int _ty )
{
    HTMLObject *obj;
    bool isSel = false;
    int a = 0, d = 0;

    if ( !head )
	return false;

    _tx += x;
    _ty += y - ascent;

    // get max ascent and descent
    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( obj->getAscent() > a )
	    a = obj->getAscent();
	if ( obj->getDescent() > d )
	    d = obj->getDescent();
    }

    int rely1 = _y1 - ( y - ascent );
    int rely2 = _y2 - ( y - ascent );
    int ypos = head->getYPos();

    if ( rely1 > ypos - a && rely1 < ypos + d )
	rely1 = ypos-1;

    if ( rely2 > ypos - a && rely2 < ypos + d )
	rely2 = ypos;

    _chain->push( this );

    // (de)select objects
    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( obj->getObjectType() == Clue )
	    isSel = obj->selectText( _htmlw, _chain, _x1 - x, _y1 - (y-ascent),
		    _x2 - x, _y2 - ( y - ascent ), _tx, _ty ) || isSel;
	else
	    isSel = obj->selectText( _htmlw, _chain, _x1 - x, rely1,
		    _x2 - x, rely2, _tx, _ty ) || isSel;
    }

    _chain->pop();

    return isSel;
}

void HTMLClueH::getSelectedText( QString &_str )
{
    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	    obj->getSelectedText( _str );
    }

    if ( tail && tail->isSelected() )
        _str += '\n';
}

void HTMLClueH::setMaxWidth( int _w )
{
    HTMLObject *obj;
    max_width = _w;

    // first calculate width minus fixed width objects
    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( obj->getPercent() <= 0 )	// i.e. fixed width objects
	    _w -= obj->getWidth();
    }

    // now call setMaxWidth for variable objects
    for ( obj = head; obj != 0; obj = obj->next() )
	if ( obj->getPercent() > 0 )
	    obj->setMaxWidth( _w - indent );
}

void HTMLClueH::calcSize( HTMLClue *parent )
{
    // make sure children are properly sized
    setMaxWidth( max_width );

    HTMLClue::calcSize( parent );
    
    HTMLObject *obj;
    int lmargin = 0;
    
    if ( parent )
	lmargin = parent->getLeftMargin( getYPos() );

    width = lmargin + indent;
    descent = 0;
    ascent = 0;

    int a = 0;
    int d = 0;
    for ( obj = head; obj != 0; obj = obj->next() )
    {
    	obj->fitLine( (obj == head), true, -1);
	obj->setXPos( width );
	width += obj->getWidth();
	if ( obj->getAscent() > a )
	    a = obj->getAscent();
	if ( obj->getDescent() > d )
	    d = obj->getDescent();
    }

    ascent = a+d;

    switch ( valign )
    {
	case Top:
	    for ( obj = head; obj != 0; obj = obj->next() )
		obj->setYPos( obj->getAscent() );
	    break;

	case VCenter:
	    for ( obj = head; obj != 0; obj = obj->next() )
		obj->setYPos( ascent/2 );
	    break;

	default:
	    for ( obj = head; obj != 0; obj = obj->next() )
		obj->setYPos( ascent - d );
    }
}

int HTMLClueH::calcMinWidth()
{
    HTMLObject *obj;
    int minWidth = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
	minWidth += obj->calcMinWidth();

    return minWidth + indent;
}
 
int HTMLClueH::calcPreferredWidth()
{
    HTMLObject *obj;
    int prefWidth = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
	prefWidth += obj->calcPreferredWidth();
	
    return prefWidth + indent;
}
    
//-----------------------------------------------------------------------------

// process one line at a time, making sure that all objects on a line
// are able to be selected if the cursor is within the maximum
// ascent and descent of the line.
//
bool HTMLClueFlow::selectText( KHTMLWidget *_htmlw, HTMLChain *_chain,
	int _x1, int _y1, int _x2, int _y2, int _tx, int _ty )
{
    HTMLObject *lineEnd = head, *obj = head;
    bool isSel = false;
    int ypos, a, d, rely1, rely2;

    _tx += x;
    _ty += y - ascent;

    _chain->push( this );

    while ( lineEnd )
    {
	ypos = lineEnd->getYPos();
	a = lineEnd->getAscent();
	d = lineEnd->getDescent();

	while ( lineEnd && lineEnd->getYPos() == ypos )
	{
	    if ( lineEnd->getAscent() > a )
		a = lineEnd->getAscent();
	    if ( lineEnd->getDescent() > d )
		d = lineEnd->getDescent();
	    lineEnd = lineEnd->next();
	}

	rely1 = _y1 - ( y - ascent );
	rely2 = _y2 - ( y - ascent );

	if ( rely1 > ypos - a && rely1 < ypos + d )
	    rely1 = ypos-1;

	if ( rely2 > ypos - a && rely2 < ypos + d )
	    rely2 = ypos;

	while ( obj != lineEnd )
	{
	    if ( obj->getObjectType() == Clue )
		isSel = obj->selectText(_htmlw,_chain,_x1 - x, _y1 - (y-ascent),
			_x2 - x, _y2 - ( y - ascent ), _tx, _ty ) || isSel;
	    else
		isSel = obj->selectText(_htmlw,_chain,_x1 - x, rely1,
			_x2 - x, rely2, _tx, _ty ) || isSel;
	    obj = obj->next();
	}
    }

    _chain->pop();

    return isSel;
}

void HTMLClueFlow::getSelectedText( QString &_str )
{
    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( obj != head || !obj->isSeparator() )
	    obj->getSelectedText( _str );
    }

    if ( tail && tail->isSelected() && !tail->isNewline() )
	_str += '\n';
}

// MRJ - 26/10/97:  I've given this a rewrite so that its layout is done
// correctly, i.e. break ONLY on separators and newlines instead of breaking
// after any object.  This is much nicer and smaller code now also.  Sorry
// if I broke something.
// 
void HTMLClueFlow::calcSize( HTMLClue *parent )
{
//    HTMLClue::calcSize( parent );

    HTMLObject *obj = head;
    HTMLObject *line = head;
    HTMLVSpace::Clear clear = HTMLVSpace::CNone;;
    int lmargin, rmargin;

    ascent = 0;
    descent = 0;
    width = 0;
    lmargin = parent->getLeftMargin( getYPos() );
    if ( indent > lmargin )
	lmargin = indent;
    rmargin = parent->getRightMargin( getYPos() );
    int w = lmargin;
    int a = 0;
    int d = 0;

    bool newLine = false;

    while ( obj != 0 )
    {
	// If we get a newline object, set newLine=true so that the current
	// line will be aligned, and the next line prepared.
	if ( obj->isNewline() )
	{
// WABA: Only update the height if this line has no height already
	    if ( !a )
		a = obj->getAscent();
	    if ( !a && (obj->getDescent() > d) )
		d = obj->getDescent();
	    newLine = true;
	    HTMLVSpace *vs = (HTMLVSpace *)obj;
	    clear = vs->clear();
	    obj = obj->next();
	}
	// add a separator
	else if ( obj->isSeparator() )
	{
	    obj->setXPos( w );

	    // skip a space at the start of a line
	    if ( w != lmargin )
	    {
		w += obj->getWidth();
	        if ( obj->getAscent() > a )
	            a = obj->getAscent();

	    	if ( obj->getDescent() > d )
		    d = obj->getDescent();
	    }
	    
	    obj = obj->next();
	}
	// a left or right aligned object is not added in this line.  It
	// is added to our parent's list of aligned objects and will be
	// taken into account in subsequent get*Margin() calls.
	else if ( obj->isAligned() )
	{
	    HTMLClueAligned *c = (HTMLClueAligned *)obj;

	    if ( !parent->appended( c ) )
	    {
	        obj->calcSize();

	        if ( c->getHAlign() == Left )
	        {
		    c->setPos( lmargin,
		        ascent + c->getAscent() );

  		    parent->appendLeftAligned( c );
	        }
	        else
	        {
		    c->setPos( rmargin - c->getWidth(),
		        ascent + c->getAscent() );

  		    parent->appendRightAligned( c );
	        }
	    }
	    obj = obj->next();
	}
	// This is a normal object.  We must add all objects upto the next
	// separator/newline/aligned object.
	else
	{
	    // By setting "newLine = true" we move the complete run to
	    // a new line.
	    // We shouldn't set newLine if we are at the start of a line. 
		
	    int runWidth = 0;
	    HTMLObject *run = obj;
	    while ( run && !run->isSeparator() && !run->isNewline() &&
		    !run->isAligned() )
	    {
		HTMLFitType fit;
		run->setMaxWidth( rmargin - lmargin );
		fit = run->fitLine( (w+runWidth == lmargin), // Start of line
				    (obj == line), // first run
				    rmargin-runWidth-w);
		
		if ( fit == HTMLNoFit)
		{
		    newLine = true;
		    break;		
		}
		//run->calcSize(parent);
		// Lars: the above gives wrong lmargins, since run 
		// run->setSize() usually calls 
		// parent->getLeftMargin( getYSize() ) to determine the
		// left margin. But y is relative to the clues parent,
		// not absolute... So this should fix it.
		run->calcSize(this);
		runWidth += run->getWidth();

		// If this run cannot fit in the allowed area, break it
		// on a non-seperator. 
		// Don't break here, if it is the first object.
		if (( run != obj) && (runWidth > rmargin - lmargin ))
		{
		     break;
		}    

	 	if ( run->getAscent() > a )
		    a = run->getAscent();
		if ( run->getDescent() > d )
		    d = run->getDescent();

		run = run->next();
		
		if (fit == HTMLPartialFit)
		{
		    // We encountered an implicit separator
		    break;
		}

		// If this is the first object but it doesn't fit the 
		// allowed area, break directly after it.
		if (runWidth > rmargin - lmargin)
		{
		     break;
		}    
	    }

	    // if these objects do not fit in the current line and we are
	    // not at the start of a line then end the current line in
	    // preparation to add this run in the next pass.
	    if ( w > lmargin && w + runWidth > rmargin )
	    {
		newLine = true;
	    }
	    
	    if (!newLine)
	    {
	    	int new_y, new_lmargin, new_rmargin;

		// Check whether this run still fits the current flow area, 
		// especially with respect to its height.
 		// If not, find a rectangle with height a+b. The size of
 		// the rectangle will be rmargin-lmargin.

		parent->findFreeArea(y, line->getWidth(),
			a+d, indent, &new_y, &new_lmargin, &new_rmargin);

		if (
		    (new_y != y) || 
		    (new_lmargin > lmargin) || 
		    (new_rmargin < rmargin)
		   )
		{
		    // We did not get the location we expected.
		    // We start building our current line again

 		    // We got shifted downwards by "new_y - y"
		    // add this to both "y" and "ascent"
		    new_y -= y;
		    y += new_y;
		    ascent += new_y;

		    lmargin = new_lmargin;
		    if ( indent > lmargin )
			lmargin = indent;
		    rmargin = new_rmargin;
		    obj = line;

		    // Reset this line
		    w = lmargin;
		    d = 0;
		    a = 0;

		    newLine = false;
		    clear = HTMLVSpace::CNone;
		}	
		else
		{
  		    while ( obj != run )
		    {
		        obj->setXPos( w );
		        w += obj->getWidth();
		        obj = obj->next();
		    }
		}
	    }
	}

	// if we need a new line, or all objects have been processed
	// and need to be aligned.
	if ( newLine || !obj )
	{
	    int extra = 0;
	    ascent += a + d;
	    y += a + d;

	    if ( w > width )
		width = w;

	    if ( halign == HCenter )
	    {
		extra = ( rmargin - w ) / 2;
		if ( extra < 0 )
		    extra = 0;
	    }
	    else if ( halign == Right )
	    {
	        extra = rmargin - w;
		if ( extra < 0 )
		    extra = 0;
	    }

	    while ( line != obj )
	    {
		if ( !line->isAligned() )
		{
		    line->setYPos( ascent - d );
		    line->setMaxAscent( a );
		    line->setMaxDescent( d );
		    if ( halign == HCenter || halign == Right )
		    {
			line->setXPos( line->getXPos() + extra );
		    }
		}
		line = line->next();
	    }

	    int oldy = y;
	    
	    if ( clear == HTMLVSpace::All )
	    {
	    	int new_lmargin, new_rmargin;

		parent->findFreeArea(oldy, max_width,
			1, 0, &y, &new_lmargin, &new_rmargin);
	    }
	    else if ( clear == HTMLVSpace::Left)
	    {
		y = parent->getLeftClear( oldy );
	    }
	    else if ( clear == HTMLVSpace::Right)
	    {
		y = parent->getRightClear( oldy );
	    }

	    ascent += y-oldy;

	    lmargin = parent->getLeftMargin( y );
	    if ( indent > lmargin )
		lmargin = indent;
	    rmargin = parent->getRightMargin( y );

	    w = lmargin;
	    d = 0;
	    a = 0;

	    newLine = false;
	    clear = HTMLVSpace::CNone;
	}
    }

    if ( width < max_width )
	    width = max_width;
}

int HTMLClueFlow::findPageBreak( int _y )
{
    if ( _y > y )
	return -1;

    HTMLObject *obj;
    int pos, minpos, yp;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	yp = obj->getYPos();
	minpos = yp;

	while ( obj && obj->getYPos() == yp )
	{
	    if ( !obj->isAligned() )
	    {
		pos = obj->findPageBreak( _y - ( y - ascent ) );
		if ( pos >= 0 && pos < minpos )
		    minpos = pos;
	    }
	    obj = obj->next();
	}
	if ( minpos != yp )
	    return ( minpos + y - ascent );
	if (!obj)
	    return -1;
    }

    return -1;
}

int HTMLClueFlow::calcMinWidth()
{
#if 1
    HTMLObject *obj;
    int minWidth = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	int w = obj->calcMinWidth();
	if ( w > minWidth )
	    minWidth = w;
    }
	
    minWidth += indent;	

    if ( isFixedWidth() )
    {
        if (width > minWidth)
            minWidth = width;
    }

    return minWidth;
#else
    HTMLObject *obj = head;
    int minWidth = 0;
    int ow, runWidth = 0;

    while ( obj )
    {
	if ( obj->isSeparator() || obj->isNewline() )
	{
	    ow = obj->calcMinWidth();

	    runWidth += ow;

	    if ( runWidth > minWidth )
                minWidth = runWidth;

	    runWidth = 0;
	}
	else
	{
	    ow = obj->calcMinWidth();

	    // we try not to grow larger than max_width by breaking at
	    // object boundaries if necessary.
	    if ( runWidth + ow > max_width )
		runWidth = 0;

	    runWidth += ow;

	    if ( runWidth > minWidth )
		minWidth = runWidth;
	}

	obj = obj->next();
    }

    if ( isFixedWidth() )
    {
    	if (width > minWidth)
            minWidth = width;
    }

    return minWidth + indent;
#endif
}

int HTMLClueFlow::calcPreferredWidth()
{
    HTMLObject *obj;
    int maxw = 0, w = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( !obj->isNewline() )
	{
            w += obj->calcPreferredWidth();
	}
	else
	{
	    if ( w > maxw )
		maxw = w;
	    w = 0;
	}
    }

    if ( w > maxw )
	maxw = w;

    return maxw + indent;
}

void HTMLClueFlow::setMaxWidth( int _max_width )
{
    max_width = _max_width;

    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->setMaxWidth( max_width - indent );
}


//-----------------------------------------------------------------------------


void HTMLClueAligned::setMaxWidth( int _max_width )
{
    max_width = _max_width;

    HTMLObject *obj;

    for ( obj = head; obj != 0; obj = obj->next() )
	obj->setMaxWidth( max_width );
}

// HTMLClueAligned behaves like a HTMLClueV
//
void HTMLClueAligned::calcSize( HTMLClue *parent )
{
    HTMLClue::calcSize( parent );

    HTMLObject *obj;

    width = 0;
    ascent = ALIGN_BORDER;
    descent = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( obj->getWidth() > width )
	     width = obj->getWidth();
	ascent += obj->getHeight();
	obj->setPos( ALIGN_BORDER, ascent - obj->getDescent() );
    }

    ascent += ALIGN_BORDER;
    width += (ALIGN_BORDER*2);
}

//-----------------------------------------------------------------------------

