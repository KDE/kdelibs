/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2003 Apple Computer, Inc.
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef RENDER_TABLE_H
#define RENDER_TABLE_H

#include <qcolor.h>
#include <q3ptrvector.h>
#include <QVector>

#include "rendering/render_box.h"
#include "rendering/render_block.h"
#include "rendering/render_style.h"

#include "misc/khtmllayout.h"

namespace DOM {
    class DOMString;
}

namespace khtml {

class RenderTable;
class RenderTableSection;
class RenderTableRow;
class RenderTableCell;
class RenderTableCol;
class TableLayout;

class RenderTable : public RenderBlock
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

    RenderTable(DOM::NodeImpl* node);
    ~RenderTable();

    virtual const char *renderName() const { return "RenderTable"; }

    virtual void setStyle(RenderStyle *style);

    virtual bool isTable() const { return true; }

    int getColumnPos(int col) const
        { return columnPos[col]; }

    int borderHSpacing() const { return hspacing; }
    int borderVSpacing() const { return vspacing; }

    Rules getRules() const { return rules; }

    bool collapseBorders() const { return style()->borderCollapse(); }
    int borderLeft() const;
    int borderRight() const;
    int borderTop() const;
    int borderBottom() const;

    const QColor &bgColor() const { return style()->backgroundColor(); }

    uint cellPadding() const { return padding; }
    void setCellPadding( uint p ) { padding = p; }

    // overrides
    virtual void addChild(RenderObject *child, RenderObject *beforeChild = 0);
    virtual void paint( PaintInfo&, int tx, int ty);
    virtual void paintBoxDecorations(PaintInfo&, int _tx, int _ty);
    virtual void layout();
    virtual void calcMinMaxWidth();
    virtual void close();

    virtual short lineHeight(bool b) const;
    virtual short baselinePosition(bool b) const;

    virtual void setCellWidths( );

    virtual void calcWidth();

    virtual int borderTopExtra();
    virtual int borderBottomExtra();

    virtual FindSelectionResult checkSelectionPoint( int _x, int _y, int _tx, int _ty,
                                                     DOM::NodeImpl*& node, int & offset,
						     SelPointState & );

#ifdef ENABLE_DUMP
    virtual void dump(QTextStream &stream, const QString &ind) const;
#endif
    struct ColumnStruct {
	enum {
	    WidthUndefined = 0xffff
	};
	ColumnStruct() {
	    span = 1;
	    width = WidthUndefined;
	}
	ushort span;
	ushort width; // the calculated position of the column
    };

    QVector<int> columnPos;
    QVector<ColumnStruct> columns;

    void splitColumn( int pos, int firstSpan );
    void appendColumn( int span );
    int numEffCols() const { return columns.size(); }
    int spanOfEffCol( int effCol ) const { return columns[effCol].span; }
    int colToEffCol( int col ) const {
	int c = 0;
	int i = 0;
	while ( c < col && i < (int)columns.size() ) {
	    c += columns[i].span;
	    i++;
	}
	return i;
    }
    int effColToCol( int effCol ) const {
	int c = 0;
	for ( int i = 0; i < effCol; i++ )
	    c += columns[i].span;
	return c;
    }

    int bordersPaddingAndSpacing() const {
	return borderLeft() + borderRight() +
               (collapseBorders() ? 0 : (paddingLeft() + paddingRight() + (numEffCols()+1) * borderHSpacing()));
     }

    RenderTableCol *colElement( int col );

    void setNeedSectionRecalc() { needSectionRecalc = true; }

    virtual RenderObject* removeChildNode(RenderObject* child);

    RenderTableCell* cellAbove(const RenderTableCell* cell) const;
    RenderTableCell* cellBelow(const RenderTableCell* cell) const;
    RenderTableCell* cellLeft(const RenderTableCell* cell) const;
    RenderTableCell* cellRight(const RenderTableCell* cell) const;

    CollapsedBorderValue* currentBorderStyle() { return m_currentBorder; }

    RenderTableSection *firstBodySection() const { return firstBody; }

protected:

    void recalcSections();

    friend class AutoTableLayout;
    friend class FixedTableLayout;

    RenderFlow         *tCaption;
    RenderTableSection *head;
    RenderTableSection *foot;
    RenderTableSection *firstBody;

    TableLayout *tableLayout;

    CollapsedBorderValue* m_currentBorder;

    Frame frame                 : 4;
    Rules rules                 : 4;

    bool has_col_elems		: 1;
    uint needSectionRecalc	: 1;
    uint padding		: 22;

    ushort hspacing;
    ushort vspacing;

    friend class TableSectionIterator;
};

