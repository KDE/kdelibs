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
class HTMLTokenizer;
class HTMLClueFlow;
class KHTMLWidget;

#include "kpixmap.h"
#include "htmlfont.h"

// Every tag as deliverd by HTMLTokenizer starts with TAG_ESCAPE. This way
// you can devide between tags and words.
#define TAG_ESCAPE 13

// The count of spaces used for each tab.
#define TAB_SIZE 8

class HTMLAnchor;

//-----------------------------------------------------------------------------

class HTMLObject
{
public:
    HTMLObject();

    /************************************************************
     * This function should cause the HTMLObject to calculate its
     * width and height.
     */
    virtual void calcSize() { }
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
    /************************************************************
     * Print the object but only if it fits in the rectangle given
     * by _x,_y,_width,_ascender. (_x|_y) is the lower left corner relative
     * to the parent of this object ( usually HTMLClue ).
     */
    virtual void print( QPainter *, int, int, int, int, int, int ) { }
    /************************************************************
     * Print the object.
     */
    virtual void print( QPainter *, int, int ) { }
    virtual HTMLObject* checkPoint( int, int );
    virtual void selectByURL( QPainter *, const char *, bool, int _tx, int _ty );
    virtual void select( QPainter *, bool, int _tx, int _ty );
    /// Select all objects matching the regular expression.
    virtual void select( QPainter *_painter, QRegExp& _pattern, bool _select, int _tx, int _ty );
    virtual void select( bool _s ) { selected = _s; }
    virtual void getSelected( QStrList & );

    /************************************************************
	 * Some objects may need to know their absolute position on the page.
	 * This is only used by form elements so far.
	 */
	virtual void calcAbsolutePos( int, int ) { }

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
    void setPos( int _x, int _y ) { y=_y; x=_x; }
    void setXPos( int _x ) { x=_x; }
    void setYPos( int _y ) { y=_y; }
    void setPrinting( bool _p ) { printing = _p; }
    void setNewline( bool _n ) { newline = _n; }
    bool isSeparator() { return separator; }
    bool isNewline() { return newline; }
    bool isSelected() { return selected; }
	bool isFixedWidth() { return fixedWidth; }

    /************************************************************
     * Searches in all children ( and tests itself ) for an HTMLAnchor object
     * with the name '_name'. Returns 0L if the anchor could not be found.
     * '_point' is modified so that it holds the position of the anchor relative
     * to the parent.
     */
    virtual HTMLAnchor* findAnchor( const char */*_name*/, QPoint */*_point*/ )
			{ return 0L; }

protected:
    int x;
    int y;
    int width;
    int ascent;
    int descent;
    int max_width;
    // May be 0 to indicate that it is not used
    int max_ascent;
    // percent stuff added for table support
    int percent;	// width = max_width * percent / 100
    bool fixedWidth;
    bool separator;
    bool printing;
    bool newline;
    bool selected;
    QString url;
};

//-----------------------------------------------------------------------------

class HTMLClue : public HTMLObject
{
public:
    /************************************************************
     * This class is abstract. Do not instantiate it. The _y argument
     * is always 0 yet. _max_width defines the width you allow this Box
     * to have. If you do not use HCenter or Right and if this Box
     * becomes a child of a VBox you may set _x to give this Box
     * a shift to the right.
     */
    HTMLClue( int _x, int _y, int _max_width, int _percent = 100);

    enum VAlign { Top, Bottom, VCenter };
    enum HAlign { Left, HCenter, Right };

