//-----------------------------------------------------------------------------
//
// KDE HTML Widget
//

#ifndef HTMLOBJ_H
#define HTMLOBJ_H

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

// The border around an aligned object
#define ALIGN_BORDER 2

class HTMLAnchor;

//-----------------------------------------------------------------------------
// HTMLObject is the base class for all HTML objects.
//
class HTMLObject
{
public:
    HTMLObject();
	virtual ~HTMLObject() {}

    enum VAlign { Top, Bottom, VCenter, VNone };
    enum HAlign { Left, HCenter, Right, HNone };

    /************************************************************
     * This function should cause the HTMLObject to calculate its
     * width and height.
     */
    virtual void calcSize( HTMLClue * = NULL ) { }
	/************************************************************
	 * This function forces a size calculation for objects which
	 * calculate their size at construction.  This is useful if
	 * the metrics of the painter change, e.g. if the html is to
	 * be printed on a printer instead of the display.
	 */
	virtual void recalcBaseSize( QPainter * ) { }
    /************************************************************
     * This function calculates the minimum width that the object
     * can be set to. (added for table support)
     */
	virtual int  calcMinWidth() { return width; }
    /************************************************************
     * This function calculates the width that the object would like
     * to be. (added for table support)
     */
    virtual int  calcPreferredWidth() { return width; }
    virtual void setMaxAscent( int ) { }
    virtual void setMaxWidth( int _w ) { max_width = _w; }
    virtual int  findPageBreak( int _y );
    /************************************************************
     * Print the object but only if it fits in the rectangle given
     * by _x,_y,_width,_ascender. (_x|_y) is the lower left corner relative
     * to the parent of this object ( usually HTMLClue ).
     */
    virtual bool print( QPainter *, int, int, int, int, int, int, bool = false ) { return false; }
    /************************************************************
     * Print the object.
     */
    virtual void print( QPainter *, int, int ) { }
    virtual HTMLObject* checkPoint( int, int );
    virtual void selectByURL( QPainter *, const char *, bool, int _tx, int _ty );
    virtual void select( QPainter *, bool, int _tx, int _ty );
    /**
     * Selects the object if it is inside the rectangle and deselects it otherwise.
     */
    virtual void select( QPainter *, QRect &_rect, int _tx, int _ty );
    /// Select all objects matching the regular expression.
    virtual void select( QPainter *_painter, QRegExp& _pattern, bool _select, int _tx, int _ty );
    virtual void select( bool _s ) { setSelected( _s ); }
    virtual void getSelected( QStrList & );

    /************************************************************
     * Some objects may need to know their absolute position on the page.
     * This is only used by form elements so far.
     */
    virtual void calcAbsolutePos( int, int ) { }
    
    virtual void reset() { setPrinted( false ); }

    /************************************************************
     * Get X-Position of this object relative to its parent
     */
    int getXPos() { return x; }
    /************************************************************
     * Get Y-Position of this object relative to its parent
     */
    int getYPos() { return y; }
    int getWidth() { return width; }
    int getHeight() { return ascent+descent; }
    int getAscent() { return ascent; }
    int getDescent() { return descent; }
    int getMaxWidth() { return max_width; }
    int getPercent() { return percent; }
    const char* getURL() { return url; }
    const char* getTarget() { return target; }

    void setPos( int _x, int _y ) { y=_y; x=_x; }
    void setXPos( int _x ) { x=_x; }
    void setYPos( int _y ) { y=_y; }

	enum ObjectFlags { Separator = 0x01, NewLine = 0x02, Selected = 0x04,
				FixedWidth = 0x08, Printing = 0x10, Aligned = 0x20,
				Printed = 0x40 };

    bool isSeparator() { return flags & Separator; }
    bool isNewline() { return flags & NewLine; }
    bool isSelected() { return flags & Selected; }
    bool isFixedWidth() { return flags & FixedWidth; }
    bool isPrinting() { return flags & Printing; }
    bool isAligned() { return flags & Aligned; }
    bool isPrinted() { return flags & Printed; }
    
    void setSeparator( bool s ) { s ? flags|=Separator : flags&=~Separator; }
    void setNewline( bool n ) { n ? flags|=NewLine : flags&=~NewLine; }
    void setSelected( bool s ) { s ? flags|=Selected : flags&=~Selected; }
    void setFixedWidth( bool f ) { f ? flags|=FixedWidth : flags&=~FixedWidth; }
    void setPrinting( bool p ) { p ? flags|=Printing : flags&=~Printing; }
    void setAligned( bool a ) { a ? flags|=Aligned : flags&=~Aligned; }
    void setPrinted( bool p ) { p ? flags|=Printed : flags&=~Printed; }
    
