/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

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
//-----------------------------------------------------------------------------
//
// KDE HTML Widget
//

#ifndef HTMLCLUE_H
#define HTMLCLUE_H

#include <qpainter.h>
#include <qlist.h>
#include <qstrlist.h>
#include <qarray.h>
#include <qpixmap.h>
#include <qstack.h>
#include <qfont.h>
#include <qtimer.h>

#include <kurl.h>

class HTMLClueV;
class HTMLImage;
class HTMLClue;
class HTMLClueFlow;
class HTMLClueAligned;
class KHTMLWidget;

#include "htmlfont.h"
#include "htmlobj.h"

// The border around an aligned object
#define ALIGN_BORDER 0

class HTMLAnchor;

//-----------------------------------------------------------------------------
// Clues are used to contain and format objects (or other clues).
// This is the base of all clues - it should be considered abstract.
//
class HTMLClue : public HTMLObject
{
public:
    /************************************************************
     * This class is abstract. Do not instantiate it. The _y argument
     * is always 0 yet. _max_width defines the width you allow this Box
     * to have. If you do not use HCenter or Right and if this Box
     * becomes a child of a VBox you may set _x to give this Box
     * a shift to the right.
     *
     * if:
     *     _percent == -ve     width = best fit
     *     _percent == 0       width = _max_width (fixed)
     *     _percent == +ve     width = _percent * 100 / _max_width
     */
    HTMLClue( int _x, int _y, int _max_width, int _percent = 100);
    virtual ~HTMLClue();

    virtual int  findPageBreak( int _y );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
	    int _height, int _tx, int _ty, bool toPrinter );
    /// Prints a special object only
    /**
     * This function is for example used to redraw an image that had to be
     * loaded from the world wide wait.
     */
    virtual void print( QPainter *_painter, HTMLObject *_obj, int _x, int _y,
	    int _width, int _height, int _tx, int _ty );
    virtual void print( QPainter *, int _tx, int _ty );
    /************************************************************
     * Calls all children and tells them to calculate their size.
     */
    virtual void calcSize( HTMLClue *parent = 0L );
    virtual void recalcBaseSize( QPainter * );
    virtual int  calcMinWidth();
    virtual int  calcPreferredWidth();
    virtual void setMaxAscent( int );
    virtual HTMLObject *checkPoint( int, int );
    virtual HTMLObject *mouseEvent( int, int, int, int );
    virtual void selectByURL( QPainter *,const char *,bool, int _tx, int _ty );
    virtual void select( QPainter *_painter, QRegExp& _pattern, bool _select,
	    int _tx, int _ty );
    virtual void select( QPainter *, bool, int _tx, int _ty );
    /*
     * Selects every objectsin this clue if it is inside the rectangle
     * and deselects it otherwise.
     */
    virtual void select( QPainter *, QRect &_rect, int _tx, int _ty );
    virtual void select( bool );
    virtual void getSelected( QStrList & );
    virtual bool selectText( QPainter *_painter, int _x1, int _y1,
			     int _x2, int _y2, int _tx, int _ty );
    virtual void getSelectedText( QString & );

    virtual void calcAbsolutePos( int _x, int _y );
    virtual void setIndent( int ) { }
    virtual void reset();

    virtual ObjectType getObjectType() const
	    {	return Clue; }
    bool hasChildren() const
	    {	return (head != 0L); }

    /************************************************************
     * Make an object a child of this Box.
     */
    void append( HTMLObject *_object )
	{
	    if ( !head )
	    {
		head = tail = _object;
	    }
	    else
	    {
		tail->setNext( _object );
		tail = _object;
	    }
	}
	
    virtual void findFreeArea( int _y, int _width, int _height, int _indent,
                               int *_y_pos, int *_lmargin, int *_rmargin)
        { *_y_pos = _y; *_lmargin = 0; *_rmargin = max_width; }
    // This method tries to find a free rectangular area of _width x _height
    // from position _y on. The start of this area is written in *y_pos.
    // The actual left and right margins of the area are returned in
    // *lmargin and *rmargin.
    virtual void appendLeftAligned( HTMLClueAligned * ) { }
    virtual void appendRightAligned( HTMLClueAligned * ) { }
    virtual int  appended(HTMLClueAligned * ) { return 0;}
    virtual void removeAlignedByParent( HTMLObject *p ) { }
    virtual int  getLeftMargin( int )
        { return 0; }
    virtual int  getRightMargin( int )
	{ return max_width; }
    virtual int  getLeftClear( int _y )
	{ return _y; }
    virtual int  getRightClear( int _y )
	{ return _y; }
    
    void setVAlign( VAlign _v ) { valign = _v; }
    void setHAlign( HAlign _h ) { halign = _h; }
    VAlign getVAlign() { return valign; }
    HAlign getHAlign() { return halign; }

    virtual HTMLAnchor* findAnchor( const char *_name, QPoint *_p );

    virtual void findCells( int _tx, int _ty, QList<HTMLCellInfo> &_list );
  
