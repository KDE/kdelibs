#ifndef HTML_H
#define HTML_H

#define KHTMLW_VERSION  820

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
#include "htmlframe.h"
#include "htmlview.h"
#include "jscript.h"

// Default borders between widgets frame and displayed text
#define LEFT_BORDER 10
#define RIGHT_BORDER 20
#define TOP_BORDER 10
#define BOTTOM_BORDER 10

class KHTMLWidget;

typedef void (KHTMLWidget::*parseFunc)(HTMLClueV *_clue, const char *str);

class KHTMLWidget : public KDNDWidget
{
    Q_OBJECT
public:
	// pixdir should not be used - it is provided only for backward
	// compatability.
    KHTMLWidget( QWidget *parent = 0L, const char *name = 0L,
		const char *pixdir = 0L );
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
     * Print current page to the printer.
     */
    void print();

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
    /**
     * Selects all objects in this rectangle and deselects all objects
     * outside the rectangle. 
     *
     * @param _rect is a rectangle in display coordinates. This means
     *              that the point (0,0) is the upper/left most point of
     *              the widget but must not be this one for the HTML page.
     *              This happens if the widget is being scrolled.
     */
    virtual void select( QPainter * _painter, QRect &_rect );
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

    /**
     * If this widget belongs to a @ref HTMLView, then this function
     * is used to tell the widget about its owner.
     *
     * @see #htmlView     
     * @see #getView
     */
    void setView( KHTMLView *_view ) { htmlView = _view; }
    /**
     * @return the @ref KHTMLView this widget belongs to.
     *
     * @see #setView
     */
    KHTMLView* getView() { return htmlView; }

    /**
     * @return TRUE if the currently displayed document is a frame set.
     */
    bool isFrameSet() { return bIsFrameSet; }

    /**
     * Tells this widget that it displayes a frameset.
     * For internal use only.
     */
    void setIsFrameSet( bool _b );

    /**
     * @return a pointer to the currently selected frame ( @ref KHTMLView ) if we are
     *         displaying a frameset, otherwise 0L. If this widget is the selected
     *         one then @ref #htmlView is returned. Otherwise all @ref HTMLFrameSet
     *         instances are asked.
     */
    KHTMLView* getSelectedFrame();
  
    /**
     * @return TRUE if the currently displayed document is a frame.
     */
    bool isFrame() { return bIsFrame; }
    /**
     * Tell the widget wether it is a frame or not.
     * For internal use only.
     *
     * @see #isFrame
     */
    void setIsFrame( bool _frame);

    /**
     * Sets the amount of pixel for the margins width. This function is used
     * to implement the
     * <tt>&lt;frame marginwidth=... &gt;</tt> tag.
     * It is called from @ref KHTMLView and is for INTERNAL USE ONLY.
     *
     * @see #leftBorder
     * @see #rightBorder
     * @see KHTMLView::setMarginWidth
     */
    void setMarginWidth( int _w ) { leftBorder = _w; rightBorder = _w; }
    /**
     * Sets the amount of pixel for the margins height. This function is used
     * to implement the
     * <tt>&lt;frame marginheight=... &gt;</tt> tag.
     * It is called from @ref KHTMLView and is for INTERNAL USE ONLY.
     *
     * @see #topBorder
     * @see #bottomBorder
     * @see KHTMLView::setMarginHeight
     */
    void setMarginHeight( int _h ) { topBorder = _h; bottomBorder = _h; }
  
    /**
     * @return if the user selected this widget.
     *
     * @see #bIsSelected
     * @see #setSelected
     */
    bool isSelected()
    {
      return bIsSelected;
    }
  
    /**
     * Switches the 'selected state' of this widget. This results in the
     * drawing or deleting of the black border around the widget.
     *
     * @see #isSelected
     */
     void setSelected( bool _active );
  
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
      function to tell the widget to parse again.
      */
    void parseAfterLastImage();

    /// Registers QImageIO handlers for JPEG and GIF
    static void registerFormats();

    /**
     * @return a pointer to the @ref JSEnvironment instance used by this widget.
     *         Every call to this function will result in the same pointer.
     *
     * @see #jsEnvironment
     */
    JSEnvironment* getJSEnvironment();

    /**
     * A convenience function to access the @ref JSWindowObject of this html widget.
     *
     * @see #getJSEvironment
     */
    JSWindowObject* getJSWindowObject();

    /**
     * @return a list of all frames.
     *
     * @see #frameList
     */
    QList<KHTMLWidget>* getFrameList() { return &frameList; }
                                       
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
     * Signals that the URL '_url' has been selected. The user used
     * the mouse button '_button' for this.
     * '_target' is the target window or 0L is there is no one.
     * ( Used to implement frames ).
     */
    void URLSelected( const char *_url, int _button, const char *_target );

    /*********************************************************
     * Signals that the mouse cursor is over URL '_url'
     * if _url is NULL then cursor moved off a URL
     */
    void onURL( const char *_url );

    /**
     * indicates the document has changed due to new url loaded
     * or progressive update.
     */
    void documentChanged();
    /**
     * This signal is emitted if the widget got a call to @ref #parse
     * or @ref #begin. This indicates that the widget is working.
     * In a Web Browser you can use this to start an animated logo
     * like netscape does. The signal @ref #documentDone will tell
     * you that the widget finished its job.
     */
    void documentStarted();
    /**
     * This signal is emitted when document is finished parsing
     * and all required images arrived.
     *
     * @see documentStarted
     */
    void documentDone();
    
