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

#include "dom_string.h"

#include "html_baseimpl.h"
#include "html_documentimpl.h"
using namespace DOM;

#include "khtml.h"
#include "kbrowser.h"
#include "khtmlattrs.h"
#include <stdio.h>
#include <kurl.h>

HTMLBodyElementImpl::HTMLBodyElementImpl(DocumentImpl *doc, KHTMLWidget *v)
    : HTMLBlockElementImpl(doc)
{
    ascent = descent = 0;
    view = v;
}

HTMLBodyElementImpl::~HTMLBodyElementImpl()
{
}

const DOMString HTMLBodyElementImpl::nodeName() const
{
    return "BODY";
}

ushort HTMLBodyElementImpl::id() const
{
    return ID_BODY;
}

void HTMLBodyElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_BGCOLOR:
    {
	QColor bg;
	setNamedColor( bg, attr->value().string() );
	if(view) view->setBGColor( bg );
	break;
    }
    case ATTR_BACKGROUND:
	// ### add the other attrs...
    default:
	HTMLBlockElementImpl::parseAttribute(attr);
    }
}


#if 0
void HTMLBodyElementImpl::layout(bool deep)
{
    width = availableWidth;
#ifdef DEBUG_LAYOUT
    printf("%s(BodyElement)::layout(%d) width=%d, layouted=%d\n", nodeName().string().ascii(), deep, width, layouted());
#endif

    if(!width) return;

    // Block elements usually just have descent.
    // ascent != 0 will give a separation.
    ascent = 0;
    descent = 0;

    NodeImpl *child = firstChild();
    while( child != 0 )
    {
#if 0 //###
	if(child->isAligned())
	{
	}
	else
#endif
	if(child->isInline())
	{
	    child = calcParagraph(child);
	}
	else
	{
	    child->setYPos(descent);
	    if(deep)
		child->layout(deep);
	    descent += child->getDescent();
	    child = child->nextSibling();
	}
    }

    setLayouted();
}

#endif

// -------------------------------------------------------------------------

HTMLFrameElementImpl::HTMLFrameElementImpl(DocumentImpl *doc)
    : HTMLPositionedElementImpl(doc)
{
    view = 0;
    parentWidget = 0;
}

HTMLFrameElementImpl::~HTMLFrameElementImpl()
{
}

const DOMString HTMLFrameElementImpl::nodeName() const
{
    return "FRAME";
}

ushort HTMLFrameElementImpl::id() const
{
    return ID_FRAME;
}

bool HTMLFrameElementImpl::noResize() const
{
    // ###
    return false;
}

void HTMLFrameElementImpl::setNoResize( const bool & )
{
}

void HTMLFrameElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
	// ### remove opening the view and url from here!
    case ATTR_SRC:
    {
	url = attr->value();
	break;
    }
    case ATTR_NAME:
    {
	name = attr->value();
	break;
    }
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLFrameElementImpl::layout(bool)
{
#ifdef DEBUG_LAYOUT
    printf("%s(Frame)::layout(???) width=%d, layouted=%d\n", nodeName().string().ascii(), width, layouted());
#endif

    if(!parentWidget || !view) return;

    int x,y;
    getAbsolutePosition(x, y);
    parentWidget->addChild(view, x, y);
    view->resize(width, descent);

    setLayouted();
}

void HTMLFrameElementImpl::attach(KHTMLWidget *w)
{
    printf("Frame::attach\n");

    parentWidget = w;
    if(w)
    {	
	view = w->newView(w->viewport(), name.string().ascii(), 0);
	view->setIsFrame(true);
    }
    if(url != 0)
    {
	KURL u(w->getDocumentURL(), url.string());
	static_cast<KBrowser *>(view)->openURL(u.url());
    }

    if(!parentWidget || !view) return;

    int x,y;
    getAbsolutePosition(x, y);
#ifdef DEBUG_LAYOUT
    printf("adding frame at %d/%d\n", x, y);
    printf("frame size %d/%d\n", width, descent);
#endif
    w->addChild(view, x, y);
    view->resize(width, descent);
    view->show();
    printf("adding frame\n");

}

void HTMLFrameElementImpl::detach()
{
    parentWidget->removeChild(view);
    delete view;
    parentWidget = 0;
}

