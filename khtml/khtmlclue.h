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
// KDE HTML Widget -- Clues
// $Id$

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

#include "khtmlobj.h"

//
// External Classes
//
///////////////////

class KHTMLWidget;
class HTMLIterator;


//
// Internal Classes
//
///////////////////

class HTMLClueV;
class HTMLClue;
class HTMLClueFlow;
class HTMLClueAligned;


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
    /*
     * This class is abstract. Do not instantiate it. 
     *
     */
    HTMLClue( );
    virtual ~HTMLClue();

    virtual int  findPageBreak( int _y );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
	    int _height, int _tx, int _ty, bool toPrinter );
    virtual void print( QPainter *_p, HTMLChain *, int _x, int _h, int _w,
	    int _h, int _tx, int _ty);
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

    void setFixedWidth( int _width);
    virtual int  calcMinWidth();
    virtual int  calcPreferredWidth();
    virtual void setMaxWidth( int );
    virtual void setMaxAscent( int );
    virtual void setMaxDescent( int );
    virtual HTMLObject *checkPoint( int, int );
    virtual HTMLObject *mouseEvent( int, int, int, int );
    virtual void selectByURL( KHTMLWidget *, HTMLChain *,const char *,bool, int _tx, int _ty );
    virtual void select( KHTMLWidget *, HTMLChain *, QRegExp& _pattern, bool _select,
	    int _tx, int _ty );
    virtual void select( KHTMLWidget *, HTMLChain *, bool, int _tx, int _ty );
    /*
     * Selects every objectsin this clue if it is inside the rectangle
     * and deselects it otherwise.
     */
    virtual void select( KHTMLWidget *, HTMLChain *, QRect &_rect, int _tx, int _ty );
    virtual void select( bool );
    virtual void getSelected( QStrList & );
    virtual bool selectText( KHTMLWidget *_htmlw, HTMLChain *_chain, int _x1,
	int _y1, int _x2, int _y2, int _tx, int _ty );
    virtual void getSelectedText( QString & );

    virtual void calcAbsolutePos( int _x, int _y );
    virtual bool getObjectPosition( const HTMLObject *objp, int &xp, int &yp );
    virtual void setIndent( int ) { }
    virtual void reset();

    virtual ObjectType getObjectType() const
	    {	return Clue; }
    bool hasChildren() const
	    {	return (head != 0L); }
    HTMLObject *children() { return head; }
	HTMLObject *lastChild() { return tail; }
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
	
    virtual void findFreeArea( int _y, int, int, int,
                               int *_y_pos, int *_lmargin, int *_rmargin)
        { *_y_pos = _y; *_lmargin = 0; *_rmargin = width; }
    // This method tries to find a free rectangular area of _width x _height
    // from position _y on. The start of this area is written in *y_pos.
    // The actual left and right margins of the area are returned in
    // *lmargin and *rmargin.
    virtual void appendLeftAligned( HTMLClueAligned * ) { }
    virtual void appendRightAligned( HTMLClueAligned * ) { }
    virtual int  appended(HTMLClueAligned * ) { return 0;}
    virtual void removeAlignedByParent( HTMLObject * ) { }
    virtual int  getLeftMargin( int )
        { return 0; }
    virtual int  getRightMargin( int )
	{ return width; }
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

    /*
     * Create an iterator.
     * The returned iterator must be deleted by the caller.
     */
    virtual HTMLIterator *getIterator();

    virtual const char * objectName() const { return "HTMLClue"; };

    virtual void printDebug( bool propagate = false, int indent = 0, 
			     bool printObjects = false );
  
protected:
    int fixed_width;
    int min_width;
    
    HTMLObject *head;
    HTMLObject *tail;
    HTMLObject *curr;

    VAlign valign;
    HAlign halign;
};

//-----------------------------------------------------------------------------
// Align objects vertically
//
// Sizes supported: Variable, Fixed
// Sizes unsupported: Percentage
class HTMLClueV : public HTMLClue
{
public:
    HTMLClueV();
    virtual ~HTMLClueV() { }

    virtual void reset();

    virtual HTMLObject *checkPoint( int, int );
    virtual HTMLObject *mouseEvent( int, int, int, int );
    virtual void calcSize( HTMLClue *parent = 0L );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
		int _height, int _tx, int _ty, bool toPrinter );
    virtual void print( QPainter *_painter, HTMLObject *_obj, int _x, int _y,
		int _width, int _height, int _tx, int _ty )
	{ HTMLClue::print(_painter,_obj,_x,_y,_width,_height,_tx,_ty); }

    virtual void print( QPainter *, int _tx, int _ty );
    
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
  
    virtual const char * objectName() const { return "HTMLClueV"; };
    virtual void printDebug( bool propagate, int indent, bool printObjects );

