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
// KDE HTML Widget -- Objects
// $Id$

#ifndef HTMLOBJ_H
#define HTMLOBJ_H

#include <stdio.h>

#include <qpainter.h>
#include <qlist.h>
#include <qstringlist.h>
#include <qarray.h>
#include <qpixmap.h>
#include <qstack.h>
#include <qfont.h>
#include <qtimer.h>

#include <kurl.h>
#include <kallocator.h>
#include "khtmlstring.h"

//
// External Classes
//
///////////////////

class HTMLFont;
class HTMLImage;
class HTMLClue;
class HTMLClueV;
class HTMLClueFlow;
class HTMLClueAligned;
class HTMLChain;
class HTMLIterator;
class KHTMLWidget;
class HTMLAnchor;
class HTMLCell;

/**
 * Used by @ref HTMLObject::findCells
 */
class HTMLCellInfo
{
public:
  HTMLCell *pCell;
  int xAbs;
  int baseAbs;
  int tx;
  int ty;
};

class HTMLAllocator : public KZoneAllocator
{
public:
    HTMLAllocator(long _blockSize = 128*1024) 
    	: KZoneAllocator( _blockSize ) { };
    HTMLString newString( const QString &str );
};


typedef enum { HTMLNoFit, HTMLPartialFit, HTMLCompleteFit } HTMLFitType;

const int UNDEFINED = -1;

//-----------------------------------------------------------------------------
// HTMLObject is the base class for all HTML objects.
//
class HTMLObject
{
public:
    static void * operator new(size_t _size, HTMLAllocator *allocator)
    { return allocator->allocate(_size); }
    static void operator delete(void *) { /* empty! */ }
    
    HTMLObject();
    virtual ~HTMLObject() { objCount--; }

    enum VAlign { Bottom=0, VNone=0, VCenter=1, Top=2 };
    enum HAlign { Left, HCenter, Right, HNone };

    /**
     * This function should cause the HTMLObject to calculate its
     * width and height.
     */
    virtual void calcSize( HTMLClue * = 0L ) { }

    /**
     * This function should cause the HTMLObject to break itself up so 
     * that it will fit within the given length. Only usefull for text.
     * If 'startOfLine' is 'false', this function may return 'HTMLNoFit' to 
     * indicate it is not possible to use the specified 'widthLeft'.
     */
    virtual HTMLFitType fitLine( bool , 
				 bool , 
				 int ) 
    { return HTMLCompleteFit; }
    
    /**
     * This function forces a size calculation for objects which
     * calculate their size at construction.  This is useful if
     * the metrics of the painter change, e.g. if the html is to
     * be printed on a printer instead of the display.
     */
    virtual void recalcBaseSize( QPainter * ) { }

    /**
     * This function calculates the minimum width that the object
     * can be set to. (added for table support)
     */
    virtual int  calcMinWidth() { return width; }

    /**
     * This function calculates the width that the object would like
     * to be. (added for table support)
     */
    virtual int  calcPreferredWidth() { return width; }

    virtual void setMaxAscent( int ) { }
    virtual void setMaxDescent( int ) { }
    virtual void setMaxWidth( int ) { }
    virtual int  findPageBreak( int _y );

    /**
     * Print the object but only if it fits in the rectangle given
     * by _x,_y,_width,_ascender. (_x|_y) is the lower left corner relative
     * to the parent of this object ( usually HTMLClue ).
     */
    virtual bool print( QPainter *, int, int, int, int, int, int, bool )
	{ return false; }

    virtual void print( QPainter *, HTMLObject *, int, int, int, int, int, int )
	{}

    virtual void print( QPainter *_p, HTMLChain *, int _x, int _y, int _w,
	    int _h, int _tx, int _ty)
	{ print( _p, _x, _y, _w, _h, _tx, _ty, false ); }
    /**
     * Print the object.
     */
    virtual void print( QPainter *, int, int ) { }

