/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999 Lars Knoll (knoll@kde.org)
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
#include "html_tableimpl.h"

#include "dom_exception.h"
#include "html_documentimpl.h"
using namespace DOM;

#include "htmlhashes.h"

#include "css/cssstyleselector.h"
#include "css/cssproperties.h"

#include "rendering/render_table.h"
using namespace khtml;

#include <kdebug.h>

HTMLTableElementImpl::HTMLTableElementImpl(DocumentImpl *doc)
  : HTMLElementImpl(doc)
{
    tCaption = 0;
    head = 0;
    foot = 0;
    firstBody = 0;

    rules = None;
    frame = Void;

    incremental = false;
}

HTMLTableElementImpl::~HTMLTableElementImpl()
{
}

const DOMString HTMLTableElementImpl::nodeName() const
{
    return "TABLE";
}

ushort HTMLTableElementImpl::id() const
{
    return ID_TABLE;
}

void HTMLTableElementImpl::setCaption( HTMLTableCaptionElementImpl *c )
{
    if(tCaption)
	replaceChild ( c, tCaption );
    else
	insertBefore( c, firstChild() );
    tCaption = c;
}

void HTMLTableElementImpl::setTHead( HTMLTableSectionElementImpl *s )
{
    if(head)
	replaceChild ( s, head );
    else if( foot )
	insertBefore( s, foot );
    else if( firstBody )
	insertBefore( s, firstBody );
    else
	appendChild( s );
    head = s;
}

void HTMLTableElementImpl::setTFoot( HTMLTableSectionElementImpl *s )
{
    if(foot)
	replaceChild ( s, foot );
    else if( firstBody )
	insertBefore( s, firstBody );
    else
	appendChild( s );
    foot = s;
}

HTMLElementImpl *HTMLTableElementImpl::createTHead(  )
{
    if(!head)
    {
	head = new HTMLTableSectionElementImpl(document, ID_THEAD);
	if(foot)
	    insertBefore( head, foot );
	if(firstBody)
	    insertBefore( head, firstBody);
	else
	    appendChild(head);
    }
    return head;
}

void HTMLTableElementImpl::deleteTHead(  )
{
    if(head) HTMLElementImpl::removeChild(head);
    head = 0;
}

HTMLElementImpl *HTMLTableElementImpl::createTFoot(  )
{
    if(!foot)
    {
	foot = new HTMLTableSectionElementImpl(document, ID_TFOOT);
	if(firstBody)
	    insertBefore( foot, firstBody );
	else
	    appendChild(foot);
    }
    return foot;
}

void HTMLTableElementImpl::deleteTFoot(  )
{
    if(foot) HTMLElementImpl::removeChild(foot);
    foot = 0;
}

HTMLElementImpl *HTMLTableElementImpl::createCaption(  )
{
    if(!tCaption)
    {
	tCaption = new HTMLTableCaptionElementImpl(document);
	insertBefore( tCaption, firstChild() );
    }
    return tCaption;
}

void HTMLTableElementImpl::deleteCaption(  )
{
    if(tCaption) HTMLElementImpl::removeChild(tCaption);
    tCaption = 0;
}

HTMLElementImpl *HTMLTableElementImpl::insertRow( long /*index*/ )
{
    // ###
    return 0;
}

void HTMLTableElementImpl::deleteRow( long /*index*/ )
{
    // ###
}

NodeImpl *HTMLTableElementImpl::addChild(NodeImpl *child)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6030 ) << nodeName().string() << "(Table)::addChild( " << child->nodeName().string() << " )" << endl;
#endif

    switch(child->id())
    {
    case ID_CAPTION:
	setCaption(static_cast<HTMLTableCaptionElementImpl *>(child));
	break;
    case ID_COL:
    case ID_COLGROUP:
    	{
	// these have to come before the table definition!
	if(head || foot || firstBody)
	    throw DOMException(DOMException::HIERARCHY_REQUEST_ERR);
	HTMLElementImpl::addChild(child);
	// ###
#if 0
	HTMLTableColElementImpl* colel = static_cast<HTMLTableColElementImpl *>(child);
	if (_oldColElem)
	    _currentCol = _oldColElem->lastCol();
	_oldColElem = colel;
	colel->setStartCol(_currentCol);
	if (child->id() == ID_COL)
	    _currentCol++;
	else
	    _currentCol+=colel->span();
	addColInfo(colel);	
	incremental = true;
#endif
	}
	return child;
    case ID_THEAD:
	//	if(incremental && !columnPos[totalCols]);// calcColWidth();
	setTHead(static_cast<HTMLTableSectionElementImpl *>(child));
	break;
    case ID_TFOOT:
	//if(incremental && !columnPos[totalCols]);// calcColWidth();
	setTFoot(static_cast<HTMLTableSectionElementImpl *>(child));
	break;
    case ID_TBODY:
	//if(incremental && !columnPos[totalCols]);// calcColWidth();
	if(!firstBody)
	    firstBody = static_cast<HTMLTableSectionElementImpl *>(child);
    default:	
 	HTMLElementImpl::addChild(child);
	break;
    }
    return child;
}

