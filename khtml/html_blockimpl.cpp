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
// -------------------------------------------------------------------------
//#define DEBUG

#include <qpainter.h>
#include <qpalette.h>
#include <qdrawutil.h>

#include "dom_string.h"
#include "html_blockimpl.h"
using namespace DOM;

#include "khtmlattrs.h"
#include "khtmlstyle.h"

#include <stdio.h>

#define BLOCKQUOTEINDENT 20

HTMLBlockquoteElementImpl::HTMLBlockquoteElementImpl(DocumentImpl *doc)
    : HTMLBlockElementImpl(doc)
{
    x = BLOCKQUOTEINDENT;
}

HTMLBlockquoteElementImpl::~HTMLBlockquoteElementImpl()
{
}

const DOMString HTMLBlockquoteElementImpl::nodeName() const
{
    return "BLOCKQUOTE";
}

ushort HTMLBlockquoteElementImpl::id() const
{
    return ID_BLOCKQUOTE;
}

void HTMLBlockquoteElementImpl::setAvailableWidth(int w)
{
    if(w != -1) availableWidth = w - 2*BLOCKQUOTEINDENT;
    if(availableWidth < 20) availableWidth = 20;

    setLayouted(false);

    NodeImpl *child = firstChild();
    while(child != 0)
    {
	child->setAvailableWidth(availableWidth);
	child = child->nextSibling();
    }
    width = availableWidth;
}

void HTMLBlockquoteElementImpl::setXPos( int xPos )
{
    x = xPos+BLOCKQUOTEINDENT;
}

// -------------------------------------------------------------------------

HTMLDivElementImpl::HTMLDivElementImpl(DocumentImpl *doc) : HTMLBlockElementImpl(doc)
{
}

HTMLDivElementImpl::~HTMLDivElementImpl()
{
}

const DOMString HTMLDivElementImpl::nodeName() const
{
    return "DIV";
}

ushort HTMLDivElementImpl::id() const
{
    return ID_DIV;
}

void HTMLDivElementImpl::parseAttribute(Attribute *attr)
{
    switch( attr->id )
    {
    case ATTR_ALIGN:
	if ( strcasecmp( attr->value(), "left" ) == 0 )
	    halign = Left;
	else if ( strcasecmp( attr->value(), "right" ) == 0 )
	    halign = Right;
	else if ( strcasecmp( attr->value(), "center" ) == 0 )
	    halign = HCenter;
	break;
    default:
	HTMLBlockElementImpl::parseAttribute(attr);
    }
}
// -------------------------------------------------------------------------

HTMLHRElementImpl::HTMLHRElementImpl(DocumentImpl *doc) : HTMLBlockElementImpl(doc)
{
    ascent = 4; // 3 pixel space on each side + 1 pixel for the line
    descent = 3;
    halign = HCenter;
    shade = TRUE;

}

HTMLHRElementImpl::~HTMLHRElementImpl()
{
}

const DOMString HTMLHRElementImpl::nodeName() const
{
    return "HR";
}

ushort HTMLHRElementImpl::id() const
{
    return ID_HR;
}

bool HTMLHRElementImpl::noShade() const
{
    return shade;
}

void HTMLHRElementImpl::setNoShade( bool s)
{
    shade = s;
    // ### change Attribute too...
}

void HTMLHRElementImpl::parseAttribute(Attribute *attr)
{
    switch( attr->id )
    {
    case ATTR_ALIGN:
	if ( strcasecmp( attr->value(), "left" ) == 0 )
	    halign = Left;
	else if ( strcasecmp( attr->value(), "right" ) == 0 )
	    halign = Right;
	else if ( strcasecmp( attr->value(), "center" ) == 0 )
	    halign = HCenter;
	break;
    case ATTR_SIZE:
	ascent = attr->value().toInt() + 3;
	break;
    case ATTR_WIDTH:
	length = attr->val()->toLength();
	break;
    case ATTR_NOSHADE:
	shade = FALSE;
    default:
	HTMLBlockElementImpl::parseAttribute(attr);
    }
}

void HTMLHRElementImpl::layout(bool)
{
#ifdef DEBUG_LAYOUT
     printf("%s(HR)::layout(???) width=%d, layouted=%d\n", nodeName().string().ascii(), width, layouted());
#endif
   // nothing to do here...
    setLayouted();
}

void HTMLHRElementImpl::print( QPainter *p, int _x, int _y, int _w, int _h,
			       int _tx, int _ty)
{
    _tx += x;
    _ty += y;
    printObject(p, _x, _y, _w, _h, _tx, _ty);
}

void HTMLHRElementImpl::printObject(QPainter *p, int, int _y,
				    int, int _h, int _tx, int _ty)
{
#ifdef DEBUG_LAYOUT
    printf("%s(HR)::printObject()\n", nodeName().string().ascii());
#endif

    if((_ty - ascent > _y + _h) || (_ty + descent < _y)) return;

    QColorGroup colorGrp( Qt::black, Qt::black, QColor(220,220,220), QColor(100,100,100),
    Qt::gray, Qt::black, Qt::black );

    int l = length.width(width);
    int size = ascent - 3;
    int xp = x + _tx;
    if(halign == HCenter)
    {
	xp += (width - l)/2;
    }

    int yp = _ty ;

    int lw = size/2;

    if ( shade )
    {
	if(size < 2) size = 2, lw = 1;
	qDrawShadePanel( p, xp, yp-size, l, size,
		colorGrp, true, lw, 0 );
    }
    else
    {
	if(size < 1) size = 1;
	p->fillRect( xp, yp-size, l, size, Qt::black );
    }
}

