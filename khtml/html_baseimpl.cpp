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
#include "khtmlattrs.h"
#include <stdio.h>
#include <kurl.h>

#include <qcursor.h>
#include <qnamespace.h>

#include <kapp.h>

HTMLBodyElementImpl::HTMLBodyElementImpl(DocumentImpl *doc, KHTMLWidget *v)
    : HTMLBlockElementImpl(doc), HTMLImageRequester()
{
    ascent = descent = 0;
    view = v;
    bgPixmap = 0;
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

    case ATTR_BACKGROUND:
        bgURL = attr->value();
	break;
    case ATTR_BGCOLOR: 	
    case ATTR_TEXT:
    case ATTR_LINK:
    case ATTR_VLINK:
      // handled in setStyle()
    case ATTR_ALINK:
      break;
    default:
	HTMLBlockElementImpl::parseAttribute(attr);
    }
}

void HTMLBodyElementImpl::setStyle(CSSStyle *currentStyle)
{
  DOMString s = attributeMap.valueForId(ATTR_TEXT);
    if(!s.isEmpty())
    {
	setNamedColor( currentStyle->font.color, s.string() );
    }
    s = attributeMap.valueForId(ATTR_LINK);
    if(!s.isEmpty())
    {
	setNamedColor( pSettings->linkColor, s.string() );
    }
    s = attributeMap.valueForId(ATTR_VLINK);
    if(!s.isEmpty())
    {
	setNamedColor( pSettings->vLinkColor, s.string() );
    }
    s = attributeMap.valueForId(ATTR_BGCOLOR);
    if(!s.isEmpty())
    {
	setNamedColor( currentStyle->bgcolor, s.string() );
    }


    HTMLElementImpl::setStyle(currentStyle);

}


void HTMLBodyElementImpl::attach(KHTMLWidget *)
{
    if(bgURL.length())
    {
	printf("Body: Requesting URL=%s\n", bgURL.string().ascii() );
	bgURL = document->requestImage(this, bgURL);
    }	
}

void HTMLBodyElementImpl::detach()
{
    KHTMLCache::free(bgURL, this);
    NodeBaseImpl::detach();
}

void  HTMLBodyElementImpl::setPixmap( QPixmap *p )
{
    printf("setting bg pixmap\n");
    bgPixmap = p;
}

void  HTMLBodyElementImpl::pixmapChanged( QPixmap *p )
{
    bgPixmap = p;
}

void HTMLBodyElementImpl::close()
{
    printf("BODY:close\n");
    calcMinMaxWidth();
    setLayouted(false);
    _parent->updateSize();
}

void HTMLBodyElementImpl::print( QPainter *p, int x, int y, int w, int h,
			int tx, int ty)
{
    if (bgPixmap)
    {
    	int pmX = x % bgPixmap->width();
	int pmY = y % bgPixmap->height();
	p->drawTiledPixmap(x,y,w,h,*bgPixmap,pmX,pmY);
    }
    else
    	p->fillRect(x,y,w,h,style()->bgcolor);
	
    HTMLBlockElementImpl::print(p,x,y,w,h,tx,ty);
}


// -------------------------------------------------------------------------