void HTMLTableElementImpl::parseAttribute(AttrImpl *attr)
{
    // ### to CSS!!
    switch(attr->attrId)
    {
    case ATTR_WIDTH:
	addCSSLength(CSS_PROP_WIDTH, attr->value(), false);
    	break;
    case ATTR_HEIGHT:
	addCSSLength(CSS_PROP_HEIGHT, attr->value(), false);
    	break;
    case ATTR_BORDER:
    {
	int border;
	// ### this needs more work, as the border value is not only
	//     the border of the box, but also between the cells
	if(attr->val()->l == 0)
	    border = 1;
	else
	    border = attr->val()->toInt();
#ifdef DEBUG_DRAW_BORDER
    	border=1;
#endif
	QString str;
	str.sprintf("%dpx solid", border);
	addCSSProperty(CSS_PROP_BORDER, str, false);
#if 0	
	// wanted by HTML4 specs
	if(!border)
	    frame = Void, rules = None;
	else
	    frame = Box, rules = All;
#endif
	break;
    }
    case ATTR_BGCOLOR:
	addCSSProperty(CSS_PROP_BACKGROUND_COLOR, attr->value(), false );
	break;
    case ATTR_BACKGROUND:
    {
	HTMLDocumentImpl *doc = static_cast<HTMLDocumentImpl *>(document);
	DOMString url(Cache::completeURL(attr->value(), doc->baseURL()).url());
	addCSSProperty(CSS_PROP_BACKGROUND_IMAGE, url, false );
	break;
    }
    case ATTR_FRAME:
#if 0
	if ( strcasecmp( attr->value(), "void" ) == 0 )
	    frame = Void;
	else if ( strcasecmp( attr->value(), "border" ) == 0 )
	    frame = Box;
	else if ( strcasecmp( attr->value(), "box" ) == 0 )
	    frame = Box;
	else if ( strcasecmp( attr->value(), "hsides" ) == 0 )
	    frame = Hsides;
	else if ( strcasecmp( attr->value(), "vsides" ) == 0 )
	    frame = Vsides;
	else if ( strcasecmp( attr->value(), "above" ) == 0 )
	    frame = Above;
	else if ( strcasecmp( attr->value(), "below" ) == 0 )
	    frame = Below;
	else if ( strcasecmp( attr->value(), "lhs" ) == 0 )
	    frame = Lhs;
	else if ( strcasecmp( attr->value(), "rhs" ) == 0 )
	    frame = Rhs;
	break;
#endif
    case ATTR_RULES:
#if 0
	if ( strcasecmp( attr->value(), "none" ) == 0 )
	    rules = None;
	else if ( strcasecmp( attr->value(), "groups" ) == 0 )
	    rules = Groups;
	else if ( strcasecmp( attr->value(), "rows" ) == 0 )
	    rules = Rows;
	else if ( strcasecmp( attr->value(), "cols" ) == 0 )
	    rules = Cols;
	else if ( strcasecmp( attr->value(), "all" ) == 0 )
	    rules = All;
#endif
    case ATTR_CELLSPACING:
	addCSSLength(CSS_PROP_BORDER_SPACING, attr->value(), false);
	break;
    case ATTR_CELLPADDING:
	addCSSLength(CSS_PROP_PADDING_TOP, attr->value(), false);
	addCSSLength(CSS_PROP_PADDING_LEFT, attr->value(), false);
	addCSSLength(CSS_PROP_PADDING_BOTTOM, attr->value(), false);
	addCSSLength(CSS_PROP_PADDING_RIGHT, attr->value(), false);
	break;
    case ATTR_COLS:
    {
	// ###
#if 0
	int c;
	c = attr->val()->toInt();
	addColumns(c-totalCols);
	break;
#endif
    }
    case ATTR_ALIGN:
	addCSSProperty(CSS_PROP_FLOAT, attr->value(), false);
	break;
    case ATTR_VALIGN:
	addCSSProperty(CSS_PROP_VERTICAL_ALIGN, attr->value(), false);
	break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLTableElementImpl::attach(KHTMLView *w)
{
    HTMLElementImpl::attach(w);
    style()->setFlowAroundFloats(true);
}	

// --------------------------------------------------------------------------

void HTMLTablePartElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_BGCOLOR:
	addCSSProperty(CSS_PROP_BACKGROUND_COLOR, attr->value(), false );
	break;
    case ATTR_BACKGROUND:
    {
	HTMLDocumentImpl *doc = static_cast<HTMLDocumentImpl *>(document);
	DOMString url(Cache::completeURL(attr->value(), doc->baseURL()).url());
	addCSSProperty(CSS_PROP_BACKGROUND_IMAGE, url, false );
	break;
    }
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLTablePartElementImpl::attach(KHTMLView *w)
{
    HTMLElementImpl::attach(w);
}

// -------------------------------------------------------------------------

HTMLTableSectionElementImpl::HTMLTableSectionElementImpl(DocumentImpl *doc,
							 ushort tagid)
    : HTMLTablePartElementImpl(doc)
{
    _id = tagid;
}

HTMLTableSectionElementImpl::~HTMLTableSectionElementImpl()
{
    nrows = 0;
}

const DOMString HTMLTableSectionElementImpl::nodeName() const
{
    return getTagName(_id);
}

ushort HTMLTableSectionElementImpl::id() const
{
    return _id;
}


// these functions are rather slow, since we need to get the row at
// the index... but they aren't used during usual HTML parsing anyway
HTMLElementImpl *HTMLTableSectionElementImpl::insertRow( long index )
{
    nrows++;

    HTMLTableRowElementImpl *r = new HTMLTableRowElementImpl(document);
    if(index < 1)
    {
	insertBefore(r, firstChild());
	return r;
    }

    NodeListImpl *children = childNodes();
    if(!children || (int)children->length() <= index)
	appendChild(r);
    else
	insertBefore(r, children->item(index));
    if(children) delete children;
    return r;
}

void HTMLTableSectionElementImpl::deleteRow( long index )
{
    if(index < 0) return;
    NodeListImpl *children = childNodes();
    if(children && (int)children->length() > index)
    {
	nrows--;
	HTMLElementImpl::removeChild(children->item(index));
    }
    if(children) delete children;
}

// -------------------------------------------------------------------------

HTMLTableRowElementImpl::HTMLTableRowElementImpl(DocumentImpl *doc)
  : HTMLTablePartElementImpl(doc)
{
  rIndex = -1;
}

HTMLTableRowElementImpl::~HTMLTableRowElementImpl()
{
}

const DOMString HTMLTableRowElementImpl::nodeName() const
{
    return "TR";
}

ushort HTMLTableRowElementImpl::id() const
{
    return ID_TR;
}

long HTMLTableRowElementImpl::rowIndex() const
{
    // ###
    return 0;
}

void HTMLTableRowElementImpl::setRowIndex( long  )
{
    // ###
}

void HTMLTableRowElementImpl::setCells( const HTMLCollection & )
{
}

HTMLElementImpl *HTMLTableRowElementImpl::insertCell( long index )
{
    HTMLTableCellElementImpl *c = new HTMLTableCellElementImpl(document, ID_TD);

    if(index < 1)
    {
	insertBefore(c, firstChild());
	return c;
    }

    NodeListImpl *children = childNodes();
    if(!children || (int)children->length() <= index)
	appendChild(c);
    else
	insertBefore(c, children->item(index));
    if(children) delete children;
    return c;
}

void HTMLTableRowElementImpl::deleteCell( long index )
{
    if(index < 0) return;
    NodeListImpl *children = childNodes();
    if(children && (int)children->length() > index)
	HTMLElementImpl::removeChild(children->item(index));
    if(children) delete children;
}

NodeImpl *HTMLTableRowElementImpl::addChild(NodeImpl *child)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6030 ) << nodeName().string() << "(TableRow)::addChild( " << child->nodeName().string() << " )" << endl;
#endif

    NodeImpl *ret = HTMLElementImpl::addChild(child);

    return ret;
}

void HTMLTableRowElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_ALIGN:
	addCSSProperty(CSS_PROP_TEXT_ALIGN, attr->value(), false);
	break;
    case ATTR_VALIGN:
	addCSSProperty(CSS_PROP_VERTICAL_ALIGN, attr->value(), false);
    default:
	HTMLTablePartElementImpl::parseAttribute(attr);
    }
}

// -------------------------------------------------------------------------

HTMLTableCellElementImpl::HTMLTableCellElementImpl(DocumentImpl *doc, int tag)
  : HTMLTablePartElementImpl(doc)
{
  _col = -1;
  _row = -1;
  cSpan = rSpan = 1;
  nWrap = false;
  _id = tag;
  rowHeight = 0;
}

HTMLTableCellElementImpl::~HTMLTableCellElementImpl()
{
}

const DOMString HTMLTableCellElementImpl::nodeName() const
{
    return getTagName(_id);
}

void HTMLTableCellElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_BORDER:
    {
	int border;
	border = attr->val()->toInt();
	QString str;
	str.sprintf("%dpx solid black", border);
	addCSSProperty(CSS_PROP_BORDER, str, false);
    }
    case ATTR_ROWSPAN:
	// ###
	rSpan = attr->val()->toInt();
	if(rSpan < 1) rSpan = 1; // fix for GNOME websites... ;-)
	break;
    case ATTR_COLSPAN:
	// ###
	cSpan = attr->val()->toInt();
	if(cSpan < 1) cSpan = 1; // fix for GNOME websites... ;-)
	break;
    case ATTR_NOWRAP:
	nWrap = true;
	break;
    case ATTR_WIDTH:
	addCSSLength(CSS_PROP_WIDTH, attr->value(), false);
	break;
    case ATTR_HEIGHT:
	addCSSLength(CSS_PROP_HEIGHT, attr->value(), false);
	break;
    case ATTR_ALIGN:
	addCSSProperty(CSS_PROP_TEXT_ALIGN, attr->value(), false);
	break;
    case ATTR_VALIGN:
	addCSSProperty(CSS_PROP_VERTICAL_ALIGN, attr->value(), false);
	break;
    default:
	HTMLTablePartElementImpl::parseAttribute(attr);
    }
}

