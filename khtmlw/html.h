#ifndef HTML_H
#define HTML_H

#include <qpainter.h>
#include <qstrlist.h>
#include <qpixmap.h>
#include <qstack.h>
#include <qfont.h>
#include <qtimer.h>
#include <qlist.h>
#include <kurl.h>

class KHTMLWidget;

#include "drag.h"
#include "htmlobj.h"
#include "htmlform.h"
#include "htmltoken.h"

// Borders between widgets frame and displayed text
#define LEFT_BORDER 10
#define RIGHT_BORDER 10

class KHTMLWidget;

typedef void (KHTMLWidget::*parseFunc)(HTMLClueV *_clue, const char *str);

class KHTMLWidget : public KDNDWidget
{
    Q_OBJECT
public:
    /// Constructor
    /**
      _pix_path is the directory that conatins the *.xpm files. The file khtmlw_dnd.xpm
      is used for DND stuff supported by this widget.
      */
    KHTMLWidget( QWidget *parent, const char *name, const char *_pix_path );
    virtual ~KHTMLWidget();

	// we don't want global palette changes affecting us
	virtual void setPalette( const QPalette & ) {}

    /*********************************************************
     * Start writing HTML code using the write function.
     * '_url' is the URL we are going to display. The value is
     * stored in actualURL. You can tell the widget to use the given
     * offsets. It must not use them. It is just a hint. Use this
     * if the user is in the middle of the document and you have to make little
     * chnages to the HTML code ( deleting files in KFM ). So the user does not
     * go back to start of the document.
     */
    void begin( const char *_url = 0L, int _x_offset = 0, int _y_offset = 0 );
    /*********************************************************
     * Adds the string to the HTML code.
     */
    void write( const char * );
    /*********************************************************
     * End writing HTML code.
     */
    void end();
    /*********************************************************
     * Parse the HTML code. Use this after begin(), write()..., end().
     * This does not cause any painting actions but be aware that
     * for font stuff we need a QPainter there.
     */
    void parse();
	void stopParser();

    /*********************************************************
     * Selects all objects which refer to _url. All selected ojects
     * are redrawn if they changed their selection mode.
     */
    virtual void selectByURL( QPainter *_painter, const char *_url, bool _select );
    /*********************************************************
     * Selects/Unselects alls objects. This is usually used to disable
     * a selection. All objects are redrawn afterwards if they changed
     * their selection mode.
     */
    virtual void select( QPainter *_painter, bool _select );
    /// Select all objects matching the regular expression.
    /**
      If _painter is NULL a new painter is created.
      */
    virtual void select( QPainter *_painter, QRegExp& _pattern, bool _select );
    /*********************************************************
     * Gets a list of all selected URLs. The list may be Null.
     * You can test this using list.isNull().
     */
    virtual void getSelected( QStrList &_list );
    /*********************************************************
     * Checks out wether there is a URL under the point p and returns a pointer
     * to this URL or 0L if there is none.
     */
    const char* getURL( QPoint & );

    /*********************************************************
     * Returns the width of the parsed HTML code. Remember that
     * the documents width depends on the width of the widget.
     */
    int docWidth();
    /*********************************************************
     * Returns the height of the parsed HTML code. Remember that
     * the documents height depends on the height of the widget.
     */
    int docHeight();

    int xOffset() { return x_offset; }
    int yOffset() { return y_offset; }

    /*********************************************************
     * Find the anchor named '_name'. If the anchor is found, the widget
     * scrolls to the closest position. Returns TRUE if the anchor has
     * been found.
     */
    bool gotoAnchor( const char *_name );

    /*********************************************************
     * Sets the base font size ( range: 2-5,  default: 3 )
     * Note that font sizes are not defined in points.
     * Font sizes range from 1 (smallest) to 7 (biggest).
     */
    void setDefaultFontBase( int size )
    {	if ( size < 2 ) size = 2;
        else if ( size > 5 ) size = 5;
        defaultFontBase = size - 1;
    }

    /*********************************************************
     * Sets the font styles
     */
	void setStandardFont( const char *name )
		{	standardFont = name; }
	void setFixedFont( const char *name )
		{	fixedFont = name; }

    /*********************************************************
     * Sets the cursor to use when the cursor is on a link
     */
    void setURLCursor( const QCursor &c )
    {	linkCursor = c; }

	/*********************************************************
	 * Cryptic?  This is used to set the number of tokens to parse
	 * in one timeslice during background processing.  You probably
	 * don't need to touch this.
	 */
	void setGranularity( int g )
		{   granularity = g; }

    /// Requests an image
    /**
      If a HTMLImage object needs an image from the web, it
      calls this function.
      */
    void requestImage( HTMLImage* _img, const char *_url );