protected:
    void removeAlignedByParent( HTMLObject *p );

protected:
    // These are the objects which are left or right aligned within this
    // clue.  Child objects must wrap their contents around these.
    HTMLClueAligned *alignLeftList;
    HTMLClueAligned *alignRightList;
};

//-----------------------------------------------------------------------------
// Used for aligning images etc. to the left or right of the page.
//
// Sizes supported: Variable
// Sizes unsupported: Fixed, Percentage
class HTMLClueAligned : public HTMLClueV
{
public:
    HTMLClueAligned( HTMLClue *_parent )
	: HTMLClueV()
    { prnt = _parent; nextAligned = 0; setAligned( true ); }
    virtual ~HTMLClueAligned() { }
    
    virtual void setMaxAscent( int ) { }
    virtual void calcSize( HTMLClue *_parent = 0L );
    
    HTMLClue *parent()
	{ return prnt; }
    HTMLClueAligned *nextClue() const
	{ return nextAligned; }
    void setNextClue( HTMLClueAligned *n )
	{ nextAligned = n; }

    virtual const char * objectName() const { return "HTMLClueAligned"; };

private:
    HTMLClue *prnt;
    HTMLClueAligned *nextAligned;
};

//-----------------------------------------------------------------------------
// Align objects across the page, wrapping at the end of a line
//
// Sizes supported: Variable
// Sizes unsupported: Fixed, Percentage
class HTMLClueFlow : public HTMLClue
{
public:
    HTMLClueFlow()
		: HTMLClue() { indent = 0; }
    virtual ~HTMLClueFlow() { }
    
    virtual bool selectText( KHTMLWidget *_htmlw, HTMLChain *_chain, int _x1,
	int _y1, int _x2, int _y2, int _tx, int _ty );
    virtual void getSelectedText( QString & );
    virtual void calcSize( HTMLClue *parent = 0L );
	virtual int  findPageBreak( int _y );
    virtual int  calcMinWidth();
    virtual int  calcPreferredWidth();

    virtual void setIndent( int i )
	    {	indent = i; }

    virtual const char * objectName() const { return "HTMLClueFlow"; };

protected:
    short indent;
};


//-----------------------------------------------------------------------------
// Used for KFMs HTML extension
//
// Sizes supported: Variable, Fixed
// Sizes unsupported: Percentage
class HTMLCell : public HTMLClueV
{
public:
  HTMLCell( const char *_url = 0L, const char *_target = 0L );
  virtual ~HTMLCell() { }
  
  virtual const char* getURL() const { return url; }
  virtual const char* getTarget() const { return target; }

  virtual void select( KHTMLWidget *, HTMLChain *, QRect &_rect, int _tx, int _ty );

  virtual bool print( QPainter *_painter, int _x, int _y, int _width,
		      int _height, int _tx, int _ty, bool toPrinter );

  virtual bool isMarked() { return bIsMarked; }
  virtual void setMarker( QPainter *_painter, int _tx, int _ty, bool _mode );
  virtual void findCells( int _tx, int _ty, QList<HTMLCellInfo> &_list );

  virtual const char * objectName() const { return "HTMLCell"; };

protected:
  const char *url;
  const char *target;

  bool bIsMarked;
};

//-----------------------------------------------------------------------------
// Align objects across the page, without wrapping.
// This clue is required for lists, etc. so that tables can dynamically
// change width and have the contents' widths changed appropriately.
// Also used by <pre> lines
//
// Sizes supported: Variable
// Sizes unsupported: Fixed, Percentage
class HTMLClueH : public HTMLClue
{
public:
    HTMLClueH()
	    : HTMLClue() { indent = 0; }
    virtual ~HTMLClueH() { }
	
    virtual bool selectText( KHTMLWidget *_htmlw, HTMLChain *_chain, int _x1,
	int _y1, int _x2, int _y2, int _tx, int _ty );
    virtual void calcSize( HTMLClue *parent = 0L );
    virtual int  calcMinWidth();
    virtual int  calcPreferredWidth();

    virtual void setIndent( int i )
	    {	indent = i; }

    virtual const char * objectName() const { return "HTMLClueH"; };

private:
    short indent;
};

//-----------------------------------------------------------------------------

#endif // HTMLCLUE_H