    /************************************************************
     * Searches in all children ( and tests itself ) for an HTMLAnchor object
     * with the name '_name'. Returns 0L if the anchor could not be found.
     * '_point' is modified so that it holds the position of the anchor relative
     * to the parent.
     */
    virtual HTMLAnchor* findAnchor( const char */*_name*/, QPoint */*_point*/ )
			{ return 0L; }

    void printCount() { printf( "Object count: %d\n", objCount ); }

protected:
    int x;
    int y;
    int width;
    int ascent;
    int descent;
    int max_width;
    // percent stuff added for table support
    short percent;	// width = max_width * percent / 100
    unsigned char flags;
    QString url;
    QString target;
    static int objCount;
};

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
	virtual ~HTMLClue() { }

	virtual int  findPageBreak( int _y );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter = false );
    /// Prints a special object only
    /**
     * This function is for example used to redraw an image that had to be loaded from the world wide wait.
     */
    virtual void print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, HTMLObject *_obj );
    virtual void print( QPainter *, int _tx, int _ty );
    /************************************************************
     * Calls all children and tells them to calculate their size.
     */
    virtual void calcSize( HTMLClue *parent = NULL );
	virtual void recalcBaseSize( QPainter * );
    virtual int  calcMinWidth();
    virtual int  calcPreferredWidth();
    virtual void setMaxAscent( int );
    virtual HTMLObject* checkPoint( int, int );
    virtual void selectByURL( QPainter *, const char *, bool, int _tx, int _ty );
    virtual void select( QPainter *_painter, QRegExp& _pattern, bool _select, int _tx, int _ty );
    virtual void select( QPainter *, bool, int _tx, int _ty );
    /**
     * Selects every objectsin this clue if it is inside the rectangle
     * and deselects it otherwise.
     */
    virtual void select( QPainter *, QRect &_rect, int _tx, int _ty );
    virtual void select( bool );
    virtual void getSelected( QStrList & );

    virtual void calcAbsolutePos( int _x, int _y );
	virtual void setIndent( int ) { }
    virtual void reset();

    /************************************************************
     * Make an object a child of this Box.
     */
    void append( HTMLObject *_object )
		{	list.append( _object ); }
	
    virtual void appendLeftAligned( HTMLClueAligned * ) { }
    virtual void appendRightAligned( HTMLClueAligned * ) { }
    virtual int  getLeftMargin( int )
    {	return 0; }
    virtual int  getRightMargin( int )
    {	return max_width; }
    
    void setVAlign( VAlign _v ) { valign = _v; }
    void setHAlign( HAlign _h ) { halign = _h; }
    VAlign getVAlign() { return valign; }
    HAlign getHAlign() { return halign; }

    virtual HTMLAnchor* findAnchor( const char *_name, QPoint *_p );

protected:
    QList<HTMLObject> list;

    int prevCalcObj;

    VAlign valign;
    HAlign halign;
};

//-----------------------------------------------------------------------------
// This clue is very experimental.  It is to be used for aligning images etc.
// to the left or right of the page.
//
class HTMLClueAligned : public HTMLClue
{
public:
    HTMLClueAligned( HTMLClue *_parent, int _x, int _y, int _max_width,
		     int _percent = 100 )
	: HTMLClue( _x, _y, _max_width, _percent )
    { prnt = _parent; setAligned( true ); }
    virtual ~HTMLClueAligned() { }
    
    virtual void setMaxWidth( int );
    virtual void setMaxAscent( int ) { }
    virtual void calcSize( HTMLClue *_parent = NULL );
    
    HTMLClue *parent()
    {	return prnt; }
    
private:
    HTMLClue *prnt;
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
    
    virtual void calcSize( HTMLClue *parent = NULL );
	virtual int  findPageBreak( int _y );
    virtual int  calcMinWidth();
    virtual int  calcPreferredWidth();
    virtual void setMaxWidth( int );

	virtual void setIndent( int i )
		{	indent = i; }

private:
	int indent;
};