// -------------------------------------------------------------------------

HTMLFrameSetElementImpl::HTMLFrameSetElementImpl(DocumentImpl *doc)
    : HTMLPositionedElementImpl(doc)
{
    rowHeight = 0;
    colWidth = 0;

    // default value for rows and cols...
    rows = 0;
    cols = 0;

    setBlocking();
}

HTMLFrameSetElementImpl::~HTMLFrameSetElementImpl()
{
    if(rowHeight) delete rowHeight;
    if(colWidth) delete colWidth;
}

const DOMString HTMLFrameSetElementImpl::nodeName() const
{
    return "FRAMESET";
}

ushort HTMLFrameSetElementImpl::id() const
{
    return ID_FRAMESET;
}

void HTMLFrameSetElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_ROWS:
	rows = attr->val()->toLengthList();
	break;
    case ATTR_COLS:
	cols = attr->val()->toLengthList();
	break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLFrameSetElementImpl::layout(bool deep)
{
#ifdef DEBUG_LAYOUT
    printf("%s(FrameSet)::layout(%d) width=%d, layouted=%d\n", nodeName().string().ascii(), deep, width, layouted());
#endif

    int r;
    int c;
    NodeImpl *child = _first;
    int yPos = 0;
    int totalRows = 1;
    if(rows)
	totalRows = rows->count();
    int totalCols = 1;
    if(cols)
	totalCols = cols->count();

    for(r = 0; r < totalRows; r++)
    {
	int xPos = 0;
	for(c = 0; c < totalCols; c++)
	{
	    HTMLElementImpl *e = static_cast<HTMLElementImpl *>(child);
	    e->setXPos(xPos);
	    e->setYPos(yPos);
	    e->setWidth(colWidth[c]);
	    e->setDescent(rowHeight[r]);
	    if(deep)
		e->layout();
	    xPos += colWidth[c];
	    child = child->nextSibling();
	    if(!child) goto end;
	}
	yPos += rowHeight[r];
    }

 end:
    setLayouted();
}

void HTMLFrameSetElementImpl::attach(KHTMLWidget *)
{
    width = availableWidth;

    int remainingWidth = width;
    int remainingHeight = descent;
    int totalRows = 1;
    if(rows)
	totalRows = rows->count();
    int totalCols = 1;
    if(cols)
	totalCols = cols->count();
    ascent = 0;

    if(rowHeight) delete [] rowHeight;
    if(colWidth) delete [] colWidth;
    rowHeight = new int[totalRows];
    colWidth = new int[totalCols];

    int i;
    int totalRelative = 0;
    int colsRelative = 0;
    int rowsRelative = 0;
    int remainingRelativeWidth = 0;
    // fixed rows first, then percent and then relative

    if(rows)
    {
	for(i = 0; i< totalRows; i++)
	{
	    printf("setting row %d\n", i);
	    if(rows->at(i)->type == Fixed || rows->at(i)->type == Percent)
	    {
		rowHeight[i] = rows->at(i)->width(descent);
		printf("setting row height to %d\n", rowHeight[i]);
		remainingHeight -= rowHeight[i];
	    }
	    else if(rows->at(i)->type == Relative)
	    {
		totalRelative += rows->at(i)->value;
		rowsRelative++;
	    }
	}
	// ###
	if(remainingHeight < 0) remainingHeight = 0;

	if ( !totalRelative && rowsRelative )
	  remainingRelativeWidth = remainingWidth/rowsRelative;
	
	for(i = 0; i< totalRows; i++)
	 {
	    if(rows->at(i)->type == Relative)
	    {
		if ( totalRelative )
		  rowHeight[i] = rows->at(i)->value*remainingHeight/totalRelative;
		else
		  rowHeight[i] = remainingRelativeWidth;
	  	remainingHeight -= rowHeight[i];
	    }
	}
    }
    else
	rowHeight[0] = descent;

    if(cols)
    {
	totalRelative = 0;
	remainingRelativeWidth = 0;

	for(i = 0; i< totalCols; i++)
	{
	    if(cols->at(i)->type == Fixed || cols->at(i)->type == Percent)
	    {
		colWidth[i] = cols->at(i)->width(width);
		remainingWidth -= colWidth[i];
	    }
	    else if(cols->at(i)->type == Relative)
	    {
		totalRelative += cols->at(i)->value;
		colsRelative++;
	    }
	}
	// ###
	if(remainingWidth < 0) remainingWidth = 0;
	
	if ( !totalRelative && colsRelative )
	  remainingRelativeWidth = remainingWidth/colsRelative;

	for(i = 0; i< totalCols; i++)
	{
	    if(cols->at(i)->type == Relative)
	    {
		if ( totalRelative )
		  colWidth[i] = cols->at(i)->value*remainingWidth/totalRelative;
		else
		  colWidth[i] = remainingRelativeWidth;
		remainingWidth -= colWidth[i];
	    }
	}
    }
    else
	colWidth[0] = width;

}

