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
// --------------------------------------------------------------------------

#include "dom_string.h"
#include "html_element.h"
#include "html_misc.h"
#include "html_miscimpl.h"

#include "html_table.h"
#include "html_tableimpl.h"
using namespace DOM;


HTMLTableCaptionElement::HTMLTableCaptionElement() : HTMLElement()
{
}

HTMLTableCaptionElement::HTMLTableCaptionElement(const HTMLTableCaptionElement &other) : HTMLElement(other)
{
}

HTMLTableCaptionElement::HTMLTableCaptionElement(HTMLTableCaptionElementImpl *impl) : HTMLElement(impl)
{
}

HTMLTableCaptionElement &HTMLTableCaptionElement::operator = (const HTMLTableCaptionElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLTableCaptionElement::~HTMLTableCaptionElement()
{
}

DOMString HTMLTableCaptionElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLTableCaptionElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

// --------------------------------------------------------------------------

HTMLTableCellElement::HTMLTableCellElement() : HTMLElement()
{
}

HTMLTableCellElement::HTMLTableCellElement(const HTMLTableCellElement &other) : HTMLElement(other)
{
}

HTMLTableCellElement::HTMLTableCellElement(HTMLTableCellElementImpl *impl) : HTMLElement(impl)
{
}

HTMLTableCellElement &HTMLTableCellElement::operator = (const HTMLTableCellElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLTableCellElement::~HTMLTableCellElement()
{
}

long HTMLTableCellElement::cellIndex() const
{
    if(!impl) return 0;
    return ((HTMLTableCellElementImpl *)impl)->cellIndex();
}

void HTMLTableCellElement::setCellIndex( long _cellIndex )
{

    if(impl)
        ((HTMLTableCellElementImpl *)impl)->setCellIndex( _cellIndex );
}

DOMString HTMLTableCellElement::abbr() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("abbr");
}

void HTMLTableCellElement::setAbbr( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("abbr", value);
}

DOMString HTMLTableCellElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLTableCellElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

DOMString HTMLTableCellElement::axis() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("axis");
}

void HTMLTableCellElement::setAxis( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("axis", value);
}

DOMString HTMLTableCellElement::bgColor() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("bgColor");
}

void HTMLTableCellElement::setBgColor( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("bgColor", value);
}

DOMString HTMLTableCellElement::ch() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("ch");
}

void HTMLTableCellElement::setCh( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("ch", value);
}

DOMString HTMLTableCellElement::chOff() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("chOff");
}

void HTMLTableCellElement::setChOff( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("chOff", value);
}

long HTMLTableCellElement::colSpan() const
{
    if(!impl) return 0;
    return ((HTMLTableCellElementImpl *)impl)->colSpan();
}

void HTMLTableCellElement::setColSpan( long _colSpan )
{

    if(impl)
        ((HTMLTableCellElementImpl *)impl)->setColSpan( _colSpan );
}

DOMString HTMLTableCellElement::headers() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("headers");
}

void HTMLTableCellElement::setHeaders( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("headers", value);
}

DOMString HTMLTableCellElement::height() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("height");
}

void HTMLTableCellElement::setHeight( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("height", value);
}

bool HTMLTableCellElement::noWrap() const
{
    if(!impl) return 0;
    return ((HTMLTableCellElementImpl *)impl)->noWrap();
}

void HTMLTableCellElement::setNoWrap( bool _noWrap )
{
    if(impl)
        ((HTMLTableCellElementImpl *)impl)->setNoWrap( _noWrap );
}

long HTMLTableCellElement::rowSpan() const
{
    if(!impl) return 0;
    return ((HTMLTableCellElementImpl *)impl)->rowSpan();
}

void HTMLTableCellElement::setRowSpan( long _rowSpan )
{

    if(impl)
        ((HTMLTableCellElementImpl *)impl)->setRowSpan( _rowSpan );
}

DOMString HTMLTableCellElement::scope() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("scope");
}

void HTMLTableCellElement::setScope( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("scope", value);
}

DOMString HTMLTableCellElement::vAlign() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("vAlign");
}

void HTMLTableCellElement::setVAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("vAlign", value);
}

DOMString HTMLTableCellElement::width() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("width");
}

void HTMLTableCellElement::setWidth( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("width", value);
}

// --------------------------------------------------------------------------

HTMLTableColElement::HTMLTableColElement() : HTMLElement()
{
}

HTMLTableColElement::HTMLTableColElement(const HTMLTableColElement &other) : HTMLElement(other)
{
}

HTMLTableColElement::HTMLTableColElement(HTMLTableColElementImpl *impl) : HTMLElement(impl)
{
}

HTMLTableColElement &HTMLTableColElement::operator = (const HTMLTableColElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLTableColElement::~HTMLTableColElement()
{
}

DOMString HTMLTableColElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLTableColElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

DOMString HTMLTableColElement::ch() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("ch");
}

void HTMLTableColElement::setCh( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("ch", value);
}