//-----------------------------------------------------------------------------
// Align objects vertically
//
class HTMLClueV : public HTMLClue
{
public:
    HTMLClueV( int _x, int _y, int _max_width, int _percent = 100 )
		: HTMLClue( _x, _y, _max_width, _percent ) { }
	virtual ~HTMLClueV() { }

	virtual void reset();

	virtual void setMaxWidth( int );
    virtual HTMLObject* checkPoint( int, int );
    virtual void calcSize( HTMLClue *parent );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
		int _height, int _tx, int _ty, bool toPrinter = false );
    virtual void print( QPainter *_painter, int _x, int _y, int _width,
		int _height, int _tx, int _ty, HTMLObject *_obj )
		{	HTMLClue::print(_painter,_x,_y,_width,_height,_tx,_ty,_obj ); }

	virtual void appendLeftAligned( HTMLClueAligned *_clue )
		{	alignLeftList.append( _clue ); }
	virtual void appendRightAligned( HTMLClueAligned *_clue )
		{	alignRightList.append( _clue ); }
	virtual int  getLeftMargin( int _y );
	virtual int  getRightMargin( int _y );

protected:
	// These are the objects which are left or right aligned within this
	// clue.  Child objects must wrap their contents around these.
	QList<HTMLClueAligned> alignLeftList;
	QList<HTMLClueAligned> alignRightList;
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
		: HTMLClue( _x, _y, _max_width, _percent ) { }
	virtual ~HTMLClueH() { }
	
	virtual void setMaxWidth( int );
	virtual void calcSize( HTMLClue *parent = NULL );
    virtual int  calcMinWidth();
	virtual int  calcPreferredWidth();
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
    virtual bool print( QPainter *_painter, int _x, int _y, int _width,
		int _height, int _tx, int _ty, bool toPrinter = false );

protected:
	int rspan;
	int cspan;
	int padding;
	QColor bg;
};

//-----------------------------------------------------------------------------
// HTMLTable is totally unprepared for improperly formatted tables.
// MRJ - remember to add error checking to tables.
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
    virtual void calcSize( HTMLClue *parent = NULL );
	virtual int  calcMinWidth();
	virtual int  calcPreferredWidth();
	virtual void setMaxWidth( int _max_width );
    virtual void setMaxAscent( int );
    virtual HTMLObject* checkPoint( int, int );
    virtual void selectByURL( QPainter *, const char *, bool, int _tx, int _ty );
    virtual void select( QPainter *_painter, QRegExp& _pattern, bool _select, int _tx, int _ty );
    virtual void select( QPainter *, bool, int _tx, int _ty );
    /**
     * Selects every object in this table if it is inside the rectangle
     * and deselects it otherwise.
     */
    virtual void select( QPainter *, QRect &_rect, int _tx, int _ty );
    virtual void select( bool );
    virtual void getSelected( QStrList & );

	virtual void calcAbsolutePos( int _x, int _y );

	virtual HTMLAnchor *findAnchor( const char *_name, QPoint *_p );

	virtual int  findPageBreak( int _y );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter = false );
    virtual void print( QPainter *, int _tx, int _ty );

protected:
	void setCells( unsigned int r, unsigned int c, HTMLTableCell *cell );
	void calcColumnWidths();
	void optimiseCellWidth();
	void calcRowHeights();
	void addRows( int );
	void addColumns( int );

protected:
	HTMLTableCell ***cells;
	QArray<int> columnPos;
	QArray<int> columnPrefPos;
	QArray<int> columnOpt;
	QArray<int> rowHeights;
	unsigned int col, totalCols;
	unsigned int row, totalRows;
	int spacing;
	int padding;
	int border;
	HTMLClueV *caption;
	HTMLClue::VAlign capAlign;
};

//-----------------------------------------------------------------------------

class HTMLText : public HTMLObject
{
public:
    HTMLText( const char*, const HTMLFont *, QPainter *, const char *_url, const char *_target );
    HTMLText( const HTMLFont *, QPainter * );
	virtual ~HTMLText() { }

	virtual void recalcBaseSize( QPainter *_painter );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter = false );
    virtual void print( QPainter *, int _tx, int _ty );

protected:
    const char* text;
    const HTMLFont *font;
};

//-----------------------------------------------------------------------------

class HTMLRule : public HTMLObject
{
public:
    HTMLRule( int _max_width, int _width, int _percent, int _size=1,
		 HAlign _align=HCenter, bool _shade=TRUE );