    virtual HTMLObject *checkPoint( int, int );
    virtual HTMLObject *mouseEvent( int, int, int, int ) { return 0; }
    virtual void selectByURL(KHTMLWidget *, HTMLChain *, QString, bool,
	int _tx, int _ty);
    virtual void select( KHTMLWidget *, HTMLChain *, bool, int _tx, int _ty );

    /**
     * Selects the object if it is inside the rectangle and deselects it
     * otherwise.
     */
    virtual void select( KHTMLWidget *, HTMLChain *, QRect &_rect, int _tx,
	int _ty );

    /**
     * Select all objects matching the regular expression.
     */
    virtual void select( KHTMLWidget *, HTMLChain *, QRegExp& _pattern,
	bool _select, int _tx, int _ty );

    virtual void select( bool _s ) { setSelected( _s ); }

    /**
     * select text.  returns whether any text was selected.
     */
    virtual bool selectText( KHTMLWidget *_htmlw, HTMLChain *_chain, int _x1,
	int _y1, int _x2, int _y2, int _tx, int _ty );

    virtual void getSelected( QStringList & );
    virtual void getSelectedText( QString & ) {}

    /**
     * Some objects may need to know their absolute position on the page.
     */
    virtual void calcAbsolutePos( int, int ) { }

    virtual int  getAbsX() { return -1; }
    virtual int  getAbsY() { return -1; }

    /**
     * returns the postion on the page of the specified object
     */
    virtual bool getObjectPosition( const HTMLObject *obj, int &x, int &y );

    virtual void reset() { setPrinted( false ); }


    /**
     * This function is called from the imagecache, when it can deliver
     * an image requested with htmlWidget->requestImage()
     */
    virtual void setPixmap( QPixmap * ) { }

    /** 
     * This function is called, whenever the pixmap changes (animated gifs, 
     * etc.). If pixmap = 0, just the content of the pixmap changed.
     * In this case, the object just needs to call 
     * htmlWidget->paintSingleObject(this).
     */
    virtual void pixmapChanged( QPixmap * = 0 ) { }

    enum ObjectType { Object, Clue };

    /**
     * sometimes a clue would like to know if an object is a 
     * clue or a basic object.
     */
    virtual ObjectType getObjectType() const
	    {	return Object; }

    /**
     * Get X-Position of this object relative to its parent
     */
    int getXPos() const { return x; }

    /**
     * Get Y-Position of this object relative to its parent
     */
    int getYPos() const { return y; }

    int getWidth() const { return width; }
    int getHeight() const { return ascent+descent; }
    int getAscent() const { return ascent; }
    int getDescent() const { return descent; }

    /**
     * return the URL associated with this object if available, else 0.
     */
    virtual HTMLString getURL() const { return 0; }
    virtual HTMLString getTarget() const { return 0; }

    void setPos( int _x, int _y ) { y=_y; x=_x; }
    void setXPos( int _x ) { x=_x; }
    void setYPos( int _y ) { y=_y; }

    enum ObjectFlags { Separator = 0x01, NewLine = 0x02, 
                       Selected = 0x04, AllSelected = 0x08, 
                       Aligned = 0x10, Printed = 0x20, 
                       Hidden = 0x40};

    bool isSeparator() const { return flags & Separator; }
    bool isNewline() const { return flags & NewLine; }
    bool isSelected() const { return flags & Selected; }
    bool isAllSelected() const { return flags & AllSelected; }
    bool isHAligned() const { return flags & Aligned; }
    bool isPrinted() const { return flags & Printed; }
    bool isHidden() const { return flags & Hidden; }
    virtual VAlign isVAligned() const { return VNone; }
    virtual bool isSlave() const { return 0; }
    