protected:
    HTMLObject *head;
    HTMLObject *tail;
    HTMLObject *curr;

    VAlign valign;
    HAlign halign;
};

//-----------------------------------------------------------------------------
// Used for aligning images etc. to the left or right of the page.
//
class HTMLClueAligned : public HTMLClue
{
public:
    HTMLClueAligned( HTMLClue *_parent, int _x, int _y, int _max_width,
		     int _percent = 100 )
	: HTMLClue( _x, _y, _max_width, _percent )
    { prnt = _parent; nextAligned = 0; setAligned( true ); }
    virtual ~HTMLClueAligned() { }
    
    virtual void setMaxWidth( int );
    virtual void setMaxAscent( int ) { }
    virtual void calcSize( HTMLClue *_parent = 0L );
    
    HTMLClue *parent()
	{ return prnt; }
    HTMLClueAligned *nextClue() const
	{ return nextAligned; }
    void setNextClue( HTMLClueAligned *n )
	{ nextAligned = n; }
    
private:
    HTMLClue *prnt;
    HTMLClueAligned *nextAligned;
};

//-----------------------------------------------------------------------------
// Align objects across the page, wrapping at the end of a line
//
class HTMLClueFlow : public HTMLClue
{
public:
    HTMLClueFlow( int _x, int _y, int _max_width, int _percent=100)
		: HTMLClue( _x, _y, _max_width, _percent ) { indent = 0; }
    virtual ~HTMLClueFlow() { }
    
    virtual bool selectText( QPainter *_painter, int _x1, int _y1,
	    int _x2, int _y2, int _tx, int _ty );
    virtual void getSelectedText( QString & );
    virtual void calcSize( HTMLClue *parent = 0L );
	virtual int  findPageBreak( int _y );
    virtual int  calcMinWidth();
    virtual int  calcPreferredWidth();
    virtual void setMaxWidth( int );

    virtual void setIndent( int i )
	    {	indent = i; }

protected:
    short indent;
};

//-----------------------------------------------------------------------------
// Align objects vertically
//
class HTMLClueV : public HTMLClue
{
public:
    HTMLClueV( int _x, int _y, int _max_width, int _percent = 100 );
    virtual ~HTMLClueV() { }

    virtual void reset();

    virtual void setMaxWidth( int );
    virtual HTMLObject *checkPoint( int, int );
    virtual HTMLObject *mouseEvent( int, int, int, int );
    virtual void calcSize( HTMLClue *parent = 0L );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
		int _height, int _tx, int _ty, bool toPrinter );
    virtual void print( QPainter *_painter, HTMLObject *_obj, int _x, int _y,
		int _width, int _height, int _tx, int _ty )
	{ HTMLClue::print(_painter,_obj,_x,_y,_width,_height,_tx,_ty); }

    virtual void findFreeArea( int _y, int _width, int _height, int _indent,
                               int *_y_pos, int *_lmargin, int *_rmargin);
    // This method tries to find a free rectangular area of _width x _height
    // from position _y on. The start of this area is written in *y_pos.
    // The actual left and right margins of the area are returned in
    // *lmargin and *rmargin.
                   
    virtual void appendLeftAligned( HTMLClueAligned *_clue );
    virtual void appendRightAligned( HTMLClueAligned *_clue );
    virtual int  appended(HTMLClueAligned * );
    virtual int  getLeftMargin( int _y );
    virtual int  getRightMargin( int _y );
    virtual int  getLeftClear( int _y );
    virtual int  getRightClear( int _y );
  
protected:
    void removeAlignedByParent( HTMLObject *p );

protected:
    // These are the objects which are left or right aligned within this
    // clue.  Child objects must wrap their contents around these.
    HTMLClueAligned *alignLeftList;
    HTMLClueAligned *alignRightList;
};

/**
 * Used for KFMs HTML extension
 */
class HTMLCell : public HTMLClueV
{
public:
  HTMLCell( int _x, int _y, int _max_width, int _percent = 100, const char *_url = 0L, const char *_target = 0L );
  virtual ~HTMLCell() { }
  
  virtual const char* getURL() const { return url; }
  virtual const char* getTarget() const { return target; }

  virtual void select( QPainter *, QRect &_rect, int _tx, int _ty );

  virtual bool print( QPainter *_painter, int _x, int _y, int _width,
		      int _height, int _tx, int _ty, bool toPrinter );