    /// This function is called to download the background image from the web
    void requestBackgroundImage( const char *_url );

    /// Redraws a single object
    /**
      This function is used to repaint images that have been loaded from the web.
      */
    void paintSingleObject( HTMLObject *_obj );

    /// Tells the widget to parse again after the last image arrived
    /**
      If we have a image of undefined size, the HTMLImage will call this
      function to tell the widget to do so.
      */
    void parseAfterLastImage();

    /// Registers QImageIO handlers for JPEG and GIF
    static void registerFormats();
    
signals:
    /*********************************************************
     * This signal is emitted whenever the Widget wants to
     * change the windows title. Usually this is the text
     * enclosed in <title>....</title>
     */
    void setTitle( const char * );
    /*********************************************************
     * The user double clicked on the URL _url with the
     * mouse button _button.
     */
    void doubleClick( const char * _url, int _button);

    /*********************************************************
     * Tells the parent, that the widget wants to scroll. This may happen if
     * the user selects an <a href="#anchor">.
     */
    void scrollVert( int _y );
    /*********************************************************
     * Tells the parent, that the widget wants to scroll. This may happen if
     * the user selects an <a href="#anchor">.
     */
    void scrollHorz( int _x );

    /*********************************************************
     * Signals that the URL '_url' has been selected. The user used
     * the mouse button '_button' for this.
     */
    void URLSelected( const char *_url, int _button );

    /*********************************************************
     * Signals that the mouse cursor is over URL '_url'
     * if _url is NULL then cursor moved off a URL
     */
    void onURL( const char *_url );

    /*********************************************************
     * indicates the document has changed due to new url loaded
     * or progressive update
     */
    void documentChanged();

	/*********************************************************
	 * signal when document is finished parsing
	 */
	void documentDone();
    
    /// Emitted if the user pressed the right mouse button
    /**
      If the user pressed the mouse button over an URL than _url
      points to this URL. _url may be 0L, too.
      The position is already in global cooredinates.
      */
    void popupMenu( const char *_url, const QPoint & );

    /// The widget requests to load an image
    /**
      KHTMLWidget can only load image from your local disk. If it
      finds an image with another protocol in its URL, it will emit this
      signal. If the image is loaded at some time, call 'slotImageLoaded'.
      If the image is not needed any more, the signal 'cancelImageRequest'
      is emitted.
      */
    void imageRequest( const char *_url );
    
    /// Cancels an image that has been requested.
    void cancelImageRequest( const char *_url );

	/// signal when user has submitted a form
	void formSubmitted( const char *_method, const char *_url );

	/// signal that the HTML Widget has changed size
	void resized( const QSize &size );
        
public slots:
    /*********************************************************
     * Causes the widget to scroll _dy up/down.
     */
    void slotScrollVert( int _dy );

    /*********************************************************
     * Causes the widget to scroll _dx left/right.
     */
    void slotScrollHorz( int _dx );

    /// Called if an image request is completed
    /**
      The rquested image is named '_url' and is stored on the local disk
      in the file named '_filename'.
      */
    void slotImageLoaded( const char *_url, const char *_filename );
    
protected slots:
    void slotTimeout();

	void slotFormSubmitted( const char *_method, const char *_url );

protected:
    enum ListType { Unordered, UnorderedPlain, Ordered, Menu, Dir };

    virtual void mousePressEvent( QMouseEvent * );
    /*********************************************************
     * This function emits the 'doubleClick' signal when the user
     * double clicks a <a href=...> tag.
     */
    virtual void mouseDoubleClickEvent( QMouseEvent * );
    /*********************************************************
     * Overload this method if you dont want any drag actions.
     */
    virtual void dndMouseMoveEvent( QMouseEvent * _mouse );
    /*********************************************************
     * This function emits the 'URLSelected' signal when the user
     * pressed a <a href=...> tag.
     */
    virtual void dndMouseReleaseEvent( QMouseEvent * );

    virtual void dragEndEvent();

    virtual void paintEvent( QPaintEvent * );

	virtual void resizeEvent( QResizeEvent * );

	virtual void keyPressEvent( QKeyEvent * );

	// reimplemented to prevent flicker
	virtual void focusInEvent( QFocusEvent * ) { }
	virtual void focusOutEvent( QFocusEvent * ) { }

    /*********************************************************
     * This function is called after <body> usually. You can
     * call it for every rectangular area: For example a tables cell
     * or for a menus <li> tag. ht gives you one token after another.
     * _clue points to a VBox. All HTMLObjects created by this
     * function become direct or indirect children of _clue.
     * The last two parameters define which token signals the end
     * of the section this function should parse, for example </body>.
     * You can specify two tokens, for example </li> and </menu>. You
     * may even set the second one to "" if you dont need it.
     */
    const char* parseBody( HTMLClueV *_clue, const char *[], bool toplevel = FALSE );

