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
#ifndef HTML_MISCIMPL_H
#define HTML_MISCIMPL_H

#include "dtd.h"
#include "html_elementimpl.h"
#include "dom_misc.h"

namespace DOM {

class Node;
class DOMString;

class HTMLBaseFontElementImpl : public HTMLElementImpl
{
public:
    HTMLBaseFontElementImpl(DocumentImpl *doc);

    ~HTMLBaseFontElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return BASEFONTStartTag; }
    virtual tagStatus endTag() { return BASEFONTEndTag; }
};

// -------------------------------------------------------------------------

class HTMLCollectionImpl : public DomShared
{
public:
    enum Type {
	// from HTMLDocument
	DOC_IMAGES,    // all IMG elements in the document
	DOC_APPLETS,   // all OBJECT and APPLET elements
	DOC_FORMS,     // all FORMS 
	DOC_LINKS,     // all A _and_ AREA elements with a value for href
	DOC_ANCHORS,      // all A elements with a value for name
	// from HTMLTable, HTMLTableSection, HTMLTableRow
	TABLE_ROWS,    // all rows in this table or tablesection
	TABLE_TBODIES, // all TBODY elements in this table
	TSECTION_ROWS, // all rows elements in this table section
	TR_CELLS,      // all CELLS in this row
	// from FORM
	FORM_ELEMENTS,
	// from SELECT
	SELECT_OPTIONS,
	// from HTMLMap
	MAP_AREAS
    };

    HTMLCollectionImpl(NodeImpl *_base, int _tagId);

    ~HTMLCollectionImpl();
    unsigned long length() const;
    NodeImpl *item ( unsigned long index );
    NodeImpl *namedItem ( const DOMString &name );

protected:
    unsigned long calcLength(NodeImpl *current) const;
    NodeImpl *getItem(NodeImpl *current, int index, int &pos);
    NodeImpl *getNamedItem( NodeImpl *current, int attr_id, 
			    const DOMString &name );
   // the base node, the collection refers to
    NodeImpl *base;
    // The collection list the following elements
    int type;
    
};

}; //namespace

#endif