HTMLFrameElementImpl::HTMLFrameElementImpl(DocumentImpl *doc)
    : HTMLPositionedElementImpl(doc)
{
    view = 0;
    parentWidget = 0;

    frameBorder = true;
    marginWidth = -1;
    marginHeight = -1;
    scrolling = QScrollView::Auto;
    noresize = false;
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

void HTMLFrameElementImpl::parseAttribute(Attribute *attr)
{
    printf("parsing attribute %d=%s\n", attr->id, attr->value().string().ascii());

    switch(attr->id)
    {
    case ATTR_SRC:
	url = attr->value();
	break;
    case ATTR_NAME:
	name = attr->value();
	break;
    case ATTR_FRAMEBORDER:
	if(attr->value() == "0" || strcasecmp( attr->value(), "no" ) == 0 )
	    frameBorder = false;
	break;
    case ATTR_MARGINWIDTH:
	marginWidth = attr->val()->toInt();
	break;
    case ATTR_MARGINHEIGHT:
	marginHeight = attr->val()->toInt();
	break;
    case ATTR_NORESIZE:
	noresize = true;
	break;
    case ATTR_SCROLLING:
	if( strcasecmp( attr->value(), "auto" ) == 0 )
	    scrolling = QScrollView::Auto;
	else if( strcasecmp( attr->value(), "yes" ) == 0 )
	    scrolling = QScrollView::AlwaysOn;
	else if( strcasecmp( attr->value(), "no" ) == 0 )
	    scrolling = QScrollView::AlwaysOff;

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

    // needed for restoring frames
    bool open = true;

    parentWidget = w;
    if(w)
    {	
	// we need a unique name for every frame in the frameset. Hope that's unique enough.
	if(name.isEmpty())
	{
	    QString tmp;
	    tmp.sprintf("0x%p", this);
	    name = DOMString(tmp) + url;
	    printf("creating frame name: %s\n",name.string().ascii());
	}
	view = w->getFrame(name.string());
	if(view)
	    open = false;
	else
	    view = w->createFrame(w->viewport(), name.string());
	view->setIsFrame(true);
    }
    if(url != 0 && open)
    {
	KURL u(w->url(), url.string());
	view->openURL(u.url());
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

    if(!frameBorder || !((static_cast<HTMLFrameSetElementImpl *>(_parent))->frameBorder()))
	view->setFrameStyle(QFrame::NoFrame);
    view->setVScrollBarMode(scrolling);
    view->setHScrollBarMode(scrolling);
    if(marginWidth != -1) view->setMarginWidth(marginWidth);
    if(marginHeight != -1) view->setMarginHeight(marginHeight);

    view->show();
    printf("adding frame\n");

}

void HTMLFrameElementImpl::detach()
{
    delete view;
    parentWidget = 0;
    NodeBaseImpl::detach();
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
    totalRows = 1;
    totalCols = 1;

    frameborder = true;
    border = 4;
    noresize = false;

    resizing = false;
    hSplit = -1;
    vSplit = -1;
    hSplitVar = 0;
    vSplitVar = 0;

    view = 0;

    setBlocking();
}

HTMLFrameSetElementImpl::~HTMLFrameSetElementImpl()
{
    if(rowHeight) delete [] rowHeight;
    if(colWidth) delete [] colWidth;

    if(vSplitVar) delete [] vSplitVar;
    if(hSplitVar) delete [] hSplitVar;
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
	totalRows = rows->count();
	break;
    case ATTR_COLS:
	cols = attr->val()->toLengthList();
	totalCols = cols->count();
	break;
    case ATTR_FRAMEBORDER:
	if(attr->value() == "0" || strcasecmp( attr->value(), "no" ) == 0 )
	    frameborder = false;
	break;
    case ATTR_NORESIZE:
	noresize = true;
	break;
    case ATTR_BORDER:
	border = attr->val()->toInt();
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

    width = availableWidth;

    if(_parent->id() == ID_HTML && view)
	descent = view->clipper()->height();

    ascent = 0;

    int remainingWidth = width - (totalCols-1)*border;
    if(remainingWidth<0) remainingWidth=0;
    int remainingHeight = descent - (totalRows-1)*border;
    if(remainingHeight<0) remainingHeight=0;
    int widthAvailable = remainingWidth;
    int heightAvailable = remainingHeight;

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
		rowHeight[i] = rows->at(i)->width(heightAvailable);
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
	  remainingRelativeWidth = remainingHeight/rowsRelative;
	
	for(i = 0; i< totalRows; i++)
	 {
	    if(rows->at(i)->type == Relative)
	    {
		if ( totalRelative )
		  rowHeight[i] = rows->at(i)->value*remainingHeight/totalRelative;
		else
		  rowHeight[i] = remainingRelativeWidth;
	  	remainingHeight -= rowHeight[i];
		totalRelative--;
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
		colWidth[i] = cols->at(i)->width(widthAvailable);
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

	for(i = 0; i < totalCols; i++)
	{
	    if(cols->at(i)->type == Relative)
	    {
		if ( totalRelative )
		  colWidth[i] = cols->at(i)->value*remainingWidth/totalRelative;
		else
		  colWidth[i] = remainingRelativeWidth;
		remainingWidth -= colWidth[i];
		totalRelative--;
	    }
	}
    }
    else
	colWidth[0] = width;


    positionFrames(deep);

    NodeImpl *child = _first;
    if(!child) return;

    if(!hSplitVar && !vSplitVar)
    {
	printf("calculationg fixed Splitters\n");
	if(!vSplitVar && totalCols > 1)
	{
	    vSplitVar = new bool[totalCols];
	    for(int i = 0; i < totalCols; i++) vSplitVar[i] = true;
	}
	if(!hSplitVar && totalRows > 1)
	{
	    hSplitVar = new bool[totalRows];
	    for(int i = 0; i < totalRows; i++) hSplitVar[i] = true;
	}
	
	for(int r = 0; r < totalRows; r++)
	{
	    for(int c = 0; c < totalCols; c++)
	    {
		bool fixed = false;
		if(child->id() == ID_FRAMESET)
		    fixed = (static_cast<HTMLFrameSetElementImpl *>(child))->noResize();
		else if(child->id() == ID_FRAME)
		    fixed = (static_cast<HTMLFrameElementImpl *>(child))->noResize();

		if(fixed)
		{
		    printf("found fixed cell %d/%d!\n", r, c);
		    if(totalCols > 1)
		    {
			if(c>0) vSplitVar[c-1] = false;
			vSplitVar[c] = false;
		    }
		    if(totalRows > 1)
		    {
			if(r>0) hSplitVar[r-1] = false;
			hSplitVar[r] = false;
		    }
		    child = child->nextSibling();
		    if(!child) goto end2;
		}		
		else
		    printf("not fixed: %d/%d!\n", r, c);
	    }
	}
    }
 end2:
    setLayouted();
}

void HTMLFrameSetElementImpl::positionFrames(bool deep)
{
    int r;
    int c;
    NodeImpl *child = _first;
    if(!child) return;

    int yPos = 0;

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
	    xPos += colWidth[c] + border;
	    child = child->nextSibling();
	    if(!child) return;
	}
	yPos += rowHeight[r] + border;
    }
}

void HTMLFrameSetElementImpl::attach(KHTMLWidget *w)
{
    // ensure the htmlwidget knows we have a frameset, and adjusts the width accordingly
    w->layout();
    view = w;
    NodeBaseImpl::attach(w);
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

bool HTMLFrameSetElementImpl::mouseEvent( int _x, int _y, int button, MouseEventType type,
				  int _tx, int _ty, DOMString &url)
{
    _x-=_tx;
    _y-=_ty;
    printf("mouseEvent\n");

    NodeImpl *child = _first;
    while(child)
    {
	if(child->id() == ID_FRAMESET)
	    if(child->mouseEvent( _x, _y, button, type, _tx, _ty, url)) return true;
	child = child->nextSibling();
    }

    if(noresize) return true;

    if(!resizing && type == MouseMove || type == MousePress)
    {
	printf("mouseEvent:check\n");
	
	hSplit = -1;
	vSplit = -1;
	bool resizePossible = true;

	// check if we're over a horizontal or vertical boundary
	int pos = colWidth[0];
	for(int c = 1; c < totalCols; c++)
	{
	    if(_x >= pos && _x <= pos+border)
	    {
		if(vSplitVar && vSplitVar[c-1] == true) vSplit = c-1;
		printf("vsplit!\n");
		break;
	    }
	    pos += colWidth[c] + border;
	}
	pos = rowHeight[0];
	for(int r = 1; r < totalRows; r++)
	{
	    if( _y >= pos && _y <= pos+border)
	    {
		if(hSplitVar && hSplitVar[r-1] == true) hSplit = r-1;
		printf("hsplitvar = %p\n", hSplitVar);
		printf("hsplit!\n");
		break;
	    }
	    pos += rowHeight[r] + border;
	}
	printf("%d/%d\n", hSplit, vSplit);

	QCursor cursor;
	if(hSplit != -1 && vSplit != -1)
	{
	    cursor = Qt::sizeAllCursor;
	}
	else if( vSplit != -1 )
	{
	    cursor = Qt::splitHCursor;
	}	
	else if( hSplit != -1 )
	{
	    cursor = Qt::splitVCursor;
	}

	if(type == MousePress)
	{
	    resizing = true;
	    KApplication::setOverrideCursor(cursor);
	    vSplitPos = _x;
	    hSplitPos = _y;
	}
	else
	    view->setCursor(cursor);
    }

    // ### need to draw a nice movin indicator for the resize.
    // ### check the resize is not going out of bounds.
    if(resizing && type == MouseRelease)
    {
	resizing = false;
	KApplication::restoreOverrideCursor();

	if(hSplit)
	{
	    printf("split xpos=%d\n", _x);
	    int delta = vSplitPos - _x;
	    colWidth[vSplit] -= delta;
	    colWidth[vSplit+1] += delta;
	}	
	if(vSplit)
	{
	    printf("split ypos=%d\n", _y);
	    int delta = hSplitPos - _y;
	    rowHeight[hSplit] -= delta;
	    rowHeight[hSplit+1] += delta;
	}
	positionFrames(true);
    }
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
    view = 0;
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

    child->setAvailableWidth(availableWidth);
    if(child->id() == ID_FRAMESET)
    {
	KHTMLWidget *w = (KHTMLWidget *)static_cast<HTMLDocumentImpl *>(document)->HTMLWidget();
	if(w) child->setDescent(w->height());
    }

    return NodeBaseImpl::addChild(child);
}

void HTMLHtmlElementImpl::getAbsolutePosition(int &xPos, int &yPos)
{
	xPos = 0, yPos = 0;
}

#include "qdatetime.h"

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
	child->setXPos(view->marginWidth());
	child->setYPos(view->marginHeight());
    }

    if(!child) return;

    QTime qt;
    qt.start();
    child->layout(deep);
    printf("TIME: layout() dt=%d\n",qt.elapsed());

    ascent = 0;
    descent = child->getHeight() + 2*view->marginHeight();

    setLayouted();
}

void HTMLHtmlElementImpl::attach(KHTMLWidget *w)
{
    view = w;
}

void HTMLHtmlElementImpl::setAvailableWidth(int w)
{
#ifdef DEBUG_LAYOUT
    printf("%s(Element)::setAvailableWidth(%d)\n", nodeName().string().ascii(), w);
#endif

    if (w != -1 && w != availableWidth)
    	setLayouted(false);

    if(w != -1) availableWidth = w;

    NodeImpl *child = firstChild();
    minWidth = 0;
    while(child != 0)
    {
    	if (child->getMinWidth() > minWidth)
	    minWidth = child->getMinWidth();
	child = child->nextSibling();
    }
    if(minWidth > availableWidth)
	availableWidth = minWidth;
	
	
    child = firstChild();
    while(child != 0)
    {
    	if (child->getMinWidth() > availableWidth)
	{
	    printf("ERROR: %d too narrow availableWidth=%d minWidth=%d\n",
		   id(), availableWidth, child->getMinWidth());
	    calcMinMaxWidth();
	    setLayouted(false);
	}
	child->setAvailableWidth(availableWidth);
	child = child->nextSibling();
    }
}

