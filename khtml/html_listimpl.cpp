/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */
#define INDENT 30
#define LISTSEP 5

#include <qpainter.h>
#include <qcolor.h>
#include <qnamespace.h>

#include "html_list.h"
#include "html_listimpl.h"
using namespace DOM;

#include "khtmlfont.h"
#include "khtmlstyle.h"

#include <stdio.h>

QString toRoman( int number, bool upper )
{
    QString roman;
    QChar ldigits[] = { 'i', 'v', 'x', 'l', 'c', 'd', 'm' };
    QChar udigits[] = { 'I', 'V', 'X', 'L', 'C', 'D', 'M' };
    QChar *digits = upper ? udigits : ldigits;
    int i, d = 0;

    do
    {   
	int num = number % 10;

	if ( num % 5 < 4 )
	    for ( i = num % 5; i > 0; i-- )
		roman.insert( 0, digits[ d ] );

	if ( num >= 4 && num <= 8)
	    roman.insert( 0, digits[ d+1 ] );

	if ( num == 9 )
	    roman.insert( 0, digits[ d+2 ] );

	if ( num % 5 == 4 )
	    roman.insert( 0, digits[ d ] );

	number /= 10;
	d += 2;
    }
    while ( number );

    return roman;
}

HTMLUListElementImpl::HTMLUListElementImpl(DocumentImpl *doc) 
    : HTMLBlockElementImpl(doc)
{
    _type = Disc;
}

HTMLUListElementImpl::~HTMLUListElementImpl()
{
}

const DOMString HTMLUListElementImpl::nodeName() const
{
    return "UL";
}

ushort HTMLUListElementImpl::id() const
{
    return ID_UL;
}

void HTMLUListElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
#if 0 // ### should we still support this????
    case ID_TYPE:
	if ( strcasecmp( attr->value(), "disc" ) == 0 )
	    _type = Disc;
	else if ( strcasecmp( attr->value(), "circle" ) == 0 )
	    _type = Circle;
	else if ( strcasecmp( attr->value(), "square" ) == 0 )
	    _type = Square;
	break;
#endif
    default:
	HTMLBlockElementImpl::parseAttribute(attr);
    }
}

void HTMLUListElementImpl::layout(bool deep)
{
    ascent = 0;
    descent = LISTSEP;

    width = availableWidth;

#ifdef DEBUG_LAYOUT
    printf("%s(UList)::layout(%d) width=%d, layouted=%d\n", nodeName().string().ascii(), deep, width, layouted());
#endif

    NodeImpl *child = firstChild();
    while( child != 0 )
    {
	if(child->id() != ID_LI)
	{
	    printf("wrong structured document in HTMLUListElement!\n");
	}
	else 
	{
	    child->setXPos(INDENT);
	    child->setYPos(descent);
	    if(deep)
		child->layout(deep);
	    descent += child->getHeight();
	}
	child = child->nextSibling();
    }
    descent += 5;
    setLayouted();
}

void HTMLUListElementImpl::print(QPainter *p, int _x, int _y, int _w, int _h, 
		       int _tx, int _ty)
{
    _tx += x;
    _ty += y;
    // check if we need to do anything at all...
    if((_ty - ascent > _y + _h) || (_ty + descent < _y)) return;

    printObject(p, _x, _y, _w, _h, _tx, _ty);

    NodeImpl *child;

    child = firstChild();
    while(child != 0) 
    {
	child->print(p, _x, _y, _w, _h, _tx, _ty);
	child = child->nextSibling();
    }
}

void HTMLUListElementImpl::printObject(QPainter *, int, int, 
				       int, int, int, int)
{
#ifdef DEBUG_LAYOUT
    printf("%s(UList)::printObject()\n", nodeName().string().ascii());
#endif
}

