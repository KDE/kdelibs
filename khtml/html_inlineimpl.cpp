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

#include <qfontinfo.h>
#include <qstack.h>
#include <qregexp.h>

#include <stdio.h>

#include "khtmltext.h"
#include "dom_textimpl.h"
#include "dom_string.h"
#include "html_inline.h"
#include "html_inlineimpl.h"
using namespace DOM;

#include "khtmlattrs.h"
#include "khtmlstyle.h"
#include "khtmltokenizer.h"

HTMLAnchorElementImpl::HTMLAnchorElementImpl(DocumentImpl *doc)
: HTMLAreaElementImpl(doc)
{
}

HTMLAnchorElementImpl::~HTMLAnchorElementImpl()
{
}

const DOMString HTMLAnchorElementImpl::nodeName() const
{
    return "A";
}

ushort HTMLAnchorElementImpl::id() const
{
    return ID_A;
}

long HTMLAnchorElementImpl::tabIndex() const
{
    // ###
    return 0;
}

void HTMLAnchorElementImpl::setTabIndex( long  )
{
}

void HTMLAnchorElementImpl::blur(  )
{
}

void HTMLAnchorElementImpl::focus(  )
{
}

void HTMLAnchorElementImpl::setStyle(CSSStyle *style)
{
#if 0
    if ( visited )
	style->font.color = settings->vLinkColor;
    else
#endif
    if( href )
    {
	style->font.color = pSettings->linkColor;
	if ( pSettings->underlineLinks )
	    style->font.decoration = CSSStyleFont::decUnderline;
    }
    _style = new CSSStyle(*style);

}

bool HTMLAnchorElementImpl::mouseEvent( int _x, int _y, int button, MouseEventType type,
				  int _tx, int _ty, DOMString &_url)
{
    bool inside = false;

    NodeImpl *child = firstChild();
    while(child != 0)
    {
	if(child->mouseEvent(_x, _y, button, type, _tx, _ty, _url))
	{
	    inside = true;
	    break;
	}
	child = child->nextSibling();
    }
    //    printf("Anchor::mouseEvent inside=%d\n", inside);

    if(inside)
    {
	// set the url
	if(_url != 0) printf("Error in Anchor::mouseEvent, nested url's!\n");
	if(target && href)
	{
	    DOMString s = DOMString("target://") + DOMString(target) + DOMString("/#") + DOMString(href);
	    _url = s;
	}
	else
	    _url = href;

	// dynamic HTML...
	mouseEventHandler(button, type);
    }
    return inside;
}

void HTMLAnchorElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_HREF:
    default:
	HTMLAreaElementImpl::parseAttribute(attr);
    }
}

// I don't like this way of implementing the method, but I didn't find any
// other way. Lars
void HTMLAnchorElementImpl::getAnchorPosition(int &xPos, int &yPos)
{
    if(_parent)
    {
	_parent->getAbsolutePosition( xPos, yPos );
	// now we need to get the postion of this element. As it's
	// not positioned, we use the first child which is positioned
	NodeImpl *current = firstChild();
	QStack<NodeImpl> nodeStack;
	bool found = false;
	while(1)
	{
	    if(!current)
	    {
		if(nodeStack.isEmpty()) break;
		current = nodeStack.pop();
	    }
	    else if(current->isRendered())
	    {
		found = true;
		break;
	    }	

	    NodeImpl *child = current->firstChild();
	    if(child)
	    {	
		nodeStack.push(current);
		current = child;
	    }
	    else
	    {
		current = current->nextSibling();
	    }
	}
	if(found)
	{
	    if(current->isTextNode())
	    {
		TextImpl *t = static_cast<TextImpl *>(current);
		if(!t->first) return;
		//printf("text is at: %d/%d\n", t->first->x, t->first->y);
		xPos += t->first->x;
		yPos += t->first->y;
	    }
	    else
	    {
		xPos += current->getXPos();
		yPos += current->getYPos();
	    }
	}
	else
	{
	  // we take the position of the parent.
	  return;
	}
    }
    else
	xPos = yPos = -1;
}
// -------------------------------------------------------------------------

HTMLBRElementImpl::HTMLBRElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
    _clear = BRNone;
}

HTMLBRElementImpl::~HTMLBRElementImpl()
{
}

