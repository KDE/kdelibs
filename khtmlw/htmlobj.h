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
	// select text.  returns whether any text was selected.
	virtual bool selectText( QPainter *_painter, int _x1, int _y1,
	    int _x2, int _y2, int _tx, int _ty );
    virtual void getSelected( QStrList & );
	virtual void getSelectedText( QString & ) {}

    /************************************************************
     * Some objects may need to know their absolute position on the page.
     * This is only used by form elements so far.
     */
    virtual void calcAbsolutePos( int, int ) { }
    
    virtual void reset() { setPrinted( false ); }

	enum ObjectType { Object, Clue };
    /************************************************************
	 * sometimes a clue would like to know if an object is a 
	 * clue or a basic object.
	 */
	virtual ObjectType getObjectType() const
		{	return Object; }

    /************************************************************
     * Get X-Position of this object relative to its parent
     */
    int getXPos() const { return x; }
    /************************************************************
     * Get Y-Position of this object relative to its parent
     */
    int getYPos() const { return y; }
    int getWidth() const { return width; }
    int getHeight() const { return ascent+descent; }
    int getAscent() const { return ascent; }
    int getDescent() const { return descent; }
    int getMaxWidth() const { return max_width; }
    int getPercent() const { return percent; }
    const char* getURL() const { return url; }
    const char* getTarget() const { return target; }

    void setPos( int _x, int _y ) { y=_y; x=_x; }
    void setXPos( int _x ) { x=_x; }
    void setYPos( int _y ) { y=_y; }

	enum ObjectFlags { Separator = 0x01, NewLine = 0x02, Selected = 0x04,
				FixedWidth = 0x08, Printing = 0x10, Aligned = 0x20,
				Printed = 0x40 };

    bool isSeparator() const { return flags & Separator; }
    bool isNewline() const { return flags & NewLine; }
    bool isSelected() const { return flags & Selected; }
    bool isFixedWidth() const { return flags & FixedWidth; }
    bool isPrinting() const { return flags & Printing; }
    bool isAligned() const { return flags & Aligned; }
    bool isPrinted() const { return flags & Printed; }
    
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

class HTMLText : public HTMLObject
{
public:
    HTMLText( const char*, const HTMLFont *, QPainter *, const char *_url, const char *_target );
    HTMLText( const HTMLFont *, QPainter * );
	virtual ~HTMLText() { }

	virtual bool selectText( QPainter *_painter, int _x1, int _y1,
	    int _x2, int _y2, int _tx, int _ty );
	virtual void getSelectedText( QString & );
	virtual void recalcBaseSize( QPainter *_painter );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter = false );
    virtual void print( QPainter *, int _tx, int _ty );

protected:
	int getCharIndex( QPainter *_painter, int _xpos );

protected:
    const char* text;
    const HTMLFont *font;
	// This is a rediculous waste of memory, but I can't think of another
	// way at the moment.
	short selStart;
	short selEnd;
};

//-----------------------------------------------------------------------------

class HTMLRule : public HTMLObject
{
public:
    HTMLRule( int _max_width, int _percent, int _size=1, bool _shade=TRUE );

	virtual int  calcMinWidth();
	virtual int  calcPreferredWidth() { return calcMinWidth(); }
	virtual void setMaxWidth( int );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter = false );
    virtual void print( QPainter *, int _tx, int _ty );


protected:
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
    HTMLImage( KHTMLWidget *widget, const char*, const char *_url,
		const char *_target, int _max_width, int _width = -1,
		int _height = -1, int _percent = 0 );
    virtual ~HTMLImage();

	virtual int  calcMinWidth();
	virtual int  calcPreferredWidth();
	virtual void setMaxWidth( int );
    virtual bool print( QPainter *_painter, int _x, int _y, int _width, int _height, int _tx, int _ty, bool toPrinter = false );
    virtual void print( QPainter *, int _tx, int _ty );

    static void cacheImage( const char * );
    static QPixmap* findImage( const char * );

    const char* getImageURL() const { return imageURL.data(); }
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

//----------------------------------------------------------------------------
/*
 * HTMLArea holds an area as specified by the <AREA > tag.
 */
class HTMLArea
{
public:
	HTMLArea( const QPointArray &_points, const char *_url,
		const char *_target = 0 );
	HTMLArea( const QRect &_rect, const char *_url, const char *_target = 0 );
	HTMLArea( int _x, int _y, int _r, const char *_url,
		const char *_target = 0 );

	enum Shape { Poly, Rect, Circle };

	bool contains( const QPoint &_point ) const
		{	return region.contains( _point ); }

	const QString &getURL() const
		{	return url; }
	const QString &getTarget() const
		{	return target; }

protected:
	QRegion region;
	QString url;
	QString target;
};

//----------------------------------------------------------------------------

/*
 * HTMLMap contains a list of areas in the image map.
 * i.e. all areas between <MAP > </MAP>
 */
class HTMLMap
{
public:
	HTMLMap( const char *_url );

	void addArea( HTMLArea *_area )
		{	areas.append( _area ); }
	const HTMLArea *checkPoint( int, int );

	const QString &mapURL() const
		{	return url; }

protected:
	QString url;
	QList<HTMLArea> areas;
};

//----------------------------------------------------------------------------

class HTMLImageMap : public HTMLImage
{
public:
	HTMLImageMap( KHTMLWidget *widget, const char*, const char *_url,
		const char *_target, int _max_width, int _width = -1,
		int _height = -1, int _percent = 0 );
	virtual ~HTMLImageMap() {}

	virtual HTMLObject* checkPoint( int, int );

	void setMapURL( const char *_url )
		{	mapurl = _url; }
	const QString& mapURL() const
		{	return mapurl; }

protected:
	QString mapurl;
};

//----------------------------------------------------------------------------

#endif // HTMLOBJ