NodeImpl *HTMLUListElementImpl::addChild(NodeImpl *newChild)
{
#ifdef DEBUG_LAYOUT
    printf("%s(UList)::addChild( %s )\n", nodeName().string().ascii(), newChild->nodeName().string().ascii());
#endif

    if(_last)
	newChild->setYPos(_last->getYPos() + _last->getDescent());
    else
	newChild->setYPos(LISTSEP);
    newChild->setXPos(INDENT);

    NodeImpl *ret = NodeBaseImpl::addChild(newChild);

    int childWidth = availableWidth - INDENT;
    if(childWidth < 0) childWidth = 0;
    newChild->setAvailableWidth(childWidth);
    static_cast<HTMLLIElementImpl *>(newChild)->setType(_type);

    return ret;
}

void HTMLUListElementImpl::setAvailableWidth(int w)
{
#ifdef DEBUG_LAYOUT
    printf("%s(UList)::setAvailableWidth(%d)\n", nodeName().string().ascii(), w);
#endif

    if(w != -1) availableWidth = w;

    int childWidth;
    if(availableWidth)
	childWidth = availableWidth - INDENT;
    else
	childWidth = 0;

    NodeImpl *child = firstChild();
    while(child != 0) 
    {
	child->setAvailableWidth(childWidth);
	child = child->nextSibling();
    }
}    
// -------------------------------------------------------------------------


HTMLDirectoryElementImpl::HTMLDirectoryElementImpl(DocumentImpl *doc) 
    : HTMLUListElementImpl(doc)
{
}

HTMLDirectoryElementImpl::~HTMLDirectoryElementImpl()
{
}

const DOMString HTMLDirectoryElementImpl::nodeName() const
{
    return "DIR";
}

ushort HTMLDirectoryElementImpl::id() const
{
    return ID_DIR;
}

// -------------------------------------------------------------------------

HTMLMenuElementImpl::HTMLMenuElementImpl(DocumentImpl *doc)
    : HTMLUListElementImpl(doc)
{
}

HTMLMenuElementImpl::~HTMLMenuElementImpl()
{
}

const DOMString HTMLMenuElementImpl::nodeName() const
{
    return "MENU";
}

ushort HTMLMenuElementImpl::id() const
{
    return ID_MENU;
}

// -------------------------------------------------------------------------

HTMLOListElementImpl::HTMLOListElementImpl(DocumentImpl *doc)
    : HTMLUListElementImpl(doc)
{
}

HTMLOListElementImpl::~HTMLOListElementImpl()
{
}

const DOMString HTMLOListElementImpl::nodeName() const
{
    return "OL";
}

ushort HTMLOListElementImpl::id() const
{
    return ID_OL;
}

long HTMLOListElementImpl::start() const
{
    // ###
    return 0;
}

void HTMLOListElementImpl::setStart( long )
{
}

void HTMLOListElementImpl::layout(bool deep)
{
    ascent = 0;
    descent = 5;

    width = availableWidth;

#ifdef DEBUG_LAYOUT
    printf("%s(OList)::layout(%d) width=%d, layouted=%d\n", nodeName().string().ascii(), deep, width, layouted());
#endif

    int num = 1;
    NodeImpl *child = firstChild();
    while( child != 0 )
    {
	if(child->id() != ID_LI)
	{
	    printf("wrong structured document in HTMLUListElement!\n");
	}
	else 
	{
	    HTMLLIElementImpl *i = static_cast<HTMLLIElementImpl *>(child);
	    num = i->calcListValue(num);
	    child->setXPos(INDENT);
	    child->setYPos(descent);
	    if(deep)
		child->layout(deep);
	    descent += child->getDescent();
	}
	child = child->nextSibling();
    }
    descent += 5;
    setLayouted();
}

// -------------------------------------------------------------------------

HTMLLIElementImpl::HTMLLIElementImpl(DocumentImpl *doc)
    : HTMLBlockElementImpl(doc)
{
    predefVal = -1;
    val = 0;
}

HTMLLIElementImpl::~HTMLLIElementImpl()
{
}

const DOMString HTMLLIElementImpl::nodeName() const
{
    return "LI";
}

ushort HTMLLIElementImpl::id() const
{
    return ID_LI;
}