	virtual int  calcMinWidth() { return 1; }
	virtual int  calcPreferredWidth() { return 1; }
	virtual void setMaxWidth( int );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter = false );
    virtual void print( QPainter *, int _tx, int _ty );


protected:
	HAlign align;
	int length;
	bool shade;
};

//-----------------------------------------------------------------------------

class HTMLBullet : public HTMLObject
{
public:
    HTMLBullet( int _height, int _level, const QColor &col );
	virtual ~HTMLBullet() { }

    virtual bool print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter = false );
    virtual void print( QPainter *, int _tx, int _ty );

protected:
	char level;
	QColor color;
};

//-----------------------------------------------------------------------------

class HTMLVSpace : public HTMLObject
{
public:
    HTMLVSpace( int _vspace );
	virtual ~HTMLVSpace() { }
};

//-----------------------------------------------------------------------------
// This class is inserted everywhere where a <a name="anchor">
// appears.
//
class HTMLAnchor : public HTMLObject
{
public:
    HTMLAnchor( const char *_name ) : name( _name ) {}
	virtual ~HTMLAnchor() { }

    const char* getName() { return name.data(); }

    virtual void setMaxAscent( int _a );
    virtual HTMLAnchor* findAnchor( const char *_name, QPoint *_p );

protected:
    QString name;
};

//-----------------------------------------------------------------------------

class HTMLCachedImage
{
public:
    HTMLCachedImage( const char * );
	virtual ~HTMLCachedImage() { }

    QPixmap* getPixmap() { return pixmap; }
    const char *getFileName() { return filename.data(); }

protected:
    QPixmap *pixmap;
    QString filename;
};

//-----------------------------------------------------------------------------

class HTMLImage : public HTMLObject
{
public:
    HTMLImage( KHTMLWidget *widget, const char*, const char *_url, const char *_target,
	       int _max_width, int _width = -1, int _height = -1, int _percent = 0 );
    virtual ~HTMLImage();

	virtual int  calcMinWidth();
	virtual int  calcPreferredWidth();
	virtual void setMaxWidth( int );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter = false );
    virtual void print( QPainter *, int _tx, int _ty );

    static void cacheImage( const char * );
    static QPixmap* findImage( const char * );

    const char* getImageURL() { return imageURL.data(); }
    /// Tells the object the the requested image is available
    /**
      The image is on the local disk in the file named '_filename.'
      */
    void imageLoaded( const char *_filename );
    
protected:

    /// Calculates the size of the loaded image.
    /**
      This function is usually called from the constructor or from
      'imageLoaded'.
      */
    void init();
    
    /// Pointer to the image
    /**
      If this pointer is 0L, that means that the picture could not be loaded
      for some strange reason or that the image is waiting to be downloaded from
      the internet for example.
      */
    QPixmap *pixmap;

    // The URL of this image.
    /**
      This variable is only used if we have to wait for the image.
      Otherwise this string will be empty.
      */
    QString imageURL;
    
    KHTMLWidget *htmlWidget;
    
    static QList<HTMLCachedImage>* pCache;

    /// Flag telling wether this image was found in the cache
    /**
      If this flag is set, you may not delete the pixmap since the pixmap
      belongs to the HTMLCachedImage.
      */
    bool cached;

    /// If we knew the size of the image from the <img width=...> tag then this flag is TRUE.
    /**
      We need this flag if the image has to be loaded from the web. In this
	  case we may have to reparse the HTML code if we did not know the size
	  during the first parsing.
      */
    bool predefinedWidth;

    /// If we knew the size of the image from the <img height=...> tag then this flag is TRUE.
    /**
      We need this flag if the image has to be loaded from the web. In this
	  case we may have to reparse the HTML code if we did not know the size
	  during the first parsing.
      */
    bool predefinedHeight;

    /// Tells the function 'imageLoaded' wether it runs synchronized with the constructor
    /**
      If an image has to be loaded from the net, it may happen that the image
	  is cached.  This means the the function 'imageLoaded' is called before
	  the control returns to the constructor, since the constructor requested
	  the image and this caused in turn 'imageLoaded' to be called. In this
	  case the images arrived just in time and no repaint or recalculate
	  action must take place. If 'imageLoaded' works synchron with
      the constructor then this flag is set to TRUE.
      */
    bool synchron;
};

#endif // HTMLOBJ