// -------------------------------------------------------------------------

HTMLHeadingElementImpl::HTMLHeadingElementImpl(DocumentImpl *doc, ushort _tagid) : HTMLBlockElementImpl(doc)
{
    _id = _tagid;
    halign = Left;
}

HTMLHeadingElementImpl::~HTMLHeadingElementImpl()
{
}

const DOMString HTMLHeadingElementImpl::nodeName() const
{
    return getTagName(_id);
}

ushort HTMLHeadingElementImpl::id() const
{
    return _id;
}

void HTMLHeadingElementImpl::setStyle(CSSStyle *currentStyle)
{
    switch(_id)
    {
    case ID_H1:
	currentStyle->font.weight = CSSStyleFont::Bold;
	currentStyle->font.style = CSSStyleFont::stNormal;
	currentStyle->font.size = pSettings->fontBaseSize+3;
	break;
	
    case ID_H2:
	currentStyle->font.weight = CSSStyleFont::Bold;
	currentStyle->font.style = CSSStyleFont::stNormal;
	currentStyle->font.size = pSettings->fontBaseSize+2;
	break;
	
    case ID_H3:
	currentStyle->font.weight = CSSStyleFont::Normal;
	currentStyle->font.style = CSSStyleFont::stItalic;
	currentStyle->font.size = pSettings->fontBaseSize+1;
	break;
	
    case ID_H4:
	currentStyle->font.weight = CSSStyleFont::Bold;
	currentStyle->font.style = CSSStyleFont::stNormal;
	currentStyle->font.size = pSettings->fontBaseSize;
	break;
	
    case ID_H5:
	currentStyle->font.weight = CSSStyleFont::Normal;
	currentStyle->font.style = CSSStyleFont::stItalic;
	currentStyle->font.size = pSettings->fontBaseSize;
	break;
	
    case ID_H6:
	currentStyle->font.weight = CSSStyleFont::Bold;
	currentStyle->font.style = CSSStyleFont::stNormal;
	currentStyle->font.size = pSettings->fontBaseSize-1;
	break;
    }
    HTMLElementImpl::setStyle(currentStyle);
}

void HTMLHeadingElementImpl::parseAttribute(Attribute *attr)
{
    switch( attr->id )
    {
    case ATTR_ALIGN:
	if ( strcasecmp( attr->value(), "left" ) == 0 )
	    halign = Left;
	else if ( strcasecmp( attr->value(), "right" ) == 0 )
	    halign = Right;
	else if ( strcasecmp( attr->value(), "center" ) == 0 )
	    halign = HCenter;
	break;
    default:
	HTMLBlockElementImpl::parseAttribute(attr);
    }
}

// -------------------------------------------------------------------------

HTMLParagraphElementImpl::HTMLParagraphElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
}

HTMLParagraphElementImpl::~HTMLParagraphElementImpl()
{
}

const DOMString HTMLParagraphElementImpl::nodeName() const
{
    return "P";
}

ushort HTMLParagraphElementImpl::id() const
{
    return ID_P;
}

void HTMLParagraphElementImpl::parseAttribute(Attribute *attr)
{
    //printf("P attr: %s\n",attr->value().string().latin1());
    switch( attr->id )
    {
    case ATTR_ALIGN:
	if ( strcasecmp( attr->value(), "left" ) == 0 )
	    halign = Left;
	else if ( strcasecmp( attr->value(), "right" ) == 0 )
	    halign = Right;
	else if ( strcasecmp( attr->value(), "center" ) == 0 )
	    halign = HCenter;
	break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

// -------------------------------------------------------------------------

HTMLPreElementImpl::HTMLPreElementImpl(DocumentImpl *doc)
    : HTMLBlockElementImpl(doc)
{
}

HTMLPreElementImpl::~HTMLPreElementImpl()
{
}

const DOMString HTMLPreElementImpl::nodeName() const
{
    return "PRE";
}

ushort HTMLPreElementImpl::id() const
{
    return ID_PRE;
}

long HTMLPreElementImpl::width() const
{
    return HTMLBlockElementImpl::width;
}

void HTMLPreElementImpl::setWidth( long w )
{
    HTMLBlockElementImpl::width = w;
}

#if 0
void HTMLPreElementImpl::layout( bool )
{
#ifdef DEBUG_LAYOUT
     printf("%s(Pre)::layout(???) width=%d, layouted=%d\n", nodeName().string().ascii(), HTMLPositionedElementImpl::width, layouted());
#endif

    ascent = 3;
    descent = 0;

    HTMLPositionedElementImpl::width = availableWidth;

    NodeImpl *child = firstChild();
    while( child != 0 )
    {
#ifdef DEBUG
	if(!child->isInline())
	{
	  printf("error: non inline element in <pre>\n");
	  child = child->nextSibling();
	}
	else
#endif
	    child = calcParagraph(child);
    }
    descent += 3;
    setLayouted();
}
#endif

void HTMLPreElementImpl::setStyle(CSSStyle *s)
{
    s->font.family = pSettings->fixedFontFace;
    _style = new CSSStyle(*s);
}