int HTMLLIElementImpl::calcListValue( long v )
{
// ###
#if 0
    DOMString s = attributeMap.valueForId(ATTR_VALUE);
    if(s != 0)
	val = s.toInt();
    else
#endif
	val = v;
    return val+1;
}

void HTMLLIElementImpl::print(QPainter *p, int _x, int _y, int _w, int _h, 
			     int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT
    printf("%s(LI)::print()\n", nodeName().string().ascii());
#endif
    printIcon(p, _tx+x, _ty+y);
    HTMLBlockElementImpl::print(p, _x, _y, _w, _h, _tx, _ty);
}

void HTMLLIElementImpl::printIcon(QPainter *p, int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT
    printf("%s(LI)::printObject(%d, %d)\n", nodeName().string().ascii(), _tx, _ty);
#endif

    // ### this should scale with the font size in the body... possible?
    int yp = _ty + 4;
    int xp = _tx - 10;

    QColor color(pSettings->fontBaseColor);
    p->setPen( QPen( color ) );

   // Just print the item identifier, then do usual BlockElement rendering
    switch(t)
    {
    case Disc:
	p->setBrush( QBrush( color ) );
	p->drawEllipse( xp, yp, 7, 7 );
	break;	
    case Circle:
	p->setBrush( QBrush() );
	p->drawEllipse( xp, yp, 7, 7 );
	break;
    case Square: 
	p->setBrush( QBrush( color ) );
	p->drawRect( xp, yp, 7, 7 );
	break;
    default:
    {
	QString item;
	switch(t)
	{
	case LowRoman:
	    item = toRoman( val, false );
	    item += ".";
	    break;
	case UpRoman:
	    item = toRoman( val, true );
	    item += ".";
	    break;
	case LowAlpha:
	    item = (QChar) ((int)('a' + val - 1));
	    item += ".";
	    break;
	case UpAlpha:
	    item = (QChar) ((int)('A' + val - 1));
	    item += ".";
	    break;
	case Num:
	    item.sprintf( "%2d", val );
	    item += ".";
	    break;
	default:
	    break;
	}
	QFont font = p->font();
	QFont f = font;
	f.setBold(true);
	p->setFont(f);
	p->drawText(_tx-5, _ty, 0, 0, Qt::AlignRight|Qt::DontClip, item);
	p->setFont(font);

    }
    }
}
void HTMLLIElementImpl::printObject(QPainter *p, int _x, int _y,
				    int _w, int _h, int _tx, int _ty)
{
    printIcon(p, _tx, _ty);
    HTMLBlockElementImpl::printObject(p, _x, _y, _w, _h, _tx, _ty);
}

// -------------------------------------------------------------------------


HTMLDListElementImpl::HTMLDListElementImpl(DocumentImpl *doc)
    : HTMLBlockElementImpl(doc)
{
}

HTMLDListElementImpl::~HTMLDListElementImpl()
{
}

const DOMString HTMLDListElementImpl::nodeName() const
{
    return "DL";
}

ushort HTMLDListElementImpl::id() const
{
    return ID_DL;
}

void HTMLDListElementImpl::layout(bool deep)
{
    ascent = 0;
    descent = 5;

    width = availableWidth;

#ifdef DEBUG_LAYOUT
    printf("%s(DList)::layout(%d) width=%d, layouted=%d\n", nodeName().string().ascii(), deep, width, layouted());
#endif

    NodeImpl *child = firstChild();
    while( child != 0 )
    {
	if(child->id() == ID_DT)
	{
	    child = calcParagraph(child);
	}
	else if(child->id() == ID_DD)
	{
	    child->setXPos(INDENT);
	    child->setYPos(descent);
	    if(deep)
		child->layout(deep);
	    descent += child->getHeight();
	    child = child->nextSibling();
	}
	else 
	{
	    printf("wrong structured document in HTMLUListElement!\n");
	    child = child->nextSibling();
	}
    }
    descent += 5;
    setLayouted();
}