DOMString HTMLTableColElement::chOff() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("chOff");
}

void HTMLTableColElement::setChOff( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("chOff", value);
}

long HTMLTableColElement::span() const
{
    if(!impl) return 0;
    return ((HTMLTableColElementImpl *)impl)->span();
}

void HTMLTableColElement::setSpan( long _span )
{

    if(impl)
        ((HTMLTableColElementImpl *)impl)->setSpan( _span );
}

DOMString HTMLTableColElement::vAlign() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("vAlign");
}

void HTMLTableColElement::setVAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("vAlign", value);
}

DOMString HTMLTableColElement::width() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("width");
}

void HTMLTableColElement::setWidth( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("width", value);
}

// --------------------------------------------------------------------------

HTMLTableElement::HTMLTableElement() : HTMLElement()
{
}

HTMLTableElement::HTMLTableElement(const HTMLTableElement &other) : HTMLElement(other)
{
}

HTMLTableElement::HTMLTableElement(HTMLTableElementImpl *impl) : HTMLElement(impl)
{
}

HTMLTableElement &HTMLTableElement::operator = (const HTMLTableElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLTableElement::~HTMLTableElement()
{
}

HTMLTableCaptionElement HTMLTableElement::caption() const
{
    if(!impl) return 0;
    return ((HTMLTableElementImpl *)impl)->caption();
}

void HTMLTableElement::setCaption( const HTMLTableCaptionElement &_caption )
{

    if(impl)
        ((HTMLTableElementImpl *)impl)
	    ->setCaption( ((HTMLTableCaptionElementImpl *)_caption.impl) );
}

HTMLTableSectionElement HTMLTableElement::tHead() const
{
    if(!impl) return 0;
    return ((HTMLTableElementImpl *)impl)->tHead();
}

void HTMLTableElement::setTHead( const HTMLTableSectionElement &_tHead )
{

    if(impl)
        ((HTMLTableElementImpl *)impl)
	    ->setTHead( ((HTMLTableSectionElementImpl *)_tHead.impl) );
}

HTMLTableSectionElement HTMLTableElement::tFoot() const
{
    if(!impl) return 0;
    return ((HTMLTableElementImpl *)impl)->tFoot();
}

void HTMLTableElement::setTFoot( const HTMLTableSectionElement &_tFoot )
{

    if(impl)
        ((HTMLTableElementImpl *)impl)
	    ->setTFoot( ((HTMLTableSectionElementImpl *)_tFoot.impl) );
}

HTMLCollection HTMLTableElement::rows() const
{
    if(!impl) return HTMLCollection();
    return HTMLCollection(impl, HTMLCollectionImpl::TABLE_ROWS);
}

HTMLCollection HTMLTableElement::tBodies() const
{
    if(!impl) return HTMLCollection();
    return HTMLCollection(impl, HTMLCollectionImpl::TABLE_TBODIES);
}

DOMString HTMLTableElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLTableElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

DOMString HTMLTableElement::bgColor() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("bgColor");
}

void HTMLTableElement::setBgColor( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("bgColor", value);
}

DOMString HTMLTableElement::border() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("border");
}

void HTMLTableElement::setBorder( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("border", value);
}

DOMString HTMLTableElement::cellPadding() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("cellPadding");
}

void HTMLTableElement::setCellPadding( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("cellPadding", value);
}

DOMString HTMLTableElement::cellSpacing() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("cellSpacing");
}

void HTMLTableElement::setCellSpacing( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("cellSpacing", value);
}

DOMString HTMLTableElement::frame() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("frame");
}

void HTMLTableElement::setFrame( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("frame", value);
}

DOMString HTMLTableElement::rules() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("rules");
}

void HTMLTableElement::setRules( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("rules", value);
}

DOMString HTMLTableElement::summary() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("summary");
}

void HTMLTableElement::setSummary( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("summary", value);
}

DOMString HTMLTableElement::width() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("width");
}

void HTMLTableElement::setWidth( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("width", value);
}

HTMLElement HTMLTableElement::createTHead(  )
{
    if(!impl) return 0;
    return ((HTMLTableElementImpl *)impl)->createTHead(  );
}

void HTMLTableElement::deleteTHead(  )
{
    if(impl)
        ((HTMLTableElementImpl *)impl)->deleteTHead(  );
}

HTMLElement HTMLTableElement::createTFoot(  )
{
    if(!impl) return 0;
    return ((HTMLTableElementImpl *)impl)->createTFoot(  );
}

void HTMLTableElement::deleteTFoot(  )
{
    if(impl)
        ((HTMLTableElementImpl *)impl)->deleteTFoot(  );
}

HTMLElement HTMLTableElement::createCaption(  )
{
    if(!impl) return 0;
    return ((HTMLTableElementImpl *)impl)->createCaption(  );
}

void HTMLTableElement::deleteCaption(  )
{
    if(impl)
        ((HTMLTableElementImpl *)impl)->deleteCaption(  );
}