    virtual void print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty );
    /// Prints a special object only
    /**
      This function is for example used to redraw an image that had to be loaded from the world wide wait.
      */
    virtual void print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, HTMLObject *_obj );
    virtual void print( QPainter *, int _tx, int _ty );
    /************************************************************
     * Calls all children and tells them to calculate their size.
     */
    virtual void calcSize();
    virtual int  calcMinWidth();
    virtual int  calcPreferredWidth();
    virtual void setMaxAscent( int );
    virtual HTMLObject* checkPoint( int, int );
    virtual void selectByURL( QPainter *, const char *, bool, int _tx, int _ty );
    virtual void select( QPainter *_painter, QRegExp& _pattern, bool _select, int _tx, int _ty );
    virtual void select( QPainter *, bool, int _tx, int _ty );
    virtual void select( bool );
    virtual void getSelected( QStrList & );

	virtual void calcAbsolutePos( int _x, int _y );

    /************************************************************
     * Make an object a child of this Box.
     */
    void append( HTMLObject * );
    void setVAlign( VAlign _v ) { valign = _v; }
    void setHAlign( HAlign _h ) { halign = _h; }
    VAlign getVAlign() { return valign; }
    HAlign getHAlign() { return halign; }

    virtual HTMLAnchor* findAnchor( const char *_name, QPoint *_p );

protected:
    QList<HTMLObject> list;

	HTMLObject *prevCalcObj;

    VAlign valign;
    HAlign halign;
};

//-----------------------------------------------------------------------------

class HTMLClueFlow : public HTMLClue
{
public:
    HTMLClueFlow( int _x, int _y, int _max_width, int _percent=100)
		: HTMLClue( _x, _y, _max_width, _percent ) { }
    
    virtual void calcSize();
    virtual int  calcPreferredWidth();
    virtual void setMaxWidth( int );
    // virtual void print( QPainter *_painter, int _x, int _y, int _width, int _height );
};

//-----------------------------------------------------------------------------

class HTMLClueV : public HTMLClue
{
public:
    HTMLClueV( int _x, int _y, int _max_width, int _percent = 100 )
		: HTMLClue( _x, _y, _max_width, _percent ) { }

	virtual void setMaxWidth( int );
    virtual void calcSize();
    // virtual void print( QPainter *_painter, int _x, int _y, int _width, int _height );
};

//-----------------------------------------------------------------------------
// This clue is required for lists, etc. so that tables can dynamically
// change max_width and have the contents' max_widths changed appropriately.
//
class HTMLClueH : public HTMLClue
{
public:
	HTMLClueH( int _x, int _y, int _max_width, int _percent = 100 )
		: HTMLClue( _x, _y, _max_width, _percent ) { }
	
	virtual void setMaxWidth( int );
	virtual void calcSize();
	virtual int  calcPreferredWidth();
};

//-----------------------------------------------------------------------------
// really only useful for tables.
//
class HTMLTableCell : public HTMLClueV
{
public:
	HTMLTableCell( int _x, int _y, int _max_width, int _width, int _percent,
		int rs, int cs );

	int rowSpan() const
		{	return rspan; }
	int colSpan() const
		{	return cspan; }

protected:
	int rspan;
	int cspan;
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

    virtual void calcSize();
	virtual int  calcMinWidth();
	virtual int  calcPreferredWidth();
	virtual void setMaxWidth( int _max_width );
    virtual void setMaxAscent( int );
    virtual HTMLObject* checkPoint( int, int );
    virtual void selectByURL( QPainter *, const char *, bool, int _tx, int _ty );
    virtual void select( QPainter *_painter, QRegExp& _pattern, bool _select, int _tx, int _ty );
    virtual void select( QPainter *, bool, int _tx, int _ty );
    virtual void select( bool );
    virtual void getSelected( QStrList & );

	virtual void calcAbsolutePos( int _x, int _y );

	virtual HTMLAnchor *findAnchor( const char *_name, QPoint *_p );

    virtual void print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty );
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
	QArray<int> rowHeights;
	unsigned int col, totalCols;
	unsigned int row, totalRows;
	int spacing;
	int padding;
	int border;
};

//-----------------------------------------------------------------------------

class HTMLText : public HTMLObject
{
public:
    HTMLText( const char*, const HTMLFont *, QPainter *, const char * );
    HTMLText( const HTMLFont *, QPainter * );