    void setSeparator( bool s ) { s ? flags|=Separator : flags&=~Separator; }
    void setNewline( bool n ) { n ? flags|=NewLine : flags&=~NewLine; }
    void setSelected( bool s ) { s ? flags|=Selected : flags&=~Selected; }
    void setAllSelected( bool s ) { s ? flags|=AllSelected : flags&=~AllSelected; }
    void setAligned( bool a ) { a ? flags|=Aligned : flags&=~Aligned; }
    void setPrinted( bool p ) { p ? flags|=Printed : flags&=~Printed; }
    void setHidden( bool p ) { p ? flags|=Hidden : flags&=~Hidden; }
    
    /**
     * Searches in all children ( and tests itself ) for an HTMLAnchor object
     * with the name '_name'. Returns 0L if the anchor could not be found.
     * '_point' is modified so that it holds the position of the anchor relative
     * to the parent.
     */
    virtual HTMLAnchor* findAnchor( QString /*_name*/, int &/* x */, int &/* y*/)
			{ return 0L; }

    /**
     * All objects can be an element in a list and maintain a pointer to
     * the next object.
     */
    void setNext( HTMLObject *n ) { nextObj = n; }
    HTMLObject *next() const { return nextObj; }

    void printCount() { printf( "Object count: %d\n", objCount ); }

    virtual void findCells( int, int, QList<HTMLCellInfo> & ) { }

    /**
     * Create an iterator.
     * The returned iterator must be deleted by the caller.
     */
    virtual HTMLIterator *getIterator() { return 0; }

    /**
     * Select this object's text if it matches.
     * returns true if a match was found.
     */
    virtual bool selectText( const QRegExp & ) { return false; }

    //
    // functions mainly used for debugging
    //
    //////////////////////////////////////

    /**
     * returns the actual class name of the html object
     */
    virtual const char * objectName() const = 0;// { return "HTMLObject"; }
    // the above declaration makes HTMLObject pure virtual...

    /**
     * prints debug info to stdout
     */
    virtual void printDebug( bool propagate = false, int indent = 0, 
			     bool printObjects = false );
    
protected:
    int x;
    int y;
    int ascent;
    int descent;
    short width;	// Actual width of object
    short flags;
    HTMLObject *nextObj;
    static int objCount;
};

class HTMLFileRequester 
{
public:
    /********************************
     * These two functions are overloaded by objects that need to have a remote
     * file downloaded, e.g. HTMLMap.
     *
     * fileLoaded is called when the requested file has arrived.
     */
    virtual void fileLoaded( QString /*_url*/, 
			     QString /*localfile*/ ) = 0;
    virtual bool fileLoaded( QString /* _url */, QBuffer& /* _buffer */, 
			     bool /* eof */ = false ) = 0;
};

//-----------------------------------------------------------------------------
// There are several text-related objects:
//
// HTMLHSpace: A horizontal space
// HTMLVSpace: A vertical space, e.g. linefeed
// HTMLText: A non-breakable text object
// HTMLTextMaster: A breakable text-object
// HTMLLinkText: A non-breakable hyperlinked text object
// HTMLLinkTextMaster: A breakable hyperlinked text object
//
// Use:
//    HTMLHSpace is equivalent to HTMLText(" ", ...) but slightly smaller
//               in memory usage
//    HTMLVSpace is used for a forced line-break (e.g. linefeed)
//    HTMLText is used for text which shouldn't be broken. 
//    HTMLTextMaster is used for text which may be broken on spaces,
//               it should only be used inside HTMLClueFlow.
//               For text without spaces HTMLTextMaster is equivalent
//               to HTMLText. In such cases HTMLText is more efficient.
//    HTMLLinkText is like HTMLText but can be hyperlinked.
//    HTMLLinkTextMaster is like HTMLTextMaster but can be hyperlinked.
//
// Rationale:
//    Basically all functionality is provided by HTMLVSpace and HTMLText.
//    The additional functionality of HTMLLLinkText is not put in HTMLText
//    to keep the memory usage of the frequently used HTMLText object low.
//    Since often single spaces are used in HTML, they got their own, even 
//    smaller object. 
//    Another often encountered pattern is a paragraph of text. The 
//    HTMLTextMaster is designed for this purpose. It splits the paraagraph
//    in lines during layout and allocates a HTMLTextSlave object for each 
//    line. The actual text itself is maintained by the HTMLTextMaster
//    object making efficient memory usage possible.

