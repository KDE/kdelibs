/* 
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
              (C) 1998 Waldo Bastian (bastian@kde.org)

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
// KDE HTML Widget -- Clues
// $Id$

#include <kurl.h>

#include "khtmlchain.h"
#include "khtmliter.h"
#include "khtmlobj.h"
#include "khtmlclue.h"
#include "khtml.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <qimage.h>
#include <qdrawutil.h>

//#define CLUE_DEBUG
//#define DEBUG_ALIGN
//#define DEBUG_LAYOUT


// -- Do not edit --
#ifdef DEBUG_LAYOUT
#define DEBUGL(x)   x
#else
#define DEBUGL(x)
#endif
// --- Thank you ---


// The number of pixels (horizontally) between an aligned object and the text 
// flow
static const int ALIGN_MARGIN=5;

//-----------------------------------------------------------------------------

HTMLClue::HTMLClue()
	 : HTMLObject()
{
    x = y = 0;
    fixed_width = UNDEFINED;
	
    valign = Bottom;
    halign = Left;
    head = tail = curr = 0;
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

HTMLAnchor* HTMLClue::findAnchor( QString _name, int &_x, int &_y)
{
    HTMLObject *obj;
    HTMLAnchor *ret;

    _x += x;
    _y += y - ascent;
    
    for ( obj = head; obj != 0; obj = obj->next() )
    {
	ret = obj->findAnchor( _name, _x, _y );
	if ( ret != 0 )
	    return ret;
    }

    _x -= x;
    _y -= y - ascent;    

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

void HTMLClue::getSelected( QStringList &_list )
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
    QString _url, bool _select, int _tx, int _ty )
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

HTMLIterator * HTMLClue::getIterator()
{ 
    return head ? new HTMLListIterator( this ) : 0; 
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

    if ( (_x < x) || (_x > x + width) || 
         (_y > y + descent) || (_y < y - ascent)
       )
    {
	return 0L;
    }

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

    if ( (_x < x) || (_x > x + width) || 
         (_y > y + descent) || (_y < y - ascent)
       )
    {
	return 0;
    }

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
    // This is a empty virtual function
}

void HTMLClue::setFixedWidth( int _width)
{
    fixed_width = _width;
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
    min_width = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	int w = obj->calcMinWidth();
	if ( w > min_width )
	    min_width = w;
    }

    if ( fixed_width != UNDEFINED)
    {
        if (fixed_width > min_width)
            min_width = fixed_width;
    }

    return min_width;
}

int HTMLClue::calcPreferredWidth()
{
    if ( fixed_width != UNDEFINED )
	return fixed_width;

    HTMLObject *obj;
    int prefWidth = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	int w = obj->calcPreferredWidth();
	if ( w > prefWidth )
	    prefWidth = w;
    }
    if (prefWidth < min_width)
        prefWidth = min_width;	

    return prefWidth;
}

void HTMLClue::setMaxWidth( int _max_width)
{
    if (fixed_width == UNDEFINED)
    {
        // Variable width
        width = _max_width;
        if (_max_width < min_width)
        {
            printf("Layout error in HTMLClue::setMaxWidth(this = %p): _max_width (%d) smaller than minimum width(%d)!\n", 
            this, _max_width, min_width);
        }
    }
    else 
    {
    	// Fixed with
        width = fixed_width;
        if (_max_width < width)
        {
            printf("Layout error in HTMLClue::setMaxWidth(this = %p): _max_width (%d) smaller than fixed width (%d)!\n", 
            this, _max_width, fixed_width);
        }
    }	

    if (width < min_width)
	width = min_width;
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
	if ( !obj->isHAligned() )
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
    if ( (_y + _height < y - ascent) || (_y > y + descent) )
	return false;
    
    HTMLObject *obj;

    _tx += x;
    _ty += y - ascent;

#ifdef CLUE_DEBUG
    // draw rectangles around clues - for debugging
    QString str = objectName();
    QPen pen;
    pen.setWidth(2);
    if(str == "HTMLClueFlow")
    {
	pen.setColor( QColor( "black" ));
	pen.setWidth(1);
    }
    else if(str == "HTMLClueH")
	pen.setColor( QColor( "green" ));
    else if(str == "HTMLClueV")
    {
	pen.setColor( QColor( "blue" ));
	pen.setWidth(3);
    }
    else if(str == "HTMLClueAligned")
	pen.setColor( QColor( "yellow" ));
    else
	pen.setColor( QColor( "cyan" ));
    _painter->setPen(pen);
    _painter->drawRect( _tx, _ty, width, getHeight() );
    QBrush brush( QColor("white"));
    _painter->setBrush( brush );
#endif

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	if ( !obj->isHAligned() )
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
	_chain->current()->print( _painter, _chain, _x - x,
		_y - (y - ascent), _width, _height, _tx, _ty );
    }
}

void HTMLClue::print( QPainter *_painter, int _tx, int _ty )
{
    print( _painter, 0, 0, 0xFFFF, 0xFFFF, _tx, _ty, false );
}

void HTMLClue::print( QPainter *_painter, HTMLObject *_obj, int _x, int _y, int _width, int _height, int _tx, int _ty )
{
    if ( (_y + _height < y - ascent) || (_y > y + descent) )
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
HTMLClue::printDebug( bool propagate, int indent, bool printObjects )
{
    QString str = "   ";
    QString iStr = "";
    int i;
    for( i=0; i<indent; i++)
	iStr += str;

    printf((iStr + objectName()).ascii());
    printf("\n");

    if(!propagate) return;

    printf((iStr + "\\---> pos = (%d/%d)  size = (%d/%d)\n").ascii(),
	   x, y, width, ascent+descent);
    //printf((iStr + " ---> leftMargin = %d\n").ascii(), getLeftMargin( getYPos() ));
    printf((iStr + " ---> this = %p\n").ascii(), this);
    
    // ok... go through the children
    HTMLObject *obj;
    indent++;
    for ( obj = head; obj != 0; obj = obj->next() )
	obj->printDebug( propagate, indent, printObjects);

}


//-----------------------------------------------------------------------------

HTMLClueV::HTMLClueV()
    : HTMLClue()
{
    alignLeftList  = 0;
    alignRightList = 0;
}

void HTMLClueV::reset()
{
    HTMLClue::reset();

    alignLeftList  = 0;
    alignRightList = 0;
}

HTMLObject* HTMLClueV::checkPoint( int _x, int _y )
{
    HTMLObject *obj2;

    if ( ( obj2 = HTMLClue::checkPoint( _x, _y ) ) != 0L )
	    return obj2;

    if ( (_x < x) || (_x > x + width) || 
         (_y > y + descent) || (_y < y - ascent)
       )
    {
	return 0L;
    }

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

    if ( (_x < x) || (_x > x + width) || 
         (_y > y + descent) || (_y < y - ascent) 
       )
    {
	return 0;
    }

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
DEBUGL(printf("Start HTMLClueV::CalcSize( this = %p )\n", this));
    int lmargin = parent ? parent->getLeftMargin( getYPos() ) : 0;

#if 0
    // If we have already called calcSize for the children, then just
    // continue from the last object done in previous call.
    if ( curr )
    {
        ascent = 0;
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
#endif
    {
        ascent = descent = 0;
        curr = head;
    }

    while ( curr != 0 )
    {
        // Set an initial ypos so that the alignment stuff knows where
        // the top of this object is
        curr->setMaxWidth( width );
        curr->setYPos( ascent );
        curr->calcSize( this );
        if ( curr->getWidth() > width )
        {
            printf("Layout error in HTMLClueV::calcSize(this = %p): HTMLClueV not wide enough (width = %d)!\n", 
            	this, width);
 	}
        ascent += curr->getHeight();
        curr->setPos( lmargin, ascent - curr->getDescent() );
        curr = curr->next();
    }

    // remember the last object so that we can start from here next time
    // we are called.
    curr = tail;

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
DEBUGL(printf("End HTMLClueV::CalcSize( this = %p )\n", this));
}

void HTMLClueV::print( QPainter *_painter, int _tx, int _ty )
{
    print( _painter, 0, 0, 0xFFFF, 0xFFFF, _tx, _ty, false );
}


bool HTMLClueV::print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter )
{
    bool rv = HTMLClue::print( _painter, _x, _y, _width, _height, _tx, _ty, toPrinter );

    // print aligned objects
    if ( (_y + _height < y - ascent) || (_y > y + descent) )
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
    printf("Find w,h = %d,%d from Y-pos %d on\n", _width, _height, _y);
    printf("aligLeftList:\n");

    for ( clue = alignLeftList; clue != 0; clue = clue->nextClue() )
    {
        base_y = clue->getYPos() + clue->parent()->getYPos() -
         		 clue->parent()->getAscent();
	top_y = base_y - clue->getAscent();
        printf("    x,y [x,y] / w,h = %d,%d [%d,%d] / %d, %d\n",
        	clue->getXPos()+clue->parent()->getXPos(), 
        	top_y,
        	clue->getXPos(), clue->getYPos(),
        	clue->getWidth(), clue->getHeight());
    }
    printf("aligRightList:\n");

    for ( clue = alignRightList; clue != 0; clue = clue->nextClue() )
    {
        base_y = clue->getYPos() + clue->parent()->getYPos() -
         		 clue->parent()->getAscent();
	top_y = base_y - clue->getAscent();
        printf("    x,y [x,y] / w,h = %d,%d [%d,%d] / %d, %d\n",
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
	rmargin = width;
        next_y = 0;
    
	// Calculate left margin
        for ( clue = alignLeftList; clue != 0; clue = clue->nextClue() )
        {
           base_y = clue->getYPos() + clue->parent()->getYPos() -
         		 clue->parent()->getAscent();
	   top_y = base_y - clue->getAscent();

	   if ((top_y <= try_y+_height) && (base_y > try_y))
	   {
	      int lm = clue->getXPos() + clue->getWidth()+ALIGN_MARGIN;
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
	      int rm = clue->getXPos()-ALIGN_MARGIN;
	      if (rm < rmargin)
	         rmargin = rm;

	      if ((next_y == 0) || (base_y < next_y))
	      {
	          next_y = base_y;
              }
	   }
        }
        
        /* no margins here.. just put it in */
        if ((lmargin == _indent) && (rmargin == width))
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
    printf("Got y=%d, lmargin = %d, rmargin =%d\n", try_y, lmargin, rmargin);
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
	    	printf("%s:%d Clue already in alignLeftList\n", __FILE__, __LINE__);
		return;
	    }
	    obj = obj->nextClue();
	}
	if ( obj == _clue )
	{
	    printf("%s:%d Clue already in alignLeftList\n", __FILE__, __LINE__);
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
		printf("%s:%d Clue already in alignRightList\n", __FILE__, __LINE__);
		return;
	    }
	    obj = obj->nextClue();
	}
	if ( obj == _clue )
	{
	    printf("%s:%d Clue already in alignRightList\n", __FILE__, __LINE__);
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
	    margin = clue->getXPos() + clue->getWidth() + ALIGN_MARGIN;
    }

    return margin;
}

