/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999 Lars Knoll (knoll@kde.org)
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
#ifndef HTML_TABLEIMPL_H
#define HTML_TABLEIMPL_H

#include <qcolor.h>

#include "dtd.h"
#include "html_elementimpl.h"

#include <stdio.h>

namespace DOM {

class DOMString;
class HTMLTableElementImpl;
class HTMLTableSectionElementImpl;
class HTMLTableSectionElement;
class HTMLTableRowElementImpl;
class HTMLTableRowElement;
class HTMLTableCellElementImpl;
class HTMLTableCellElement;
class HTMLTableColElementImpl;
class HTMLTableColElement;
class HTMLTableCaptionElementImpl;
class HTMLTableCaptionElement;
class HTMLElement;
class HTMLCollection;

class HTMLTableElementImpl : public HTMLBlockElementImpl
{
public:
    enum Rules {
	None    = 0x00,
	RGroups = 0x01,
	CGroups = 0x02,
	Groups  = 0x03,
	Rows    = 0x05,
	Cols    = 0x0a,
	All     = 0x0f
    };
    enum Frame {
	Void   = 0x00,
	Above  = 0x01,
	Below  = 0x02,
	Lhs    = 0x04,
	Rhs    = 0x08,
	Hsides = 0x03,
	Vsides = 0x0c,
	Box    = 0x0f
    };

    HTMLTableElementImpl(DocumentImpl *doc);
    ~HTMLTableElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;
    
    virtual int hSpace() { 
    	if (isFloating() && hspace.isUndefined())
	    return 2;
	else
	    return hspace.minWidth(width);
    }
    virtual int vSpace() {  
    	if (isFloating() && vspace.isUndefined())
	    return 2;
	else
	    return vspace.minWidth(getHeight());
    }
    
    virtual tagStatus startTag() { return TABLEStartTag; }
    virtual tagStatus endTag() { return TABLEEndTag; }

    HTMLTableCaptionElementImpl *caption() const { return tCaption; }
    void setCaption( HTMLTableCaptionElementImpl * );

    HTMLTableSectionElementImpl *tHead() const { return head; }
    void setTHead( HTMLTableSectionElementImpl * );

    HTMLTableSectionElementImpl *tFoot() const { return foot; }
    void setTFoot( HTMLTableSectionElementImpl * );

    HTMLElementImpl *createTHead (  );
    void deleteTHead (  );
    HTMLElementImpl *createTFoot (  );
    void deleteTFoot (  );
    HTMLElementImpl *createCaption (  );
    void deleteCaption (  );
    HTMLElementImpl *insertRow ( long index );
    void deleteRow ( long index );

    int getColumnPos(int col) 
	{ return columnPos[col]; }
    int getColumnWidth(int col) 
	{ if(!actColWidth.size() < col) return 0; return actColWidth[col]; }
    int cellSpacing() { return spacing; }
    int cellPadding() { return padding; }
    Rules getRules() { return rules; }
    QColor bgColor() { return bg; }
    virtual bool isFloating() {return halign==Left || halign==Right; }
    virtual bool isInline() {return false; }

    void startRow();
    void addCell( HTMLTableCellElementImpl *cell );
    void endTable();
    void  addColInfo(HTMLTableCellElementImpl *cell);
    void addColInfo(int _startCol, int _colSpan, 
		    int _minSize, int _maxSize, Length _width);

    // overrides
    virtual NodeImpl *addChild(NodeImpl *child);
    virtual void parseAttribute(Attribute *attr);
    virtual void print( QPainter *, int x, int y, int w, int h, 
			int tx, int ty);
    virtual void printObject( QPainter *, int x, int y, int w, int h, 
			int tx, int ty);
    virtual void layout(bool deep = false);
    virtual void setAvailableWidth( int w = -1 );
    virtual void calcMinMaxWidth();
    virtual void close();
    