	/*********************************************************
	 * Parse marks starting with character, e.g.
	 * <img ...  is processed by KHTMLWidget::parseI()
	 * </ul>     is processed by KHTMLWidget::parseU()
	 */
	void parseA( HTMLClueV *_clue, const char *str );
	void parseB( HTMLClueV *_clue, const char *str );
	void parseC( HTMLClueV *_clue, const char *str );
	void parseD( HTMLClueV *_clue, const char *str );
	void parseE( HTMLClueV *_clue, const char *str );
	void parseF( HTMLClueV *_clue, const char *str );
	void parseG( HTMLClueV *_clue, const char *str );
	void parseH( HTMLClueV *_clue, const char *str );
	void parseI( HTMLClueV *_clue, const char *str );
	void parseJ( HTMLClueV *_clue, const char *str );
	void parseK( HTMLClueV *_clue, const char *str );
	void parseL( HTMLClueV *_clue, const char *str );
	void parseM( HTMLClueV *_clue, const char *str );
	void parseN( HTMLClueV *_clue, const char *str );
	void parseO( HTMLClueV *_clue, const char *str );
	void parseP( HTMLClueV *_clue, const char *str );
	void parseQ( HTMLClueV *_clue, const char *str );
	void parseR( HTMLClueV *_clue, const char *str );
	void parseS( HTMLClueV *_clue, const char *str );
	void parseT( HTMLClueV *_clue, const char *str );
	void parseU( HTMLClueV *_clue, const char *str );
	void parseV( HTMLClueV *_clue, const char *str );
	void parseW( HTMLClueV *_clue, const char *str );
	void parseX( HTMLClueV *_clue, const char *str );
	void parseY( HTMLClueV *_clue, const char *str );
	void parseZ( HTMLClueV *_clue, const char *str );
 
    /*********************************************************
     * This function is called after the <grid> tag.
     */
    const char* parseGrid( HTMLClue *_clue, int _max_width, const char *attr );

    /*********************************************************
     * parse list types
     */
    const char* parseList( HTMLClueV *_clue, int _max_width, ListType t );

    /*********************************************************
     * parse glossaries
     */
    const char* parseGlossary( HTMLClueV *_clue, int _max_width );

    /*********************************************************
     * parse table
     */
    const char* parseTable( HTMLClueV *_clue, int _max_width, const char * );

    /*********************************************************
	 * parse input
	 */
	const char* parseInput( const char * );

    /*********************************************************
     * This function is used for convenience only. It inserts a vertical space
     * if this has not already been done. For example
     * <h1>Hello</h1><p>How are you ?
     * would insert a VSpace behind </h1> and one in front of <p>. This is one
     * VSpace too much. So we use 'space_inserted' to avoid this. Look at
     * 'parseBody' to see how to use this function.
     * Assign the return value to 'space_inserted'.
     */
    bool insertVSpace( HTMLClueV *_clue, bool _space_inserted );

    /*********************************************************
     * draw background area
     */
    void drawBackground( int _xval, int _yval, int _x, int _y, int _w, int _h );

    /*********************************************************
	 * position form elements (widgets) on the page
	 */
	void positionFormElements();

    /*********************************************************
     * The <title>...</title>.
     */
    QString title;
    /*********************************************************
     * If we are in an <a href=..> ... </a> tag then the href
     * is stored in this string.
     */
    char url[1024];
    /*********************************************************
     * This is the URL that the cursor is currently over
     */
	QString overURL;
    /*********************************************************
     * This painter is created at need, for example to draw
     * a selection or for font metrics stuff.
     */
    QPainter *painter;
    /*********************************************************
     * This is the pointer to the tree of HTMLObjects.
     */
    HTMLClueV *clue;
    /*********************************************************
     * This is the scroll offset. The upper left corner is (0,0).
     */
    int x_offset, y_offset;

    /*********************************************************
     * This object contains the complete text. This text is referenced
     * by HTMLText objects for example. So you may not delete
     * 'ht' until you have deletet the complete tree 'clue' is
     * pointing to.
     */
    HTMLTokenizer *ht;

    /*********************************************************
     * Makes the DEFAULT_FONT the actual font without changing
     * current weight and italic settings but adding a specified
     * amount of pixels to the DEFAULT_FONT_SIZE.
     */
    void selectFont( int );
    /*********************************************************
     * Makes the font specified by parameters the actual font
     */
    void selectFont( const char *_fontfamily, int _size, int _weight, bool _italic );
    /*********************************************************
     * Pops the top font form the stack and makes the new
     * top font the actual one. If the stack is empty ( should never
     * happen! ) the default font is pushed on the stack.
     */
    void popFont();