class HTMLHSpace : public HTMLObject
{
public:

    HTMLHSpace( const HTMLFont *, QPainter *, bool hidden=false );
    virtual ~HTMLHSpace() { }
    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
	    int _height, int _tx, int _ty, bool toPrinter );
    virtual void print( QPainter *, int _tx, int _ty );

    virtual void recalcBaseSize( QPainter *_painter );
    virtual void getSelectedText( QString & );

    virtual const char * objectName() const { return "HTMLHSpace"; }

protected:
    const HTMLFont *font;
};
//-----------------------------------------------------------------------------

class HTMLTextSlave;
class HTMLText : public HTMLObject
{
	friend HTMLTextSlave;
public:
    HTMLText( HTMLString, const HTMLFont *, QPainter *);
    HTMLText( const HTMLFont *, QPainter * );
    virtual ~HTMLText();

    virtual bool selectText( KHTMLWidget *_htmlw, HTMLChain *_chain, int _x1,
	int _y1, int _x2, int _y2, int _tx, int _ty );
    virtual void getSelectedText( QString & );
    virtual void recalcBaseSize( QPainter *_painter );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
	    int _height, int _tx, int _ty, bool toPrinter );
    virtual void print( QPainter *, int _tx, int _ty );

    virtual bool selectText( const QRegExp &exp );

    virtual const char * objectName() const { return "HTMLText"; }
    virtual void printDebug( bool propagate = false, int indent = 0, 
			     bool printObjects = false );

protected:
    int getCharIndex( int _xpos );

protected:
    HTMLString text;
    const HTMLFont *font;
    short selStart;
    short selEnd;
};

class HTMLTextMaster : public HTMLText
{
	friend HTMLTextSlave;
public:
    HTMLTextMaster( HTMLString _text, const HTMLFont *_font, 
    				QPainter *_painter);
    	             
    virtual int  calcMinWidth() { return minWidth; }
    virtual int  calcPreferredWidth() { return prefWidth; }
    virtual HTMLFitType fitLine( bool startOfLine, bool firstRun, int widthLeft );
    virtual bool print( QPainter *, int , int , int ,
					    int , int , int , bool )
	    { return false; } // Dummy
    virtual void print( QPainter *, int , int )
    	{ } // Dummy
    virtual void recalcBaseSize( QPainter * ) 
        { } // Dummy
    virtual bool selectText( KHTMLWidget *, HTMLChain *, int ,
				 int , int , int , int , int  )
		{ return false; } // Dummy
    virtual bool selectText( const QRegExp & ) { return false; }

    virtual const char * objectName() const { return "HTMLTextMaster"; }

protected:
	int minWidth;
	int prefWidth;  
	int strLen;  
};

class HTMLTextSlave : public HTMLObject
{
public:
    static void * operator new(size_t _size)
    { return malloc(_size); }
    static void operator delete(void *p) { free(p); }
    HTMLTextSlave( HTMLTextMaster *_owner, short _posStart, short _posLen);
    virtual HTMLFitType fitLine( bool startOfLine, bool firstRun, int widthLeft );
    virtual bool selectText( KHTMLWidget *_htmlw, HTMLChain *_chain, int _x1,
			     int _y1, int _x2, int _y2, int _tx, int _ty );
    virtual void getSelectedText( QString & ) { } // Handled by master
    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
	    int _height, int _tx, int _ty, bool toPrinter );
    virtual void print( QPainter *, int _tx, int _ty );
    virtual int  calcMinWidth() { return 0; }
    virtual int  calcPreferredWidth() { return 0; }

    virtual bool selectText( const QRegExp &exp );
    virtual bool isSlave() const { return 1; }

    virtual HTMLString getURL() const { return owner->getURL(); }
    virtual HTMLString getTarget() const { return owner->getTarget(); }

    virtual const char * objectName() const { return "HTMLTextSlave"; }