    /// Emitted if the user pressed the right mouse button
    /**
      If the user pressed the mouse button over an URL than _url
      points to this URL. _url may be 0L, too.
      The position is already in global cooredinates.
      */
    void popupMenu( const char *_url, const QPoint & );

    /**
     * This signal is emitted if the user presses the mouse. If he clicks on
     * a link you get the URL in '_url'.
     *
     * @param _url is the clicked URL or 0L is there was none.
     * @param _target is the target frame if one is mentioned otherwise 0L.
     * @param _ev the @ref QMouseEvent.
     */
    void mousePressed( const char *_url, const char *_target, QMouseEvent *_ev );
  
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

    /**
     * Called if this widget displays a frameset and the user selected
     * one of the frames. In this case we have to unselect the
     * currently selected frame if there is one.
     */
    void slotFrameSelected( KHTMLView *_view );
  
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
     * This function is called after the <cell> tag.
     */
    const char* parseCell( HTMLClue *_clue, const char *attr );

    /*********************************************************
     * parse glossaries
     */
    const char* parseGlossary( HTMLClueV *_clue, int _max_width );

    /*********************************************************
     * parse table
     */
    const char* parseTable( HTMLClue *_clue, int _max_width, const char * );

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
     * If we are in an <a target=..> ... </a> tag then the target
     * is stored in this string.
     */
    char target[1024];
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
    /**
     * If the user pressed the mouse button over an URL then this is the name
     * of the target window for thhis hyper link. Used to implement frames.
     *
     * @see #pressedURL
     */
    QString pressedTarget;
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
	 * size of current indenting
	 */
	int indent;

	class HTMLList
	{
		public:
			HTMLList( ListType t ) { type = t; itemNumber = 1; }
		ListType type;
		int itemNumber;
	};

	/*********************************************************
	 * Stack of lists currently active.
	 * The top affects whether a bullet or numbering is used by <li>
	 */
	QStack<HTMLList> listStack;

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
      If an image is loaded from the web and we knew already about its size,
	  it may happen that the image arrives during parsing. In this case we
	  paint the loaded image after parsing has finished.
      */
    bool bPaintAfterParsing;

	bool bAutoUpdate;

    /**
     * The URL of the not loaded!! background image
     * If we are waiting for a background image then this is its URL.
     * If the image is already loaded or if we dont have one this variable
	 * contains 0L. You can write bgPixmapURL.isNull() to test wether we are
	 * waiting for a background pixmap.
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

    /**
     * The toplevel frame set if we have frames otherwise 0L.
     */
    HTMLFrameSet *frameSet;
    /**
     * Stack of framesets used during parsing.
     */
    QList<HTMLFrameSet> framesetStack;
    /**
     * List of all framesets we are currently showing.
     * This list is not cleared after parsing like @ref #framesetStack.
     */
    QList<HTMLFrameSet> framesetList;  
    /**
     * List of all frames appearing in this window. They are stored in
     * source order. JavaScript uses this list to implement its
     * frames array.
     */
    QList<KHTMLWidget> frameList;    
  /**
   * @return TRUE if the current document is a framed document.
   */
  bool bIsFrameSet;
  /**
   * @return TRUE if the widget is a frame of a frameset.
   */
  bool bIsFrame;
  /**
   * Is TRUE if we parsed the complete frame set.
   */
  bool bFramesComplete;
  /**
   * If the owner of this widget is a @ref HTMLView then this is a
   * pointert to the owner, otherwise 0L.
   */
  KHTMLView *htmlView;
  /**
   * This is a pointer to the selectede frame. This means that the frame gets a black
   * inner border.
   */
  KHTMLView *selectedFrame;
  
  /**
   * Flag that indicates wether the user selected this widget. This is only of
   * interest if the widget is a frame in a frameset. Try Netscape to see
   * what I mean.
   *
   * @see #selectedFrame
   */
  bool bIsSelected;

  /**
   * Holds the amount of pixel for the left border. This variable is used
   * to implement the
   * <tt>&lt;frame marginwidth=... &gt;</tt> tag.
   *
   * @see #rightBorder
   * @see #setMarginWidth
   */
  int leftBorder;
  /**
   * Holds the amount of pixel for the right border. This variable is used
   * to implement the
   * <tt>&lt;frame marginwidth=... &gt;</tt> tag.
   *
   * @see #leftBorder
   * @see #setMarginWidth
   */
  int rightBorder;
  /**
   * Holds the amount of pixel for the top border. This variable is used
   * to implement the
   * <tt>&lt;frame marginheight=... &gt;</tt> tag.
   *
   * @see #bottomBorder
   * @see #setMarginHeight
   */
  int topBorder;
  /**
   * Holds the amount of pixel for the bottom border. This variable is used
   * to implement the
   * <tt>&lt;frame marginheight=... &gt;</tt> tag.
   *
   * @see #setMarginHeight
   * @see #topBorder
   */
  int bottomBorder;

    /**
     * This pointer is per default 0L. An instance of @ref JSEnvironment is created
     * if someone calls @ref #getJSEnvironment. This instance is used to run
     * java script.
     */
    JSEnvironment *jsEnvironment;      
  
};

#endif // HTML










