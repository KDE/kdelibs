/*
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)
              (C) 1998 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- Tables
// $Id$

#ifndef HTMLTABLE_H
#define HTMLTABLE_H

#include <qpainter.h>
#include <qlist.h>
#include <qstrlist.h>
#include <qarray.h>
#include <qpixmap.h>
#include <qstack.h>
#include <qfont.h>
#include <qtimer.h>

#include <kurl.h>

class HTMLChain;
class HTMLObject;
class HTMLCellInfo;
class HTMLClueV;
class HTMLImage;
class HTMLClue;
class HTMLClueFlow;
class HTMLClueAligned;
class KHTMLWidget;

#include "khtmlclue.h"

//-----------------------------------------------------------------------------
// really only useful for tables.
//
// This is a HTMLClueV with as extra features:
// * rowspan & colspan property
// * a background color can be set 
//
class HTMLTableCell : public HTMLClueV
{
public:
    HTMLTableCell( int _width, int colType, 
		   int rs, int cs, int pad );
    virtual ~HTMLTableCell() { }

    int rowSpan() const
	    {	return rspan; }
    int colSpan() const
	    {	return cspan; }
    const QColor &bgColor() const
	    {	return bg; }

    void setBGColor( const QColor &c )
	    {	bg = c; }

    virtual void setMaxWidth( int );
    virtual void calcSize( HTMLClue *parent = 0L );

    int getPercent()
        {   return percent; }
    // cell type (fixed, percent, ... )
    int type() { return colType; }
    bool closed() { return cellClosed; };
    void setClosed(bool c) { cellClosed = c; };

    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
		int _height, int _tx, int _ty, bool toPrinter );
    virtual void print( QPainter *_painter, HTMLChain *_chain, int _x,
	int _y, int _width, int _height, int _tx, int _ty );
    virtual void print( QPainter *_painter, HTMLObject *_obj, int _x, int _y,
		int _width, int _height, int _tx, int _ty )
	{ HTMLClueV::print(_painter,_obj,_x,_y,_width,_height,_tx,_ty); }

    virtual const char * objectName() const { return "HTMLTableCell"; };
    virtual ObjectType type() const
	    {	return TableCell; }

protected:
    int percent;
    int rspan;
    int cspan;
    int padding;
    QColor bg;
    int colType;
    bool cellClosed;
};

//-----------------------------------------------------------------------------
//
class HTMLRowInfo
{
public:
    struct Row 
    {
	Row()
	{
	    row = new int[5];
	    alloc = 5;
	    len = 0;
	}
	Row(int l)
	{
	    row = new int[l];
	    alloc = l;
	    len = 0;
	}
	~Row()
	{
	    if( row && alloc )
		delete [] row;
	}
	void append(int num)
	{
	    if(len == alloc)
	    {
		alloc += 5;
		int *newrow = new int[alloc];
		memcpy( newrow, row, len * sizeof(int) );		
		memset( newrow + len, 0, 5 * sizeof(int) );
		delete row;
		row = newrow;
	    }
	    row[len] = num;
	    len++;
	}
	int *row;
	int len;
	int alloc;
    };

    HTMLRowInfo()
    {
	rowCount = 0;
	entry = new Row *[5];
	alloc = 5;
	current = 0;
    }
    ~HTMLRowInfo() 
    { 
	if(entry) 
	{
	    for( unsigned int i = 0; i < rowCount; i++ ) 
		if( entry[i] ) delete entry[i];
	    delete [] entry; 
	}
    }
    int *operator[](int num) { return entry[num]->row; }
    void append( int i ) 
    { 
	if( !current ) newRow();
	current->append(i); 
    }
    int pos();
    unsigned int row() { return rowCount; }
    int len(int row) { return entry[row]->len; }
    void newRow();
    void endRow();

protected:
    Row **entry;
    Row *current; // The current row we are working on.
    unsigned int rowCount;
    unsigned int alloc;
};


class HTMLTable : public HTMLClue
{
public:
    HTMLTable( int _percent, int _width,
		int _padding = 1, int _spacing = 2, int _border = 0 );
    virtual ~HTMLTable();

    void setCaption( HTMLClueV *cap, HTMLClue::VAlign al )
	    { caption = cap; capAlign = al; cap->setParent(this); }

    enum ColType { Fixed, Percent, Relative, Variable };

    virtual void append( HTMLObject *obj );
    HTMLTableCell *append( int width, ColType colType,
		 int rowSpan, int colSpan, 
		 QColor bgcolor,  HTMLClue::VAlign valign );
    void endCell();
    void startRow();
    void endRow();
    virtual HTMLClue *close();

    virtual ObjectType type() const
	    {	return Table; }

    // function for incremental layout, used by <col> tag
    void addColumns(int num, int width, ColType colType, 
		    HTMLClue::HAlign halign, HTMLClue::VAlign valign);

    virtual void reset();
    virtual int  calcMinWidth();
    virtual int  calcPreferredWidth();

    virtual void calcSize( HTMLClue *parent = 0L );
    virtual void setMaxWidth( int _max_width );
    virtual void setMaxAscent( int );
    virtual HTMLObject *checkPoint( int, int );
    virtual HTMLObject *mouseEvent( int, int, int, int );
    virtual void selectByURL( KHTMLWidget *, HTMLChain *, const char *, bool, int _tx, int _ty );
    virtual void select( KHTMLWidget *, HTMLChain *, QRegExp& _pattern, bool _select,
	    int _tx, int _ty );
    virtual void select( KHTMLWidget *, HTMLChain *,  bool, int _tx, int _ty );
    /**
     * Selects every object in this table if it is inside the rectangle
     * and deselects it otherwise.
     */
    virtual void select( KHTMLWidget *, HTMLChain *,  QRect &_rect, int _tx, int _ty );
    virtual void select( bool );
    virtual bool selectText( KHTMLWidget *_htmlw, HTMLChain *_chain, int _x1,
	int _y1, int _x2, int _y2, int _tx, int _ty );
    virtual void getSelected( QStrList & );
    virtual void getSelectedText( QString & );

    virtual void calcAbsolutePos( int _x, int _y );
    virtual bool getObjectPosition( const HTMLObject *objp, int &xp, int &yp );

    virtual HTMLAnchor *findAnchor( const char *_name, int &_x, int &_y );

    virtual int  findPageBreak( int _y );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
    	    int _height, int _tx, int _ty, bool toPrinter );
    virtual void print( QPainter *_painter, HTMLChain *_chain, int _x,
	    int _y, int _width, int _height, int _tx, int _ty );
    virtual void print( QPainter *_painter, HTMLObject *_obj, int _x, int _y,
	    int _width, int _height, int _tx, int _ty );
    virtual void print( QPainter *, int _tx, int _ty );

    virtual void findCells( int _tx, int _ty, QList<HTMLCellInfo> &_list );

    int rows() const { return totalRows; }
    int cols() const { return totalCols; }
    HTMLTableCell *cell( int r, int c )
	{ return cells[r][c]; }

    int getPadding() { return padding; }
    void setColor ( QColor c ) { tableColor = c; tableRowColor = c; }
    QColor color() { return tableColor; }
    void setRowColor ( QColor c ) { tableRowColor = c; }
    QColor rowColor() { return tableRowColor; }
    void setRowVAlign( HTMLClue::VAlign a ) { rowvalign = a; }
    HTMLClue::VAlign rowVAlign() { return rowvalign; }
    void setRowHAlign( HTMLClue::HAlign a ) { rowhalign = a; }
    HTMLClue::HAlign rowHAlign() { return rowhalign; }

    void setOldClue ( HTMLClue *c ) { oldclue = c; }
    HTMLClue *oldClue() { return oldclue; }
    void setOldDivAlign ( HTMLClue::HAlign a ) { olddivalign = a; }
    HTMLClue::HAlign oldDivAlign() { return olddivalign; }
    void setOldIndent ( int i ) { oldindent = i; }
    int oldIndent() { return oldindent; }

    /*
     * Create an iterator.
     * The returned iterator must be deleted by the caller.
     */
    virtual HTMLIterator *getIterator();
  
    virtual const char * objectName() const { return "HTMLTable"; };