const DOMString HTMLBRElementImpl::nodeName() const
{
    return "BR";
}

ushort HTMLBRElementImpl::id() const
{
    return ID_BR;
}

void HTMLBRElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_CLEAR:
	if ( strcasecmp( attr->value(), "left" ) == 0 )
	    _clear = BRLeft;
	else if ( strcasecmp( attr->value(), "right" ) == 0 )
	    _clear = BRRight;
	else if ( strcasecmp( attr->value(), "all" ) == 0 )
	    _clear = BRAll;
	break;
    default:
    	HTMLElementImpl::parseAttribute(attr);
    }
}


// -------------------------------------------------------------------------

HTMLFontElementImpl::HTMLFontElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
}

HTMLFontElementImpl::~HTMLFontElementImpl()
{
}

const DOMString HTMLFontElementImpl::nodeName() const
{
    return "FONT";
}

ushort HTMLFontElementImpl::id() const
{
    return ID_FONT;
}

void HTMLFontElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_SIZE:
    case ATTR_COLOR:
    case ATTR_FACE:
	// handled in setStyle...
	break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLFontElementImpl::setStyle(CSSStyle *currentStyle)
{
    DOMString s = attributeMap.valueForId(ATTR_SIZE);
    if(!s.isEmpty())
    {
	int num = s.toInt();
	if ( *s.unicode() == '+' ||
	     *s.unicode() == '-' )
	    currentStyle->font.size += num;
	else
	    currentStyle->font.size = num;
	if(currentStyle->font.size > 7) currentStyle->font.size = 7;
	if(currentStyle->font.size < 1) currentStyle->font.size = 1;
    }
    s = attributeMap.valueForId(ATTR_COLOR);
    if(!s.isEmpty())
    {
	setNamedColor( currentStyle->font.color, s.string() );
    }
    s = attributeMap.valueForId(ATTR_FACE);
    if(!s.isEmpty())
    {
      printf("setting face\n");
	QString str = s.string();
	str.replace(QRegExp("[ ,]+"), ",");
	// try to find a matching font in the font list.
	int index = 0;
	int index2;
	while( 1 )
	{
	    index2 = str.find(",", index);
	    QString fname = str.mid(index, index2-index);
	    fname = fname.lower();
	    QFont tryFont( fname.data() );
	    QFontInfo fi( tryFont );
	    //printf("trying \"%s\": getting %s\n", tryFont.family().ascii(), fi.family().ascii());
	    if ( strcmp( tryFont.family(), fi.family() ) == 0 )
	    {
		// we found a matching font
	      //printf("found mathing face!\n");
		currentStyle->font.family = fname;
		break;
	    }
	    if(index2 == -1) break;
	    index = index2 + 1;
	}	
    }
    HTMLElementImpl::setStyle(currentStyle);

}

void HTMLFontElementImpl::print(QPainter *p, int _x, int _y, int _w, int _h,
				  int _tx, int _ty)
{
  HTMLElementImpl::print(p, _x, _y, _w, _h, _tx, _ty);
}

// -------------------------------------------------------------------------

HTMLIFrameElementImpl::HTMLIFrameElementImpl(DocumentImpl *doc) : HTMLElementImpl(doc)
{
}

HTMLIFrameElementImpl::~HTMLIFrameElementImpl()
{
}

const DOMString HTMLIFrameElementImpl::nodeName() const
{
    return "IFRAME";
}

ushort HTMLIFrameElementImpl::id() const
{
    return ID_IFRAME;
}

// -------------------------------------------------------------------------

HTMLModElementImpl::HTMLModElementImpl(DocumentImpl *doc, ushort tagid) : HTMLElementImpl(doc)
{
    _id = tagid;
}

HTMLModElementImpl::~HTMLModElementImpl()
{
}

const DOMString HTMLModElementImpl::nodeName() const
{
    return getTagName(_id);
}

ushort HTMLModElementImpl::id() const
{
    return _id;
}

// -------------------------------------------------------------------------

HTMLQuoteElementImpl::HTMLQuoteElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
}

HTMLQuoteElementImpl::~HTMLQuoteElementImpl()
{
}

const DOMString HTMLQuoteElementImpl::nodeName() const
{
    return "Q";
}

ushort HTMLQuoteElementImpl::id() const
{
    return ID_Q;
}