    virtual void updateSize();

protected:
    /*
     * For each table element with a different width a ColInfo struct is
     * maintained. Consider for example the following table:
     * +---+---+---+
     * | A | B | C |
     * +---+---+---+
     * |   D   | E |
     * +-------+---+
     *
     * This table would result in 4 ColInfo structs being allocated.
     * 1 for A, 1 for B, 1 for C & E, and 1 for D.
     *
     * Note that C and E share the same ColInfo.
     *
     * Note that D has a seperate ColInfo entry.
     *
     * There is always 1 default ColInfo entry which stretches across the
     * entire table.  
     */
    typedef struct ColInfo_struct
    {
	int     startCol;
	int     colSpan;
	int     minSize;
	int     prefSize;
	Length  width;     // width as predefined by attributes
    } ColInfo_t;
    
    // this function is used in case <col> or <colgroup> elements are defined
    // table layout can be done incrementally in this case
    void calcColWidth();
    // This function calculates the actual widths of the columns
    // for tables which can't be rendered incrementally
    void calcColWidthII(void);

    // calculates the height of each row
    void calcRowHeights();

    // 
    // A small set of helper functions for spreading width across columns
    void addColMinWidth(int k, int delta);
    void addColsMinWidthEqual(int kol, int span, int tooAdd);
    void addColsMinWidthVar(int kol, int span, int tooAdd, int varCount);
    int addColsMinWidthNonVar(int kol, int span, int tooAdd, int fixedCount);
    void addColsMinWidthNonFix(int kol, int span, int tooAdd, int nonFixedCount);
    void addColsMaxWidthEqual(int kol, int span, int tooAdd);
    void addColsMaxWidthVar(int kol, int span, int tooAdd, int varCount);

    void setCells( unsigned int r, unsigned int c, 
		   HTMLTableCellElementImpl *cell );
    void addRows( int num );
    void addColumns( int num );
    // ### need to provide some delete* methods too...

    HTMLTableCellElementImpl ***cells;
    //QArray<ColInfo_t> colInfo;
    int maxColSpan;

    QArray<int> columnPos;
    QArray<int> colMaxWidth;
    QArray<int> colMinWidth;
    QArray<LengthType> colType;
    QArray<int> colValue;
    QArray<int> rowHeights;
    QArray<int> actColWidth;

    unsigned int totalColInfos;
    unsigned int col;
    unsigned int totalCols;
    unsigned int row;
    unsigned int totalRows;
    unsigned int allocRows;

    HTMLTableCaptionElementImpl *tCaption;
    HTMLTableColElementImpl *cols;
    HTMLTableSectionElementImpl *head;
    HTMLTableSectionElementImpl *foot;
    HTMLTableSectionElementImpl *firstBody;

    Length predefinedWidth;
    int border;
    int spacing;
    int padding;
    
    Length vspace;
    Length hspace;

    QColor bg;
    Frame frame;
    Rules rules;

    bool incremental;
};

// -------------------------------------------------------------------------

class HTMLTablePartElementImpl : public HTMLBlockElementImpl
{
public:
    HTMLTablePartElementImpl(DocumentImpl *doc)
	: HTMLBlockElementImpl(doc) 
	{ table = 0; }
    HTMLTablePartElementImpl(DocumentImpl *doc, HTMLTableElementImpl *t)
	: HTMLBlockElementImpl(doc) 
	{ table = t; }

    void setTable(HTMLTableElementImpl *t) { table = t; }

    virtual NodeImpl *addChild(NodeImpl *child)
	{
	    NodeImpl *ret = HTMLBlockElementImpl::addChild(child);
	    static_cast<HTMLTablePartElementImpl *>(child)
		->setTable(this->table);
	    return ret;
	}
    virtual bool mouseEvent( int _x, int _y, int button, MouseEventType t, 
			     int _tx, int _ty, DOMString &url)
	{
	    return HTMLElementImpl::mouseEvent(_x, _y, button, t, 
					       _tx, _ty, url);
	} 
    virtual void updateSize() 
    { 
    	calcMinMaxWidth();    
	setLayouted(false);
	if(_parent) _parent->updateSize(); 
    }

protected:
    HTMLTableElementImpl *table;
};

// -------------------------------------------------------------------------

class HTMLTableSectionElementImpl : public HTMLTablePartElementImpl
{
public:
    HTMLTableSectionElementImpl(DocumentImpl *doc, ushort tagid);

    ~HTMLTableSectionElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return THEADStartTag; }
    virtual tagStatus endTag() { return THEADEndTag; }

    HTMLElementImpl *insertRow ( long index );
    void deleteRow ( long index );

    int numRows() { return nrows; }