protected:
    int getCharIndex( int _xpos );

protected:
    HTMLTextMaster *owner;
    short posStart;
    short posLen;
};

//-----------------------------------------------------------------------------
//
// This object is text which also has an associated link.  This data is
// not maintained in HTMLText to conserve memory.
//
class HTMLLinkText : public HTMLText
{
public:
    HTMLLinkText( HTMLString _str, const HTMLFont *_font, QPainter *_painter,
	    HTMLString _url, HTMLString _target)
	: HTMLText( _str, _font, _painter)
	    { url = _url; target = _target; }
    virtual ~HTMLLinkText() { }

    virtual HTMLString getURL() const { return url; }
    virtual HTMLString getTarget() const { return target; }

    virtual const char * objectName() const { return "HTMLLinkText"; };

protected:
    HTMLString url;
    HTMLString target;
};

//-----------------------------------------------------------------------------
//
// This object is text which also has an associated link.  This data is
// not maintained in HTMLTextMaster to conserve memory.
//
class HTMLLinkTextMaster : public HTMLTextMaster
{
public:
    HTMLLinkTextMaster( HTMLString _str, const HTMLFont *_font, QPainter *_painter,
			HTMLString _url, HTMLString _target)
	: HTMLTextMaster( _str, _font, _painter)
	    { url = _url; target = _target; }
    virtual ~HTMLLinkTextMaster() { }

    virtual HTMLString getURL() const { return url; }
    virtual HTMLString getTarget() const { return target; }

    virtual const char * objectName() const { return "HTMLLinkTextMaster"; };

protected:
    HTMLString url;
    HTMLString target;
};

//-----------------------------------------------------------------------------

class HTMLRule : public HTMLObject
{
public:
    HTMLRule( int _length=UNDEFINED, int _percent=UNDEFINED, int _size=1, bool _shade=TRUE );

	// This object supports HTMLFixedWidth and HTMLPercentWidth
    virtual int  calcMinWidth();
    virtual int  calcPreferredWidth() { return calcMinWidth(); }
    virtual void setMaxWidth( int );

    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
	int _height, int _tx, int _ty, bool toPrinter );
    virtual void print( QPainter *, int _tx, int _ty );

    virtual const char * objectName() const { return "HTMLRule"; };

protected:
	int  length;
	int  percent;
    bool shade;
};

//-----------------------------------------------------------------------------

class HTMLBullet : public HTMLObject
{
public:
    HTMLBullet( int _height, int _level, const QColor &col );
    virtual ~HTMLBullet() { }

    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
	int _height, int _tx, int _ty, bool toPrinter );
    virtual void print( QPainter *, int _tx, int _ty );

    virtual const char * objectName() const { return "HTMLBullet"; };

protected:
    char level;
    QColor color;
};

//-----------------------------------------------------------------------------

class HTMLVSpace : public HTMLObject
{
public:
    enum Clear { CNone, Left, Right, All };

    HTMLVSpace( int _vspace, Clear c = CNone );
    virtual ~HTMLVSpace() { }

    virtual void getSelectedText( QString & );

    Clear clear()
	{ return cl; }

    virtual const char * objectName() const { return "HTMLVSpace"; };

private:
    Clear cl;
};

//-----------------------------------------------------------------------------
// This class is inserted everywhere where a <a name="anchor">
// appears.
//
class HTMLAnchor : public HTMLObject
{
public:
    HTMLAnchor( HTMLString _name ) : name( _name ) {}
    virtual ~HTMLAnchor() { }

    const QString getName() { return name; }

    virtual VAlign isVAligned( void ) { return Top; }
    virtual HTMLAnchor* findAnchor( QString _name, int &_x, int &_y );

