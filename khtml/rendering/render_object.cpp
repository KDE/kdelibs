/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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

#include "render_object.h"

#include "dom_nodeimpl.h"
#include "render_box.h"
#include "render_flow.h"
#include "render_style.h"
#include "render_table.h"
#include "render_list.h"
#include "render_style.h"

#include <kdebug.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qpen.h>
#include <qsize.h>

using namespace DOM;
using namespace khtml;

RenderObject *RenderObject::createObject(DOM::NodeImpl *node)
{
    RenderStyle *style = node->style();
    RenderObject *o = 0;
    switch(style->display())
    {
    case INLINE:
    case BLOCK:
	o = new RenderFlow();
	break;
    case LIST_ITEM:
	o = new RenderListItem();
	break;
    case RUN_IN:
    case COMPACT:
    case MARKER:
	break;
    case TABLE:
    case INLINE_TABLE:
	// ### set inline/block right
	//kdDebug( 6040 ) << "creating RenderTable" << endl;
	o = new RenderTable();
	break;
    case TABLE_ROW_GROUP:
    case TABLE_HEADER_GROUP:
    case TABLE_FOOTER_GROUP:
	o = new RenderTableSection();
	break;
    case TABLE_ROW:
	o = new RenderTableRow();
	break;
    case TABLE_COLUMN_GROUP:
    case TABLE_COLUMN:
	o = new RenderTableCol();
	break;
    case TABLE_CELL:
	o = new RenderTableCell();
	break;
    case TABLE_CAPTION:
	o = new RenderTableCaption();
	break;
    case NONE:
	return 0;
    }
    if(o) o->setStyle(style);
    return o;
}


RenderObject::RenderObject()
{
    m_style = 0;
    hasKeyboardFocus=DOM::ActivationOff;

    m_layouted = false;
    m_parsing = false;
    m_minMaxKnown = false;

    m_parent = 0;
    m_previous = 0;
    m_next = 0;
    m_first = 0;
    m_last = 0;

    m_floating = false;
    m_positioned = false;
    m_relPositioned = false;
    m_printSpecial = false;
    m_containsPositioned = false;
    m_isAnonymous = false;

    m_bgImage = 0;
}

RenderObject::~RenderObject()
{
    //kdDebug( 6090 ) << "RenderObject::~RenderObject" << endl;
    // previous and next node may still reference this!!!
    // hope this fix is fine...
    if(m_previous) m_previous->setNextSibling(0);
    if(m_next) m_next->setPreviousSibling(0);

    RenderObject *n;
    RenderObject *next;

    for( n = m_first; n != 0; n = next )
    {
	n->setParent(0);
	next = n->nextSibling();
	if(n->deleteMe()) delete n;
    }

    if(m_bgImage) m_bgImage->deref(this);
}

bool RenderObject::deleteMe()
{
    if(!m_parent && _ref <= 0) return true;
    return false;
}

void RenderObject::addChild(RenderObject *newChild, RenderObject *beforeChild)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6040 ) << renderName() << "(RenderObject)::addChild( " << newChild->renderName() << ", "
                       (beforeChild ? beforeChild->renderName() : "0") << " )" << endl;