  virtual bool isMarked() { return bIsMarked; }
  virtual void setMarker( QPainter *_painter, int _tx, int _ty, bool _mode );
  virtual void findCells( int _tx, int _ty, QList<HTMLCellInfo> &_list );

protected:
  const char *url;
  const char *target;

  bool bIsMarked;
};

//-----------------------------------------------------------------------------
// Align objects across the page, without wrapping.
// This clue is required for lists, etc. so that tables can dynamically
// change max_width and have the contents' max_widths changed appropriately.
// Also used by <pre> lines
//
class HTMLClueH : public HTMLClue
{
public:
    HTMLClueH( int _x, int _y, int _max_width, int _percent = 100 )
	    : HTMLClue( _x, _y, _max_width, _percent ) { indent = 0; }
    virtual ~HTMLClueH() { }
	
    virtual bool selectText( QPainter *_painter, int _x1, int _y1,
	    int _x2, int _y2, int _tx, int _ty );
    virtual void setMaxWidth( int );
    virtual void calcSize( HTMLClue *parent = 0L );
    virtual int  calcMinWidth();
    virtual int  calcPreferredWidth();

    virtual void setIndent( int i )
	    {	indent = i; }

private:
    short indent;
};


//-----------------------------------------------------------------------------
// really only useful for tables.
//
class HTMLTableCell : public HTMLClueV
{
public:
    HTMLTableCell( int _x, int _y, int _max_width, int _percent,
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
    virtual int  calcMinWidth();
    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
		int _height, int _tx, int _ty, bool toPrinter );
    virtual void print( QPainter *_painter, HTMLObject *_obj, int _x, int _y,
		int _width, int _height, int _tx, int _ty )
	{ HTMLClueV::print(_painter,_obj,_x,_y,_width,_height,_tx,_ty); }

protected:
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
    HTMLTable( int _x, int _y, int _max_width, int _width, int _percent,
		int _padding = 1, int _spacing = 2, int _border = 0 );
    virtual ~HTMLTable();

    void startRow();
    void addCell( HTMLTableCell *cell );
    void endRow();
    void endTable();

    void setCaption( HTMLClueV *cap, HTMLClue::VAlign al )
	    {	caption = cap; capAlign = al; }

    virtual void reset();
    virtual void calcSize( HTMLClue *parent = 0L );
    virtual int  calcMinWidth();
    virtual int  calcPreferredWidth();
    virtual void setMaxWidth( int _max_width );
    virtual void setMaxAscent( int );
    virtual HTMLObject *checkPoint( int, int );
    virtual HTMLObject *mouseEvent( int, int, int, int );
    virtual void selectByURL( QPainter *,const char *, bool, int _tx, int _ty );
    virtual void select( QPainter *_painter, QRegExp& _pattern, bool _select,
	    int _tx, int _ty );
    virtual void select( QPainter *, bool, int _tx, int _ty );
    /**
     * Selects every object in this table if it is inside the rectangle
     * and deselects it otherwise.
     */
    virtual void select( QPainter *, QRect &_rect, int _tx, int _ty );
    virtual void select( bool );
    virtual bool selectText( QPainter *_painter, int _x1, int _y1,
	    int _x2, int _y2, int _tx, int _ty );
    virtual void getSelected( QStrList & );
    virtual void getSelectedText( QString & );

    virtual void calcAbsolutePos( int _x, int _y );

    virtual ObjectType getObjectType() const
	    {	return Clue; }

    virtual HTMLAnchor *findAnchor( const char *_name, QPoint *_p );

    virtual int  findPageBreak( int _y );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
    	    int _height, int _tx, int _ty, bool toPrinter );
    virtual void print( QPainter *_painter, HTMLObject *_obj, int _x, int _y,
	    int _width, int _height, int _tx, int _ty );
    virtual void print( QPainter *, int _tx, int _ty );

    virtual void findCells( int _tx, int _ty, QList<HTMLCellInfo> &_list );
  
protected:
    void setCells( unsigned int r, unsigned int c, HTMLTableCell *cell );
    void calcColumnWidths();
    void optimiseCellWidth();
    void scaleColumns(unsigned int c_start, unsigned int c_end, int tooAdd);
    void calcRowHeights();
    void addRows( int );
    void addColumns( int );

protected:
    enum ColType { Fixed, Percent, Variable };
    HTMLTableCell ***cells;
    QArray<int> columnPos;
    QArray<int> columnPrefPos;
    QArray<int> columnOpt;
    QArray<int> colSpan;
    QArray<int> rowHeights;
    QArray<ColType> colType;
    unsigned int col, totalCols;
    unsigned int row, totalRows, allocRows;
    int spacing;
    int padding;
    int border;
    HTMLClueV *caption;
    HTMLClue::VAlign capAlign;
};

//-----------------------------------------------------------------------------

#endif // HTMLCLUE_H