void HTMLFrameSetElementImpl::detach()
{
}

NodeImpl *HTMLFrameSetElementImpl::addChild(NodeImpl *child)
{
#ifdef DEBUG_LAYOUT
    printf("%s(FrameSet)::addChild( %s )\n", nodeName().string().ascii(), child->nodeName().string().ascii());
#endif

    // ### set child's size here!!!

    child->setAvailableWidth(0);
    return NodeBaseImpl::addChild(child);
}

void HTMLFrameSetElementImpl::close()
{
    setParsing(false);
    calcMinMaxWidth();
    if(!availableWidth) return;
    if(availableWidth < minWidth)
	_parent->updateSize();
    setAvailableWidth(); // update child width
    layout(true);
    if(layouted())
	static_cast<HTMLDocumentImpl *>(document)->print(this, true);
}
// -------------------------------------------------------------------------

HTMLHeadElementImpl::HTMLHeadElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
    setBlocking();
}

HTMLHeadElementImpl::~HTMLHeadElementImpl()
{
}

const DOMString HTMLHeadElementImpl::nodeName() const
{
    return "HEAD";
}

ushort HTMLHeadElementImpl::id() const
{
    return ID_HEAD;
}

// -------------------------------------------------------------------------

HTMLHtmlElementImpl::HTMLHtmlElementImpl(DocumentImpl *doc)
    : HTMLPositionedElementImpl(doc)
{
}

HTMLHtmlElementImpl::~HTMLHtmlElementImpl()
{
}

const DOMString HTMLHtmlElementImpl::nodeName() const
{
    return "HTML";
}

ushort HTMLHtmlElementImpl::id() const
{
    return ID_HTML;
}

NodeImpl *HTMLHtmlElementImpl::addChild(NodeImpl *child)
{
#ifdef DEBUG_LAYOUT
    printf("%s(HTML)::addChild( %s )\n", nodeName().string().ascii(), child->nodeName().string().ascii());
#endif

    if(child->id() == ID_FRAMESET)
    {
	child->setAvailableWidth(availableWidth);
	KHTMLWidget *w = (KHTMLWidget *)static_cast<HTMLDocumentImpl *>(document)
	    ->HTMLWidget();
	if(w)
	    child->setDescent(w->height());
    }
    else
    {
	child->setAvailableWidth(availableWidth - 2*BORDER);
	child->setXPos(BORDER);
	child->setYPos(BORDER);
    }
    return NodeBaseImpl::addChild(child);
}

void HTMLHtmlElementImpl::getAbsolutePosition(int &xPos, int &yPos)
{
#if 0
    NodeImpl *child = _first;
    while(child && child->id() != ID_BODY && child->id() != ID_FRAMESET)
	child = child->nextSibling();
    if(child && child->id() == ID_BODY)
	xPos = BORDER, yPos = BORDER;
    else
#endif
	xPos = 0, yPos = 0;
}


void HTMLHtmlElementImpl::layout(bool deep)
{
    width = availableWidth;
#ifdef DEBUG_LAYOUT
    printf("%s(BodyElement)::layout(%d) width=%d, layouted=%d\n", nodeName().string().ascii(), deep, width, layouted());
#endif

    if(!width) return;

    NodeImpl *child = _first;
    while(child && child->id() != ID_BODY && child->id() != ID_FRAMESET)
	child = child->nextSibling();
    if(child && child->id() == ID_BODY)
    {
	child->setXPos(BORDER);
	child->setYPos(BORDER);
    }
    child->layout(deep);

    ascent = 0;
    descent = child->getHeight();

    setLayouted();
}