    const HTMLFont *currentFont()  { return font_stack.top(); }

    /// List of all image objects waiting to get their image loaded.
    QList<HTMLImage> waitingImageList;
    
    /*********************************************************
     * The font stack. The font on top of the stack is the currently
     * used font. Poping a font from the stack deletes the font.
     * So use top() to get the actual font. There must always be at least
     * one font on the stack.
     */
    QStack<HTMLFont> font_stack;

    /*********************************************************
     * The base font size
     */
	int fontBase;
	int defaultFontBase;

    /*********************************************************
     * The font styles
     */
	QString standardFont;
	QString fixedFont;

    /*********************************************************
     * The weight currently selected. This is affected by <b>..</b>
     * for example.
     */
    int weight;
    /*********************************************************
     * The fonts italic flag. This is affected by <i>..</i>
     * for example.
     */
    bool italic;
    /*********************************************************
     * The fonts underline flag. This is affected by <u>..</u>
     * for example.
     */
	bool underline;
    /*********************************************************
     * The fonts underline flag. This is affected by <u>..</u>
     * for example.
     */
	bool strikeOut;

    /*********************************************************
     * Used for drag and drop.
     */
    bool pressed;
    int press_x, press_y;
    /*********************************************************
     * When the user presses the mouse over an URL, this URL is stored
     * here. We might need it if the user just started a drag.
     */
    QString pressedURL;
    /*********************************************************
     * If the user drags a URL out of the widget, by default this pixmap
     * is used for the dragged icon. The pixmap is loaded in the constructor.
     */
    QPixmap dndDefaultPixmap;

    /*********************************************************
     * This is the URL which is visible on the screen. This URL
     * is needed to complete URLs like <a href="classes.html"> since
     * for example 'URLSelected' should provide a complete URL.
     * TODO: Does not work yet, because KURL is missing.
     */
    KURL actualURL;
    KURL baseURL;	// this will do for now - MRJ
					// actually we need something like this to implement
					// <base ...>

	/*********************************************************
	 * Text colors
	 */
	QColor textColor;
	QColor linkColor;
	QColor vLinkColor;

	/*********************************************************
	 * Timer to parse html in background
	 */
	QTimer timer;

	/*********************************************************
	 * begin() has been called, but not end()
	 */
	bool writing;

	/*********************************************************
	 * Is the widget currently parsing
	 */
	bool parsing;

	/*********************************************************
	 * Nested level of current list item
	 */
	int listLevel;

	/*********************************************************
	 * The current alignment, set by <DIV > or <CENTER>
	 */
	HTMLClue::HAlign divAlign;

	/*********************************************************
	 * Number of tokens parsed in the current time-slice
	 */
	int parseCount;
	int granularity;

	/*********************************************************
	 * Used to avoid inserting multiple vspaces
	 */
	bool vspace_inserted;

	/*********************************************************
	 * The current flow box to add objects to
	 */
	HTMLClue *flow;

	/*********************************************************
	 * Array of paser functions, e.g.
	 * <img ...  is processed by KHTMLWidget::parseI() - parseFuncArray[8]()
	 * </ul>     is processed by KHTMLWidget::parseU() - parseFuncArray[20]()
	 */
	static parseFunc parseFuncArray[26];

	/*********************************************************
	 * This list holds strings which are displayed in the view,
	 * but are not actually contained in the HTML source.
	 * e.g. The numbers in an ordered list.
	 */
	QStrList tempStrings;

	QPixmap bgPixmap;

	/*********************************************************
	 * This is the cusor to use when over a link
	 */
	QCursor linkCursor;

    /// If this flag is set, the HTML code is parsed again after the last image arrived.
    bool bParseAfterLastImage;
    
    /// If this flag is set, the widget must repaint after parsing
    /**
      If an image is loaded from the web and we knew already about its size, it may
      happen that the image arrives during parsing. In this case we paint the loaded
      image after parsing has finished.
      */
    bool bPaintAfterParsing;

	bool bAutoUpdate;

    /// The URL of the not loaded!! background image
    /**
      If we are waiting for a background image then this is its URL.
      */
    QString bgPixmapURL;

	/// true if the current text is destined for <title>
	bool inTitle;

	/// List of forms in the page
	QList<HTMLForm> formList;

	/// Current form
	HTMLForm *form;

	/// Current select form element
	HTMLSelect *formSelect;

	/// true if the current text is destined for a <SELECT><OPTION>
	bool inOption;

	/// Current textarea form element
	HTMLTextArea *formTextArea;

	/// true if the current text is destined for a <TEXTAREA>...</TEXTAREA>
	bool inTextArea;

	/// the text to be put in a form element
	QString formText;
};

#endif // HTML

