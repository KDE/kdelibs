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
    HTMLTableCell( int _percent, int _width, int rs, int cs, int pad );
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

    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
		int _height, int _tx, int _ty, bool toPrinter );
    virtual void print( QPainter *_painter, HTMLChain *_chain, int _x,
	int _y, int _width, int _height, int _tx, int _ty );
    virtual void print( QPainter *_painter, HTMLObject *_obj, int _x, int _y,
		int _width, int _height, int _tx, int _ty )
	{ HTMLClueV::print(_painter,_obj,_x,_y,_width,_height,_tx,_ty); }

    virtual const char * objectName() const { return "HTMLTableCell"; };

protected:
    int percent;
    int rspan;
    int cspan;
    int padding;
    QColor bg;
};

//-----------------------------------------------------------------------------
//
class HTMLTable : public HTMLObject
{
public:
    HTMLTable( int _percent, int _width,
		int _padding = 1, int _spacing = 2, int _border = 0 );
    virtual ~HTMLTable();

    void startRow();
    void addCell( HTMLTableCell *cell );
    void endRow();
    void endTable();

    void setCaption( HTMLClueV *cap, HTMLClue::VAlign al )
	    {	caption = cap; capAlign = al; }

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

    virtual ObjectType getObjectType() const
	    {	return Clue; }

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

    /*
     * Create an iterator.
     * The returned iterator must be deleted by the caller.
     */
    virtual HTMLIterator *getIterator();
  
    virtual const char * objectName() const { return "HTMLTable"; };

protected:
    enum ColType { Fixed, Percent, Variable };

    void setCells( unsigned int r, unsigned int c, HTMLTableCell *cell );
    void calcColInfo(int pass);
    int  addColInfo(int _startCol, int _colSpan, int _minSize,
                    int _prefSize, ColType _colType);
    void addRowInfo(int _row, int _colInfoIndex);
    void optimiseCellWidth();
    void scaleColumns(unsigned int c_start, unsigned int c_end, int tooAdd);
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
       ColType colType;
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
    typedef struct RowInfo_struct
    {
       int *entry;
       int  nrEntries;
       int  minSize;
       int  prefSize;
    } RowInfo_t;
    
    HTMLTableCell ***cells;
    QArray<ColInfo_t> colInfo;
    RowInfo_t   *rowInfo;

    int         percent;
    int         fixed_width;
    int         min_width;
    int         pref_width;

    QArray<int> columnPos;
    QArray<int> columnPrefPos;
    QArray<int> columnOpt;
    QArray<int> colSpan;
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
};

//-----------------------------------------------------------------------------

#endif // HTMLTABLE_H