// -------------------------------------------------------------------------

class RenderTableSection : public RenderBox
{
public:
    RenderTableSection(DOM::NodeImpl* node);
    ~RenderTableSection();
    virtual void detach();

    virtual void setStyle(RenderStyle *style);

    virtual const char *renderName() const { return "RenderTableSection"; }

    // overrides
    virtual void addChild(RenderObject *child, RenderObject *beforeChild = 0);
    virtual bool isTableSection() const { return true; }

    virtual short lineHeight(bool) const { return 0; }
    virtual void position(InlineBox*, int, int, bool) {}

    virtual short width() const;

    virtual FindSelectionResult checkSelectionPoint( int _x, int _y, int _tx, int _ty,
                                                     DOM::NodeImpl*& node, int & offset,
						     SelPointState & );

#ifdef ENABLE_DUMP
    virtual void dump(QTextStream &stream, const QString &ind) const;
#endif

    void addCell( RenderTableCell *cell );

    void setCellWidths();
    void calcRowHeight();
    int layoutRows( int height );

    RenderTable *table() const { return static_cast<RenderTable *>(parent()); }

    typedef QVector<RenderTableCell *> Row;
    struct RowStruct {
	Row *row;
	int baseLine;
	Length height;
	bool needFlex;
    };

    RenderTableCell *&cellAt( int row,  int col ) {
	return (*(grid[row].row))[col];
    }
    RenderTableCell *cellAt( int row,  int col ) const {
	return (*(grid[row].row))[col];
    }

    virtual void paint( PaintInfo& i, int tx, int ty);

    int numRows() const { return grid.size(); }
    int getBaseline(int row) {return grid[row].baseLine;}

    void setNeedCellRecalc() {
        needCellRecalc = true;
        table()->setNeedSectionRecalc();
    }

    virtual RenderObject* removeChildNode(RenderObject* child);

    virtual bool canClear(RenderObject *child, PageBreakLevel level);
    void addSpaceAt(int pos, int dy);

    // this gets a cell grid data structure. changing the number of
    // columns is done by the table
    QVector<RowStruct> grid;
    QVector<int> rowPos;

    signed short cRow;
    ushort cCol;
    bool needCellRecalc;

    void recalcCells();
protected:
    void ensureRows( int numRows );
    void clearGrid();
    bool emptyRow(int rowNum);
    bool flexCellChildren(RenderObject* p) const;


    friend class TableSectionIterator;
};

// -------------------------------------------------------------------------

class RenderTableRow : public RenderContainer
{
public:
    RenderTableRow(DOM::NodeImpl* node);

    virtual void detach();

    virtual void setStyle( RenderStyle* );
    virtual const char *renderName() const { return "RenderTableRow"; }

    virtual bool isTableRow() const { return true; }

    // overrides
    virtual void addChild(RenderObject *child, RenderObject *beforeChild = 0);
    virtual RenderObject* removeChildNode(RenderObject* child);

    virtual short offsetWidth() const;
    virtual int offsetHeight() const;
    virtual int offsetLeft() const;
    virtual int offsetTop() const;

    virtual short lineHeight( bool ) const { return 0; }
    virtual void position(InlineBox*, int, int, bool) {}

    virtual void layout();

    void paintRow( PaintInfo& i, int tx, int ty, int w, int h);

    RenderTable *table() const { return static_cast<RenderTable *>(parent()->parent()); }
    RenderTableSection *section() const { return static_cast<RenderTableSection *>(parent()); }

#ifdef ENABLE_DUMP
    virtual void dump(QTextStream &stream, const QString& ind) const;
#endif
};

// -------------------------------------------------------------------------

class RenderTableCell : public RenderBlock
{
public:
    RenderTableCell(DOM::NodeImpl* node);

    virtual void layout();
    virtual void detach();

    virtual const char *renderName() const { return "RenderTableCell"; }
    virtual bool isTableCell() const { return true; }

    // ### FIX these two...
    long cellIndex() const { return 0; }
    void setCellIndex( long ) { }

    unsigned short colSpan() const { return cSpan; }
    void setColSpan( unsigned short c ) { cSpan = c; }

    unsigned short rowSpan() const { return rSpan; }
    void setRowSpan( unsigned short r ) { rSpan = r; }

    int col() const { return _col; }
    void setCol(int col) { _col = col; }
    int row() const { return _row; }
    void setRow(int r) { _row = r; }

    // overrides
    virtual void calcMinMaxWidth();
    virtual void calcWidth();
    virtual void setWidth( int width );
    virtual void setStyle( RenderStyle *style );

    int borderLeft() const;
    int borderRight() const;
    int borderTop() const;
    int borderBottom() const;