int HTMLClueV::getRightMargin( int _y )
{
    int margin = width;
    HTMLClueAligned *clue;

    for ( clue = alignRightList; clue != 0; clue = clue->nextClue() )
    {
	if ( clue->getYPos()-clue->getAscent()+clue->parent()->getYPos() -
		clue->parent()->getAscent() <= _y &&
		clue->getYPos() + clue->parent()->getYPos() -
		clue->parent()->getAscent() > _y )
	    margin = clue->getXPos() - ALIGN_MARGIN;
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

void 
HTMLClueV::printDebug( bool propagate, int indent, bool printObjects )
{
    QString str = "   ";
    QString iStr = "";
    int i;
    for( i=0; i<indent; i++)
	iStr += str;

    printf((iStr + objectName()).ascii());
    printf("\n");

    if(!propagate) return;

    printf((iStr + "\\---> pos = (%d/%d)  size = (%d/%d)\n").ascii(),
	   x, y, width, ascent+descent);
    if(alignLeftList)
	printf((iStr + " ---> alignLeftList != 0\n").ascii());
    //printf((iStr + " ---> leftMargin = %d\n").ascii(), getLeftMargin( getYPos() ));
    printf((iStr + " ---> this = %p\n").ascii(), this);

    // ok... go through the children
    HTMLObject *obj;
    indent++;
    for ( obj = head; obj != 0; obj = obj->next() )
      obj->printDebug( propagate, indent, printObjects);
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

void HTMLClueH::calcSize( HTMLClue *parent )
{
    HTMLObject *obj;
    int x_pos = 0;
    int remainingWidth;
    
DEBUGL(printf("Start HTMLClueH::CalcSize( this = %p )\n", this));

    // FIXME need absloute ypos here...
    // The minimum width of children is not in all circumstance guaranteed
    // this is not important _YET_ because the current code seems to work
    // for the situations in which we use it.
    // @@@WABA: Space management needs to be used more extensively!
   
    if ( parent )
	x_pos = parent->getLeftMargin( getYPos() );

    x_pos += indent;
    
    remainingWidth = width - x_pos;
    descent = 0;
    ascent = 0;

    int a = 0;
    int d = 0;
    for ( obj = head; obj != 0; obj = obj->next() )
    {
    	int w;
        obj->setMaxWidth( remainingWidth );
	obj->calcSize( this );
    	obj->fitLine( (obj == head), true, -1);
	obj->setXPos( x_pos );
	w = obj->getWidth();
	x_pos += w;
	remainingWidth -= w;
	if (remainingWidth < 0)
	{
	     remainingWidth = 1;
	     printf("Layout error: HTMLClueH::CalcSize(this = %p) not wide enough (width = %d, x_pos = %d)!\n",
	     	this, width, x_pos);
             // This might be because of lmargin: in that case
             // a larger free area should be searched!
	}
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
DEBUGL(printf("End HTMLClueH::CalcSize( this = %p )\n", this));
}

int HTMLClueH::calcMinWidth()
{
    HTMLObject *obj;
    min_width = indent;

    for ( obj = head; obj != 0; obj = obj->next() )
	min_width += obj->calcMinWidth();

    if ( fixed_width != UNDEFINED)
    {
        if (fixed_width > min_width)
            min_width = fixed_width;
    }

    return min_width;
}
 
int HTMLClueH::calcPreferredWidth()
{
    HTMLObject *obj;
    int prefWidth = indent;

    for ( obj = head; obj != 0; obj = obj->next() )
	prefWidth += obj->calcPreferredWidth();
	
    if (min_width > prefWidth)
        prefWidth = min_width;

    return prefWidth;
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

    if ( tail && tail->isSelected() )
	_str += '\n';
}

// MRJ - 26/10/97:  I've given this a rewrite so that its layout is done
// correctly, i.e. break ONLY on separators and newlines instead of breaking
// after any object.  This is much nicer and smaller code now also.  Sorry
// if I broke something.
// 
void HTMLClueFlow::calcSize( HTMLClue *parent )
{
DEBUGL(printf("Start HTMLClueFlow::CalcSize( this = %p )\n", this));

    HTMLObject *obj = head;
    HTMLObject *line = head;
    HTMLVSpace::Clear clear = HTMLVSpace::CNone;;
    int lmargin, rmargin;
    int remainingWidth = width - indent;

    ascent = 0;
    descent = 0;
    
    lmargin = parent->getLeftMargin( getYPos() );
    if ( indent > lmargin )
	lmargin = indent;
    rmargin = parent->getRightMargin( getYPos() );
    int w = lmargin;
    int a = 0;
    int d = 0;

    bool newLine = false;
    bool doVAlignment = false;
    
    while ( obj != 0 )
    {
	// If we get a newline object, set newLine=true so that the current
	// line will be aligned, and the next line prepared.
	if ( obj->isNewline() )
	{
	    // The following two function calls are skipped because
	    // 'obj' is a fixed width object anyway. 
	    // obj->setMaxWidth( remainingWidth );
            // obj->calcSize( this);
	    obj->setXPos( w );
	    if ( (a == 0) && (d == 0))
	    {
		a = obj->getAscent();
		d = obj->getDescent();
	    }
	    newLine = true;
	    HTMLVSpace *vs = (HTMLVSpace *)obj;
	    clear = vs->clear();
	    obj = obj->next();
	}
	// add a separator
	else if ( obj->isSeparator() )
	{
	    // The following two function calls are skipped because
	    // 'obj' is a fixed width object anyway. 
	    // obj->setMaxWidth( remainingWidth );
            // obj->calcSize( this);
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
	else if ( obj->isHAligned() )
	{
	    HTMLClueAligned *c = (HTMLClueAligned *)obj;

	    if ( !parent->appended( c ) )
	    {
                obj->setMaxWidth( remainingWidth );
	        obj->calcSize( this );

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
		    !run->isHAligned() )
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
		run->calcSize( this );
		runWidth += run->getWidth();

		// If this run cannot fit in the allowed area, break it
		// on a non-seperator. 
		// Don't break here, if it is the first object.
		if (( run != obj) && (runWidth > rmargin - lmargin ))
		{
		     break;
		}    

                if (run->isVAligned() == Bottom)
                {
	 	    if ( run->getAscent() > a )
		        a = run->getAscent();
		    if ( run->getDescent() > d )
		        d = run->getDescent();
		}
		else
		{
		    doVAlignment = true;
		}

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
		    doVAlignment = false;
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

	    if ( halign == HCenter )
	    {
	        // Centered
		extra = ( rmargin - w ) / 2;
		if ( extra < 0 )
		    extra = 0;
	    }
	    else if ( halign == Right )
	    {
	    	// Right aligned
	        extra = rmargin - w;
		if ( extra < 0 )
		    extra = 0;
	    }

	    ascent += a + d;
	    y += a + d;

            if (doVAlignment)
            {
              // Vertical alignment affects our ascent and descent in
              // mysterious ways:
              int old_a, old_d;
              int new_descent, new_ascent;
              do {
                old_a = a;
                old_d = d;
                // This line contains objects with align=top or align=center
	        for( HTMLObject *obj2 = line; 
	             obj2 != obj; 
	             obj2 = obj2->next())
		{
		    if (obj2->isHAligned())
		        continue;
		        
		    switch( obj2->isVAligned() )
		    {
		    case Top:
		         new_descent = obj2->getAscent()+
		                           obj2->getDescent()-a;
		         obj2->setYPos( ascent - d - obj2->getDescent()+
		                        new_descent);
		         if (new_descent > d)
		             d = new_descent;
		         break;
		    
		    case VCenter:
		         new_descent = (obj2->getAscent()+obj2->getDescent())/2;
		         new_ascent = new_descent;
		         obj2->setYPos( ascent - d - obj2->getDescent()+
		                        new_descent);
		         if (new_descent > d)
		             d = new_descent;
		         if (new_ascent > a)
		             a = new_ascent;
		         break;

                    default:
                         // Bottom
                         break;
                    }
		}
		ascent = ascent - old_a - old_d + a + d;
		y = y - old_a - old_d + a + d;
	      }
              while( (old_a != a) || (old_d != d));
              // Keep aligning until both our ascent and descent remains
              // unchanged
	    }
	    
	    while ( line != obj )
	    {
	        if ( !line->isHAligned() )
	        {
	            if (line->isVAligned() == VNone)
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

		parent->findFreeArea(oldy, width,
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
	    doVAlignment = false;
	    clear = HTMLVSpace::CNone;
	}
    }
DEBUGL(printf("End HTMLClueFlow::CalcSize( this = %p )\n", this));
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
	    if ( !obj->isHAligned() )
	    {
		pos = obj->findPageBreak( _y - ( y - ascent ) );
		if ( pos >= 0 && pos < minpos )
		    minpos = pos;
	    }
	    obj = obj->next();
	}
	if ( minpos != yp )
	    return ( minpos + y - ascent );
	if ( !obj )
	    return -1;
    }

    return -1;
}

int HTMLClueFlow::calcMinWidth()
{
    // @@@WABA This needs a bit more work, it works for HTMLtextMaster,
    // but not for multiple non-seperated, non-text objects:
    // For example 3 images.

#if 1
    HTMLObject *obj;
    min_width = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
	int w = obj->calcMinWidth();
	if ( w > min_width )
	    min_width = w;
    }
	
    min_width += indent;	

    if ( fixed_width != UNDEFINED)
    {
        if (fixed_width > min_width)
            min_width = fixed_width;
    }

    return min_width;
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

	    // we try not to grow larger than width by breaking at
	    // object boundaries if necessary.
	    if ( runWidth + ow > width )
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

    maxw += indent;

    if ( fixed_width != UNDEFINED)
    {
        if (fixed_width > maxw)
            maxw = fixed_width;
    }

    return maxw;
}

//-----------------------------------------------------------------------------

// HTMLClueAligned behaves like a HTMLClueV
//
// One difference is that HTMLClueV takes aligned flows into account.
// We don't.
//
// A second difference is that the size of a HTMLClueAligned is always
// equal to its actual contents. 
//
void HTMLClueAligned::calcSize( HTMLClue * )
{
    HTMLObject *obj;
    int new_width = 0;
    ascent = 0;
    descent = 0;

    for ( obj = head; obj != 0; obj = obj->next() )
    {
        // Set an initial ypos so that the alignment stuff knows where
        // the top of this object is
        obj->setMaxWidth( width );

        obj->setYPos( ascent );
        obj->calcSize( this );
        int w = obj->getWidth();
        if ( w > new_width )
        {
            new_width = w; 
 	}
        ascent += obj->getHeight();
        obj->setPos( 0, ascent - obj->getDescent() );
    }
    width = new_width;
}

//-----------------------------------------------------------------------------