HTMLElement HTMLTableElement::insertRow( long index )
{
    if(!impl) return 0;
    return ((HTMLTableElementImpl *)impl)->insertRow( index );
}

void HTMLTableElement::deleteRow( long index )
{
    if(impl)
        ((HTMLTableElementImpl *)impl)->deleteRow( index );
}

// --------------------------------------------------------------------------

HTMLTableRowElement::HTMLTableRowElement() : HTMLElement()
{
}

HTMLTableRowElement::HTMLTableRowElement(const HTMLTableRowElement &other) : HTMLElement(other)
{
}

HTMLTableRowElement::HTMLTableRowElement(HTMLTableRowElementImpl *impl) : HTMLElement(impl)
{
}

HTMLTableRowElement &HTMLTableRowElement::operator = (const HTMLTableRowElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLTableRowElement::~HTMLTableRowElement()
{
}

long HTMLTableRowElement::rowIndex() const
{
    if(!impl) return 0;
    return ((HTMLTableRowElementImpl *)impl)->rowIndex();
}

void HTMLTableRowElement::setRowIndex( long _rowIndex )
{

    if(impl)
        ((HTMLTableRowElementImpl *)impl)->setRowIndex( _rowIndex );
}

long HTMLTableRowElement::sectionRowIndex() const
{
    if(!impl) return 0;
    return ((HTMLTableRowElementImpl *)impl)->sectionRowIndex();
}

void HTMLTableRowElement::setSectionRowIndex( long _sectionRowIndex )
{

    if(impl)
        ((HTMLTableRowElementImpl *)impl)->setSectionRowIndex( _sectionRowIndex );
}

HTMLCollection HTMLTableRowElement::cells() const
{
    if(!impl) return HTMLCollection();
    return HTMLCollection(impl, HTMLCollectionImpl::TABLE_ROWS);
}

void HTMLTableRowElement::setCells( const HTMLCollection &_cells )
{
    if(impl)
        ((HTMLTableRowElementImpl *)impl)->setCells( _cells );
}

DOMString HTMLTableRowElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLTableRowElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

DOMString HTMLTableRowElement::bgColor() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("bgColor");
}

void HTMLTableRowElement::setBgColor( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("bgColor", value);
}

DOMString HTMLTableRowElement::ch() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("ch");
}

void HTMLTableRowElement::setCh( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("ch", value);
}

DOMString HTMLTableRowElement::chOff() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("chOff");
}

void HTMLTableRowElement::setChOff( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("chOff", value);
}

DOMString HTMLTableRowElement::vAlign() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("vAlign");
}

void HTMLTableRowElement::setVAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("vAlign", value);
}

HTMLElement HTMLTableRowElement::insertCell( long index )
{
    if(!impl) return 0;
    return ((HTMLTableRowElementImpl *)impl)->insertCell( index );
}

void HTMLTableRowElement::deleteCell( long index )
{
    if(impl)
        ((HTMLTableRowElementImpl *)impl)->deleteCell( index );
}

// --------------------------------------------------------------------------

HTMLTableSectionElement::HTMLTableSectionElement() : HTMLElement()
{
}

HTMLTableSectionElement::HTMLTableSectionElement(const HTMLTableSectionElement &other) : HTMLElement(other)
{
}

HTMLTableSectionElement::HTMLTableSectionElement(HTMLTableSectionElementImpl *impl) : HTMLElement(impl)
{
}

HTMLTableSectionElement &HTMLTableSectionElement::operator = (const HTMLTableSectionElement &other)
{
    HTMLElement::operator = (other);
    return *this;
}

HTMLTableSectionElement::~HTMLTableSectionElement()
{
}

DOMString HTMLTableSectionElement::align() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("align");
}

void HTMLTableSectionElement::setAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("align", value);
}

DOMString HTMLTableSectionElement::ch() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("ch");
}

void HTMLTableSectionElement::setCh( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("ch", value);
}

DOMString HTMLTableSectionElement::chOff() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("chOff");
}

void HTMLTableSectionElement::setChOff( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("chOff", value);
}

DOMString HTMLTableSectionElement::vAlign() const
{
    if(!impl) return 0;
    return ((ElementImpl *)impl)->getAttribute("vAlign");
}

void HTMLTableSectionElement::setVAlign( const DOMString &value )
{
    if(impl) ((ElementImpl *)impl)->setAttribute("vAlign", value);
}

HTMLCollection HTMLTableSectionElement::rows() const
{
    if(!impl) return HTMLCollection();
    return HTMLCollection(impl, HTMLCollectionImpl::TABLE_ROWS);
}

HTMLElement HTMLTableSectionElement::insertRow( long index )
{
    if(!impl) return 0;
    return ((HTMLTableSectionElementImpl *)impl)->insertRow( index );
}

void HTMLTableSectionElement::deleteRow( long index )
{
    if(impl)
        ((HTMLTableSectionElementImpl *)impl)->deleteRow( index );
}