void HTMLTableCellElementImpl::attach(KHTMLView *_view)
{
    m_style = document->styleSelector()->styleForElement(this);
    khtml::RenderObject *r = _parent->renderer();
    if(r)
    {
	m_render = khtml::RenderObject::createObject(this);
	m_render->ref();
	if(m_render && m_render->style()->display() == TABLE_CELL)
	{
	    RenderTableCell *cell = static_cast<RenderTableCell *>(m_render);
	    cell->setRowSpan(rSpan);
	    cell->setColSpan(cSpan);
	}
	if(m_render) r->addChild(m_render, _next ? _next->renderer() : 0);
    }

    NodeBaseImpl::attach(_view);
}



// -------------------------------------------------------------------------

HTMLTableColElementImpl::HTMLTableColElementImpl(DocumentImpl *doc, ushort i)
    : HTMLElementImpl(doc)
{
    _id = i;
    _span = 1;
}

HTMLTableColElementImpl::~HTMLTableColElementImpl()
{
}

const DOMString HTMLTableColElementImpl::nodeName() const
{
    return getTagName(_id);
}

ushort HTMLTableColElementImpl::id() const
{
    return _id;
}


NodeImpl *HTMLTableColElementImpl::addChild(NodeImpl *child)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6030 ) << nodeName().string() << "(Table)::addChild( " << child->nodeName().string() << " )" << endl;
#endif

    switch(child->id())
    {
    case ID_COL:
    {
	// these have to come before the table definition!	
	HTMLElementImpl::addChild(child);
	HTMLTableColElementImpl* colel = static_cast<HTMLTableColElementImpl *>(child);
	colel->setStartCol(_currentCol);
//	kdDebug( 6030 ) << "_currentCol=" << _currentCol << endl;
	_currentCol++;
	return child;
    }
    default:
	break;
	// ####
    }
    return child;

}

void HTMLTableColElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_SPAN:
	_span = attr->val()->toInt();
	break;
    case ATTR_WIDTH:
	addCSSLength(CSS_PROP_WIDTH, attr->value(), false);
	break;
    case ATTR_ALIGN:
	addCSSProperty(CSS_PROP_TEXT_ALIGN, attr->value(), false);
	break;
    case ATTR_VALIGN:
	addCSSProperty(CSS_PROP_VERTICAL_ALIGN, attr->value(), false);
	break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }

}

// -------------------------------------------------------------------------

HTMLTableCaptionElementImpl::HTMLTableCaptionElementImpl(DocumentImpl *doc)
  : HTMLTablePartElementImpl(doc)
{
}

HTMLTableCaptionElementImpl::~HTMLTableCaptionElementImpl()
{
}

const DOMString HTMLTableCaptionElementImpl::nodeName() const
{
    return "CAPTION";
}

ushort HTMLTableCaptionElementImpl::id() const
{
    return ID_CAPTION;
}