    CollapsedBorderValue collapsedLeftBorder() const;
    CollapsedBorderValue collapsedRightBorder() const;
    CollapsedBorderValue collapsedTopBorder() const;
    CollapsedBorderValue collapsedBottomBorder() const;
    virtual void collectBorders(Q3ValueList<CollapsedBorderValue>& borderStyles);

    virtual void updateFromElement();

    void setCellTopExtra(int p) { _topExtra = p; }
    void setCellBottomExtra(int p) { _bottomExtra = p; }
    int cellTopExtra() const { return _topExtra; }
    int cellBottomExtra() const { return _bottomExtra; }

    int pageTopAfter(int x) const;

    virtual void paint( PaintInfo& i, int tx, int ty);

    void paintCollapsedBorder(QPainter* p, int x, int y, int w, int h);

    virtual void close();

    // lie position to outside observers
    virtual int yPos() const { return m_y + _topExtra; }

    virtual void repaintRectangle(int x, int y, int w, int h, bool immediate=false, bool f=false);
    virtual bool absolutePosition(int &xPos, int &yPos, bool f = false);

    virtual short baselinePosition( bool = false ) const;

    virtual bool nodeAtPoint(NodeInfo& info, int _x, int _y, int _tx, int _ty, HitTestAction hitTestAction, bool inside);

    RenderTable *table() const { return static_cast<RenderTable *>(parent()->parent()->parent()); }
    RenderTableSection *section() const { return static_cast<RenderTableSection *>(parent()->parent()); }

#ifdef ENABLE_DUMP
    virtual void dump(QTextStream &stream, const QString &ind) const;
#endif

    bool widthChanged() {
	bool retval = m_widthChanged;
	m_widthChanged = false;
	return retval;
    }

    int cellPercentageHeight() const
	{ return m_percentageHeight; }
    void setCellPercentageHeight(int h)
	{ m_percentageHeight = h; }
    bool hasFlexedAnonymous() const 
        { return m_hasFlexedAnonymous; }      
    void setHasFlexedAnonymous(bool b=true) 
        { m_hasFlexedAnonymous = b; }

protected:
    virtual void paintBoxDecorations(PaintInfo& p, int _tx, int _ty);
    virtual int borderTopExtra() { return _topExtra; }
    virtual int borderBottomExtra() { return _bottomExtra; }

    short _row;
    short _col;
    ushort rSpan;
    ushort cSpan;
    int _topExtra;
    signed int _bottomExtra : 30;
    bool m_widthChanged : 1;
    bool m_hasFlexedAnonymous : 1;
    int m_percentageHeight;
};


// -------------------------------------------------------------------------

class RenderTableCol : public RenderContainer
{
public:
    RenderTableCol(DOM::NodeImpl* node);

    virtual const char *renderName() const { return "RenderTableCol"; }

    long span() const { return _span; }
    void setSpan( long s ) { _span = s; }

    virtual void addChild(RenderObject *child, RenderObject *beforeChild = 0);

    virtual bool isTableCol() const { return true; }

    virtual short lineHeight( bool ) const { return 0; }
    virtual void position(InlineBox*, int, int, bool) {}
    virtual void layout() {}

    virtual void updateFromElement();

#ifdef ENABLE_DUMP
    virtual void dump(QTextStream &stream, const QString& ind) const;
#endif

protected:
    short _span;
};

// -------------------------------------------------------------------------

/** This class provides an iterator to iterate through the sections of a
 * render table in their visual order.
 *
 * In HTML, sections are specified in the order of THEAD, TFOOT, and TBODY.
 * Visually, TBODY sections appear between THEAD and TFOOT, which this iterator
 * takes into regard.
 * @author Leo Savernik
 * @internal
 * @since 3.2
 */
class TableSectionIterator {
public:

  /**
   * Initializes a new iterator
   * @param table table whose sections to iterate
   * @param fromEnd @p true, begin with last section, @p false, begin with
   *	first section.
   */
  TableSectionIterator(RenderTable *table, bool fromEnd = false);

  /**
   * Initializes a new iterator
   * @param start table section to start with.
   */
  TableSectionIterator(RenderTableSection *start) : sec(start) {}

  /**
   * Uninitialized iterator.
   */
  TableSectionIterator() {}

  /** Returns the current section, or @p 0 if the end has been reached.
   */
  RenderTableSection *operator *() const { return sec; }

  /** Moves to the next section in visual order. */
  TableSectionIterator &operator ++();

  /** Moves to the previous section in visual order. */
  TableSectionIterator &operator --();

private:
  RenderTableSection *sec;
};

}
#endif