    // overrides
    virtual void layout(bool deep = false);
    virtual NodeImpl *addChild(NodeImpl *child);

protected:
    ushort _id;
    int nrows;
};

// -------------------------------------------------------------------------

class HTMLTableRowElementImpl : public HTMLTablePartElementImpl
{
public:
    HTMLTableRowElementImpl(DocumentImpl *doc);

    ~HTMLTableRowElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return TRStartTag; }
    virtual tagStatus endTag() { return TREndTag; }

    long rowIndex() const;
    void setRowIndex( long );

    long sectionRowIndex() const { return rIndex; }
    void setSectionRowIndex( long i ) { rIndex = i; }

    void setCells( const HTMLCollection & );

    HTMLElementImpl *insertCell ( long index );
    void deleteCell ( long index );

    QColor bgColor() { return bg; }

    // overrides
    virtual NodeImpl *addChild(NodeImpl *child);
    virtual void parseAttribute(Attribute *attr);

protected:
    // relative to the current section!
    int rIndex;
    QColor bg;
    int ncols;
};

// -------------------------------------------------------------------------

class HTMLTableCellElementImpl : public HTMLTablePartElementImpl
{
public:
    HTMLTableCellElementImpl(DocumentImpl *doc, int tagId);

    ~HTMLTableCellElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const { return _id; }

    virtual tagStatus startTag() { return TDStartTag; }
    virtual tagStatus endTag() { return TDEndTag; }

    // ### FIX these two...
    long cellIndex() const { return 0; }
    void setCellIndex( long ) { } 

    long colSpan() const { return cSpan; }
    void setColSpan( long c ) { cSpan = c; }

    long rowSpan() const { return rSpan; }
    void setRowSpan( long r ) { rSpan = r; }

    bool noWrap() const { return nWrap; }
    void setNoWrap( bool b ) {nWrap = b; } 

    int col() { return _col; }
    void setCol(int col) { _col = col; }
    int row() { return _row; }
    void setRow(int r) { _row = r; }

    LengthType colType() { return predefinedWidth.type; }
    Length predefWidth() { return predefinedWidth; }

    // overrides
    virtual void parseAttribute(Attribute *attr);
    virtual NodeImpl *addChild(NodeImpl *child)
	{
	    return HTMLBlockElementImpl::addChild(child);
	}
    virtual void calcMinMaxWidth();
    virtual void print( QPainter *, int x, int y, int w, int h, 
			int tx, int ty);
    virtual void printObject( QPainter *, int x, int y, int w, int h, 
			int tx, int ty);
    void setRowHeight(int h) { rowHeight = h; }
    virtual bool mouseEvent( int _x, int _y, int button, MouseEventType t, 
			     int _tx, int _ty, DOMString &url)
	{
	    return HTMLBlockElementImpl::mouseEvent(_x, _y, button, t, 
						    _tx, _ty, url);
	} 
   
protected:
    int _row;
    int _col;
    int rSpan;
    int cSpan;
    bool nWrap;
    Length predefinedWidth;
    Length predefinedHeight;
    int _id;
    int rowHeight;
    QColor bg;
};

// -------------------------------------------------------------------------

class HTMLTableColElementImpl : public HTMLElementImpl
{
public:
    HTMLTableColElementImpl(DocumentImpl *doc, ushort i);

    ~HTMLTableColElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return COLStartTag; }
    virtual tagStatus endTag() { return COLEndTag; }

    long span() const { return _span; }
    void setSpan( long s ) { _span = s; }
    Length width() { return predefinedWidth; }

    // overrides
    virtual void parseAttribute(Attribute *attr);

protected:
    // could be ID_COL or ID_COLGROUP ... The DOM is not quite clear on
    // this, but since both elements work quite similar, we use one
    // DOMElement for them...
    ushort _id; 
    int _span;
    Length predefinedWidth;
};

// -------------------------------------------------------------------------

class HTMLTableCaptionElementImpl : public HTMLTablePartElementImpl
{
public:
    HTMLTableCaptionElementImpl(DocumentImpl *doc);

    ~HTMLTableCaptionElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() { return CAPTIONStartTag; }
    virtual tagStatus endTag() { return CAPTIONEndTag; }

};

}; //namespace

#endif