#endif

    newChild->setParsing();

    bool needsTable = false;

    if(!newChild->isText()) {
	switch(newChild->style()->display()) {
	case INLINE:
	case BLOCK:
	case LIST_ITEM:
	case RUN_IN:
	case COMPACT:
	case MARKER:
	case TABLE:
	case INLINE_TABLE:
	    break;
	case TABLE_COLUMN_GROUP:
	case TABLE_COLUMN:
	case TABLE_CAPTION:
	case TABLE_ROW_GROUP:
	case TABLE_HEADER_GROUP:
	case TABLE_FOOTER_GROUP:
	    //kdDebug( 6040 ) << "adding section" << endl;
	    if ( !isTable() )
		needsTable = true;
	    break;
	case TABLE_ROW:
	    //kdDebug( 6040 ) << "adding row" << endl;
	    if ( !isTableSection() )
		needsTable = true;
	    break;
	case TABLE_CELL:
	    //kdDebug( 6040 ) << "adding cell" << endl;
	    if ( !isTableRow() )
		needsTable = true;
	    break;
	case NONE:
	    kdDebug( 6000 ) << "error in RenderObject::addChild()!!!!" << endl;
	}
    }

    if ( needsTable ) {
	RenderTable *table;
	if( !beforeChild )
	    beforeChild = lastChild();
	if( beforeChild && beforeChild->isAnonymousBox() && beforeChild->isTable() )
	    table = static_cast<RenderTable *>(beforeChild);
	else {
//	    kdDebug( 6040 ) << "creating anonymous table" << endl;
	    table = new RenderTable();
	    RenderStyle *newStyle = new RenderStyle(m_style);
	    newStyle->setDisplay(TABLE);
	    table->setStyle(newStyle);
	    table->setIsAnonymousBox(true);
	    addChild(table, beforeChild);
	}
	table->addChild(newChild);
	return;
    }

    // just add it...
    newChild->setParent(this);

    if (!beforeChild) {
	if(m_last)
	{
	    newChild->setPreviousSibling(m_last);
	    m_last->setNextSibling(newChild);
	    m_last = newChild;
	}
	else
	{
	    m_first = m_last = newChild;
	}
    }
    else {
	if (beforeChild == m_first)
	    m_first = newChild;
        RenderObject *newPrev = beforeChild->previousSibling();
	newChild->setNextSibling(beforeChild);
	beforeChild->setPreviousSibling(newChild);

	if(newPrev) newPrev->setNextSibling(newChild);
	newChild->setPreviousSibling(newPrev);

    }
    newChild->calcWidth();
}

void RenderObject::removeChild(RenderObject *oldChild)
{
    if (oldChild->previousSibling())
	oldChild->previousSibling()->setNextSibling(oldChild->nextSibling());
    if (oldChild->nextSibling())
	oldChild->nextSibling()->setPreviousSibling(oldChild->previousSibling());

    if (m_first == oldChild)
	m_first = oldChild->nextSibling();
    if (m_last == oldChild)
	m_last = oldChild->previousSibling();

    oldChild->setPreviousSibling(0);
    oldChild->setNextSibling(0);

// Note that we only delete the child here if it is an anonymous box.... othwerwise,
// it has a node attached to it, and will be deleted with that.
    if (oldChild->isAnonymousBox())
	delete oldChild;

    setLayouted(false);
}

RenderObject *RenderObject::containingBlock() const
{
    if(isTableCell()) {
	return static_cast<const RenderTableCell *>(this)->table();
    }

    RenderObject *o = parent();
    if(m_style->position() == FIXED) {
	while ( o && !o->isRoot() )
	    o = o->parent();
    }
    else if(m_style->position() == ABSOLUTE) {
	while (o && o->style()->position() == STATIC && !o->isHtml())
	    o = o->parent();
    } else {
	while(o && o->style()->display() == INLINE)
	    o = o->parent();
    }
    // this is just to make sure we return a valid element.
    // the case below should never happen...
    if(!o) {
	if(!isRoot())
	    kdDebug( 6040 ) << renderName() << "(RenderObject): No containingBlock!" << endl;

	return const_cast<RenderObject *>(this);
    } else
	return o;
}

QSize RenderObject::containingBlockSize() const
{
    RenderObject *o = containingBlock();

    if(m_style->position() == ABSOLUTE)
    {
	if(o->isInline())
	{
	    // ### fixme
	}
	else
	    return o->paddingSize();
    }

    return o->contentSize();
}

short RenderObject::containingBlockWidth() const
{
    // ###
    return containingBlock()->contentWidth();
}

int RenderObject::containingBlockHeight() const
{
    // ###
    return containingBlock()->contentHeight();
}


QSize RenderObject::contentSize() const
{
    return QSize(0, 0);
}

QSize RenderObject::contentOffset() const
{
    return QSize(0, 0);
}

QSize RenderObject::paddingSize() const
{
    return QSize(0, 0);
}

QSize RenderObject::size() const
{
    return QSize(0, 0);
}