protected:

    void setCells( unsigned int r, unsigned int c, HTMLTableCell *cell );

    // 
    // A small set of helper functions for spreading width across columns
    void addColMinWidth(int k, int delta, ColType t);
    void addColsMinWidthEqual(int kol, int span, int tooAdd, ColType t);
    void addColsMinWidthVar(int kol, int span, int tooAdd, ColType t, int varCount);
    void addColsMinWidthNonFix(int kol, int span, int tooAdd, ColType t, int nonFixedCount);
    int  addColsMinWidthPref(int kol, int span, int tooAdd);
    void addColsPrefWidthEqual(int kol, int span, int tooAdd);
    void addColsPrefWidthVar(int kol, int span, int tooAdd, int varCount);
    void addColsPrefWidthNonFix(int kol, int span, int tooAdd, int nonFixedCount);

    //
    // This function calculates the minimum and prefered sizes of the table
    void calcColInfoI(void);

    //
    // This function calculates the actual widths of the columns
    void calcColInfoII(void);

    int  addColInfo(int _startCol, int _colSpan, int _minSize,
                    int _prefSize, ColType _colType, int defWidth = 0,
		    HTMLClue::HAlign halign = HTMLClue::HNone, 
		    HTMLClue::VAlign valign = HTMLClue::VNone );
    int getColInfo( int _startCol, int _colSpan = 1);

    void addRowInfo(int _row, int _colInfoIndex);
    void calcRowHeights();
    void addRows( int );
    void addColumns( int );

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
	int     defWidth; // width given by the parser 
	ColType colType;  // type of table column
	HTMLClue::HAlign halign;
	HTMLClue::VAlign valign;
    } ColInfo_t;
    /*
     * The RowInfo structs maintains relations between the various ColInfo
     * structs. Given the example-table given above we would have the 
     * following ColInfo's:
     *
     * ColInfo[0] : default (startCol = 0, colSpan = 3)
     * ColInfo[1] : A       (startCol = 0, colSpan = 1)
     * ColInfo[2] : B       (startCol = 1, colSpan = 1)
     * ColInfo[3] : C,E     (startCol = 2, colSpan = 1)
     * ColInfo[4] : D       (startCol = 1, colSpan = 2)
     *
     * The relation would be made using RowInfo structs:
     *
     * RowInfo[0] : entry[] = { 1,2,3 },  
     * RowInfo[1] : entry[] = { 4, 3 },
     *
     * { 1,2,3 } refers to ColInfo[1], ColInfo[2] and ColInfo[3]
     */
     
    HTMLTableCell ***cells;
    QArray<ColInfo_t> colInfo;
    HTMLRowInfo rowInfo;

    int         percent;
    int         fixed_width;
    int         min_width;
    int         pref_width;

    QArray<int> columnPos;
    QArray<int> columnPrefPos;
    QArray<int> rowHeights;
    QArray<ColType> colType;

    unsigned int totalColInfos;
    unsigned int col, totalCols;
    unsigned int row, totalRows, allocRows;
    int spacing;
    int padding;
    int border;
    HTMLClueV *caption;
    HTMLClue::VAlign capAlign;

    QColor tableColor;
    QColor tableRowColor;
    HTMLClue::VAlign rowvalign;
    HTMLClue::HAlign rowhalign;

    HTMLClue *oldclue;
    HTMLClue::HAlign olddivalign;
    int oldindent;

    bool progressive;
    bool finished;
    int maxColSpan;
};

//-----------------------------------------------------------------------------

#endif // HTMLTABLE_H