    virtual const char * objectName() const { return "HTMLAnchor"; };

protected:
    HTMLString name;
};

//-----------------------------------------------------------------------------

class HTMLImage : public HTMLObject
{
public:
    HTMLImage( KHTMLWidget *widget, HTMLString /*ImageURL*/, HTMLString _url,
		HTMLString _target, 
		int _width = UNDEFINED, int _height = UNDEFINED, 
		int _percent = UNDEFINED, int bdr = 0 );
    virtual ~HTMLImage();

    // This object supports HTMLFixedWidth and HTMLPercentWidth
    virtual int  calcMinWidth();
    virtual int  calcPreferredWidth();
    virtual void setMaxWidth( int );
    void setVAlign( VAlign _v ) { valign = _v; }
    virtual VAlign isVAligned() const { return valign; }

    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
	int _height, int _tx, int _ty, bool toPrinter );
    virtual void print( QPainter *, int _tx, int _ty );

    virtual void calcAbsolutePos( int _x, int _y );
    virtual int  getAbsX();
    virtual int  getAbsY();

    // ####### seems to be unused, but possibly useful for jscript
    virtual void changeImage( HTMLString _url );

    virtual void setPixmap( QPixmap * );
    virtual void pixmapChanged( QPixmap * = 0 );

    void setOverlay( HTMLString ol );

    virtual HTMLString  getURL() const { return url; }
    virtual HTMLString  getTarget() const { return target; }

    void setBorderColor( const QColor &color )
	{   borderColor = color; }

    virtual const char * objectName() const { return "HTMLImage"; };
    virtual void printDebug( bool, int indent, bool printObjects );

protected:

    int percent;
    /*
     * The desired dimensions of the image. If -1, the actual image will
     * determine this value when it is loaded.
     */
    int predefinedWidth;
    int predefinedHeight;

    /*
     * border width
     */
    int border;

    QColor borderColor;

    /*
     * The dimensions of the image during the last 'setMaxWidth'.
     * If a new image is set with different dimensions, we need to 
     * recalculate the layout.
     */
    int lastWidth;
    int lastHeight;
 
    /*
     * Some stuff for vertical alignment
     */
    VAlign valign; 
	
    /*
     * Pointer to the image
     * If this pointer is 0L, that means that the picture could not be loaded
     * for some strange reason or that the image is waiting to be downloaded
     * from the internet for example.
     */
    QPixmap *pixmap;

    // this image is overlaid on top of the image.
    // Used by HTML widget to show links/read only files, etc.
    QPixmap *overlay;

    /**
     * The URL of this image.
     */
    HTMLString imageURL;
    
    KHTMLWidget *htmlWidget;
    
    /*
     * The URL this image points to 
     */
    HTMLString url;
    HTMLString target;

    // The absolute position of this object on the page
    int absX;
    int absY;

    bool bComplete;
};

//----------------------------------------------------------------------------
/*
 * HTMLArea holds an area as specified by the <AREA > tag.
 */
class HTMLArea
{
public:
    HTMLArea( const QPointArray &_points, HTMLString _url,
	    HTMLString _target = 0 );
    HTMLArea( const QRect &_rect, HTMLString _url, 
	      HTMLString _target = 0 );
    HTMLArea( int _x, int _y, int _r, HTMLString _url,
	      HTMLString _target = 0 );

    enum Shape { Poly, Rect, Circle };

    bool contains( const QPoint &_point ) const
	    {	return region.contains( _point ); }

    HTMLString getURL() const
	    {	return url; }
    HTMLString getTarget() const
	    {	return target; }

protected:
    QRegion region;
    HTMLString url;
    HTMLString target;
};

//----------------------------------------------------------------------------

/*
 * HTMLMap contains a list of areas in the image map.
 * i.e. all areas between <MAP > </MAP>
 * This object is derived from HTMLObject so that it can make use of
 * URLLoaded().
 */