void RenderObject::drawBorder(QPainter *p, int x1, int y1, int x2, int y2, int width, BorderSide s, const QColor &c, EBorderStyle style)
{
    switch(style)
    {
    case BNONE:
    case BHIDDEN:
	return;
    case DOTTED:
	p->setPen(QPen(c, width, Qt::DotLine));
	break;
    case DASHED:
	p->setPen(QPen(c, width, Qt::DashLine));
	break;
    case DOUBLE:
    case GROOVE:
    case RIDGE:
    case INSET:
    case OUTSET:
	// ### don't treat them as solid
    case SOLID:
	p->setPen(QPen(c, width, Qt::SolidLine));
	break;
    }

    int half = width/2;

    switch(s)
    {
    case BSTop:
	y1 += half; y2 += half; break;
    case BSBottom:
	y1 -= half; y2 -= half; break;
    case BSLeft:
	x1 += half; x2 += half; break;
    case BSRight:
	x1 -= half; x2 -= half; break;
    }
	
    p->drawLine(x1, y1, x2, y2);
}

void RenderObject::repaintRectangle(int x, int y, int w, int h)
{
    if(m_parent) m_parent->repaintRectangle(x, y, w, h);
}

void RenderObject::repaintObject(RenderObject *o, int x, int y)
{
    if(m_parent) m_parent->repaintObject(o, x, y);
}

// used for clear property & to layout replaced elements
bool RenderObject::isSpecial() const
{
    return (isFloating() || isPositioned());
}

void RenderObject::printTree(int indent) const
{
    QString ind;
    ind.fill(' ', indent);
    kdDebug( 6045 ) << ind << renderName() << ": " << (void*)this
    	    	 << " il=" << isInline() << " ci=" << childrenInline()
                 << " fl=" << isFloating() << " rp=" << isReplaced()
		 << " an=" << isAnonymousBox()
		 << " ps=" << isPositioned()
		 << " cp=" << containsPositioned()
                 << " laytd=" << layouted()
                 << " (" << xPos() << "," << yPos() << "," << width() << "," << height() << ")" << endl;
    RenderObject *child = firstChild();
    while( child != 0 )
    {    	
	child->printTree(indent+2);
	child = child->nextSibling();
    }
}

void RenderObject::selectionStartEnd(int& spos, int& epos)
{
    if (parent())
    	parent()->selectionStartEnd(spos, epos);
}

void RenderObject::updateSize()
{
    containingBlock()->updateSize();
}

void RenderObject::setStyle(RenderStyle *style)
{
    // deletion of styles is handled by the DOM elements
    m_style = style;

    if(m_bgImage) m_bgImage->deref(this);
    m_bgImage = m_style->backgroundImage();
    if(m_bgImage) m_bgImage->ref(this);

    if( m_style->backgroundColor().isValid() || m_style->hasBorder() || m_bgImage )
	m_printSpecial = true;

    setMinMaxKnown(false);
    setLayouted(false);
}

void RenderObject::setContainsPositioned(bool p)
{
    if (p)
    {
    	m_containsPositioned = true;
    	if (containingBlock()!=this)
    	    containingBlock()->setContainsPositioned(true);
    }
    else
    {
	RenderObject *n;
	bool c=false;

	for( n = m_first; n != 0; n = n->nextSibling() )
	{
	    if (n->isPositioned() || n->containsPositioned())
	    	c=true;
	}
	
	if (c)
	    return;
	else
	{
	    m_containsPositioned = false;
	    if (containingBlock()!=this)
    	    	containingBlock()->setContainsPositioned(false);	
	}
    }
}

void RenderObject::setKeyboardFocus(DOM::ActivationState b)
{
  RenderObject *actChild = firstChild();
  printTree(0);
  while(actChild) {
      actChild->setKeyboardFocus(b);
      actChild=actChild->nextSibling();
  }
  hasKeyboardFocus=b;
}

QRect RenderObject::viewRect() const
{
    return containingBlock()->viewRect();
}

void RenderObject::absolutePosition(int &xPos, int &yPos)
{
    if(m_parent)
	m_parent->absolutePosition(xPos, yPos);
    else
	xPos = yPos = -1;
}

void RenderObject::cursorPos(int /*offset*/, int &_x, int &_y, int &height)
{
    _x = _y = height = -1;
}