    virtual void print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty );
    virtual void print( QPainter *, int _tx, int _ty );

protected:
    const char* text;
    const HTMLFont *font;
};

//-----------------------------------------------------------------------------

class HTMLRule : public HTMLObject
{
public:
	enum HAlign { Left, HCenter, Right };

    HTMLRule( int _max_width, int _width, int _percent, int _size=1,
		 HAlign _align=HCenter, bool _shade=TRUE );

	virtual int  calcMinWidth() { return 1; }
	virtual int  calcPreferredWidth() { return 1; }
	virtual void setMaxWidth( int );
    virtual void print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty );
    virtual void print( QPainter *, int _tx, int _ty );


protected:
	HAlign align;
	bool shade;
};

//-----------------------------------------------------------------------------

class HTMLBullet : public HTMLObject
{
public:
    HTMLBullet( int _height, int _level, const QColor &col );

    virtual void print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty );
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
};

//-----------------------------------------------------------------------------
// This class is inserted everywhere where a <a name="anchor">
// appears.
//
class HTMLAnchor : public HTMLObject
{
public:
    HTMLAnchor( const char *_name ) : name( _name ) {}

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

    KPixmap* getPixmap() { return pixmap; }
    const char *getFileName() { return filename.data(); }

protected:
    KPixmap *pixmap;
    QString filename;
};

//-----------------------------------------------------------------------------

class HTMLImage : public HTMLObject
{
public:
    HTMLImage( KHTMLWidget *widget, const char*, const char *, int _max_width, int _width = -1,
		 int _height = -1, int _percent = 0 );
	virtual ~HTMLImage();

	virtual int  calcMinWidth();
	virtual int  calcPreferredWidth();
	virtual void setMaxWidth( int );
    virtual void print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty );
    virtual void print( QPainter *, int _tx, int _ty );

    static void cacheImage( const char * );
    static KPixmap* findImage( const char * );

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
    KPixmap *pixmap;

    // The URL of this image.
    /**
      This variable is only used if we have to wait for the image.
      Otherwise this string will be empty.
      */
    QString imageURL;
    
    KHTMLWidget *htmlWidget;
    
    static QList<HTMLCachedImage> cache;

    /// Flag telling wether this image was found in the cache
    /**
      If this flag is set, you may not delete the pixmap since the pixmap
      belongs to the HTMLCachedImage.
      */
    bool cached;

    /// If we knew the size of the image from the <img width=...> tag then this flag is TRUE.
    /**
      We need this flag if the image has to be loaded from the web. In this case we may
      have to reparse the HTML code if we did not know the size during the first parsing.
      */
    bool predefinedWidth;

    /// If we knew the size of the image from the <img height=...> tag then this flag is TRUE.
    /**
      We need this flag if the image has to be loaded from the web. In this case we may
      have to reparse the HTML code if we did not know the size during the first parsing.
      */
    bool predefinedHeight;

    /// Tells the function 'imageLoaded' wether it runs synchronized with the constructor
    /**
      If an image has to be loaded from the net, it may happen that the image is cached.
      This means the the function 'imageLoaded' is called before the control returns to
      the constructor, since the constructor requested the image and this caused in turn
      'imageLoaded' to be called. In this case the images arrived just in time and no
      repaint or recalculate action must take place. If 'imageLoaded' works synchron with
      the constructor then this flag is set to TRUE.
      */
    bool synchron;
};

//-----------------------------------------------------------------------------

class StringTokenizer
{
public:
    StringTokenizer( QString &, const char * );
    ~StringTokenizer();

    const char* nextToken();
    bool hasMoreTokens();

protected:
    int pos;
    int size;
    char *buffer;
};

//-----------------------------------------------------------------------------

class HTMLTokenizer
{
public:
    HTMLTokenizer( QString & );
    ~HTMLTokenizer();

    const char* nextToken();
    bool hasMoreTokens();

protected:
    int pos;
    int size;
    char *buffer;
};

#endif // HTMLOBJ