class HTMLMap : public HTMLFileRequester
{
public:
    HTMLMap( KHTMLWidget *w, HTMLString _url );
    virtual ~HTMLMap();

    virtual void fileLoaded( QString, 
			     QString _filename );
    virtual bool fileLoaded( QString _url, QBuffer& _buffer, 
			     bool = false );

    void addArea( HTMLArea *_area )
	    {	areas.append( _area ); }
    const HTMLArea *containsPoint( int, int );

    HTMLString mapURL() const
	    {	return mapurl; }

    virtual const char * objectName() const { return "HTMLMap"; };

protected:
    bool fileLoaded( QIODevice& file );
  
    QList<HTMLArea> areas;
    KHTMLWidget *htmlWidget;
    HTMLString mapurl;
};

//----------------------------------------------------------------------------

class HTMLImageMap : public HTMLImage
{
public:
    HTMLImageMap( KHTMLWidget *widget, HTMLString, 
                  HTMLString _url, HTMLString _target, 
                  int _width = UNDEFINED, int _height = UNDEFINED, 
                  int _percent = UNDEFINED, int brd = 0 );
    virtual ~HTMLImageMap() {}

    virtual HTMLObject* checkPoint( int, int );

    virtual HTMLString getURL() const { return dynamicURL; }
    virtual HTMLString getTarget() const { return dynamicTarget; }
     
    void setMapURL( HTMLString _url )
	    {	mapurl = _url; }
    HTMLString mapURL() const
	    {	return mapurl; }

    enum Type { ClientSide, ServerSide };

    void setMapType( Type t )
	    {	type = t; }
    bool mapType() const
	    {	return type; }

    virtual const char * objectName() const { return "HTMLImageMap"; };

protected:
    /*
     * The URL set by <a href=...><img ... ISMAP></a> for server side maps
     */
    QString serverurl;

    /*
     * The destination URL
     */
    QString dynamicURL;

    /*
     * The destination Target
     */
    QString dynamicTarget;

    /*
     * The URL set by <img ... USEMAP=mapurl> for client side maps
     */
    HTMLString mapurl;

    /*
     * ClientSide or ServerSide
     */
    Type type;
};

//----------------------------------------------------------------------------

class HTMLBackground : public HTMLObject
{
public:
    HTMLBackground( KHTMLWidget *widget, HTMLString imageURL, 
		    QColor& color );
    HTMLBackground( KHTMLWidget *widget, QColor& color );
    virtual ~HTMLBackground();

    virtual void changeImage( HTMLString _url );
    virtual void changeColor( QColor& );

    virtual void setPixmap( QPixmap * );
    virtual void pixmapChanged( QPixmap * = 0 );

    virtual HTMLString  getURL() const { return imageURL; }

    virtual const char * objectName() const { return "HTMLBackground"; };
    /**
     * prints debug info to stdout
     */
    virtual void printDebug( bool propagate = false, int indent = 0,
 			     bool printObjects = false );

    virtual int  getAbsX() { return 0; }
    virtual int  getAbsY() { return 0; }

    bool isNull() { return pixmap == 0; };

    void setBorder( int left, int right, int top, int bottom);

    bool print( QPainter *_painter, int _x, int _y, int _width,
		int _height, int _tx, int _ty, bool toPrinter );

protected:

    /*
     * Pointer to the image
     * If this pointer is 0L, that means that the picture could not be loaded
     * for some strange reason or that the image is waiting to be downloaded
     * from the internet for example.
     */
    QPixmap *pixmap;
    QColor bgColor;

    /**
     * The URL of this image.
     */
    HTMLString imageURL;
    
    KHTMLWidget *htmlWidget;
    
    bool bComplete;

    int leftBorder;
    int rightBorder;
    int topBorder;
    int bottomBorder;
};

//-----------------------------------------------------------------------------

#endif // HTMLOBJ
