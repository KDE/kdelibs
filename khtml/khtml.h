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
// KDE HTML Widget -- Main Widget
// $Id$

#ifndef HTML_H
#define HTML_H

#define KHTMLW_VERSION  2000		// 00.20.00

//#include <qpainter.h>
//#include <qstringlist.h>
#include <qpixmap.h>
//#include <qstack.h>
//#include <qfont.h>
#include <qtimer.h>
//#include <qlist.h>
#include <qdict.h>
#include <qstring.h>
#include <qscrollview.h>

#include "kurl.h"
#include "khtmlio.h"

class KHTMLCache;
class HTMLSettings;
class HTMLFileRequester;
class KHTMLDecoder;

class KHTMLWidget;

// DOM stuff
#include <html_document.h>
namespace DOM {
class HTMLFrameElement;
class HTMLDocumentImpl;
};
using namespace DOM;

class SavedPage;

/**
 * @short Basic HTML Widget.  Does not handle frames.
 *
 * This widget is good for use in your custom application which does not
 * necessarily want to handle frames.
 * To add content to the widget you should do the follwing:
 * <PRE>
 * view->begin( "file:/tmp/test.html" );
 * view->parse();
 * view->write( "&lt;HTML&gt;&lt;TITLE&gt;...." );
 * .....
 * view->end();
 * view->show();
 * </PRE>
 * The widget will take care of resize events and paint events.
 * Have a look at the set of signals emitted by this widget. You should connect
 * to most of them.
 *
 * Note: All HTML is parsed in the background using Qt timers, so you will not
 * see any content displayed until the event loop is running.
 */
class KHTMLWidget : public QScrollView
{
    friend class HTMLDocumentImpl;

    Q_OBJECT
public:
    /**
     * Create a new HTML widget.  The widget is empty by default.
     * You must use @ref #begin, @ref #write, @ref #end and @ref #parse
     * to fill the widget with content.
     *
     * @param _name is the name of the widget. Usually this name is only
     *              meaningful for Qt but in this case it is the name of
     *              the HTML window. This means you can reference this name
     *              in the &lt; href=... target=... &gt; tag. If this argument
     *              is 0L then a unique default name is chosen.
     *
     */
    KHTMLWidget( QWidget *parent = 0L, const char *name = 0L);
    virtual ~KHTMLWidget();

    /**
     * Clears the widget and prepares it for new content. If you display
     * for example "file:/tmp/test.html", you can use the following code
     * to get a value for '_url':
     * <PRE>
     * KURL u( "file:/tmp/test.html" );
     * view->begin( u.directoryURL() );
     * </PRE>
     *
     * @param _url is the url of the document to be displayed.  Even if you
     * are generating the HTML on the fly, it may be useful to specify
     * a directory so that any pixmaps are found.
     * @param _dx is the initial horizontal scrollbar value. Usually you don't
     * want to use this.
     * @param _dy is the initial vertical scrollbar value. Usually you don't
     * want to use this.
     */
    void begin( QString _url = 0L, int _x_offset = 0, int _y_offset = 0 );

    /**
     * Writes another part of the HTML code to the widget. You may call
     * this function many times in sequence. But remember: The less calls
     * the faster the widget is.
     *
     * The html code is send through a decoder, which decodes the stream to
     * unicode.
     *
     * Attention: Don't mix calls to write( const char *) with calls
     * to write( const QString & ). The result might not be what you want.
     */
    void write( const char * );

    /**
     * Writes another part of the HTML code to the widget. You may call
     * this function many times in sequence. But remember: The less calls
     * the faster the widget is.
     */
    void write( const QString & );

    /**
     * Call this after your last call to @ref #write.
     */
    void end();

    /**
     * Stop parsing the HTML immediately.
     */
    void stopParser();

    /**
     * Print current HTML page to the printer.
     */
    void print();

    /**
     * Recalculate the size and position of objects in the page.
     * This is mainly intended for internal use.
     */
    void calcSize();

    /**
     * Selects all objects which refer to _url. All selected ojects
     * are redrawn if they changed their selection mode.
     */
    virtual void selectByURL( QPainter *_painter, QString _url, bool _select );

    /**
     * Selects/Unselects all objects with an associated URL.
     * This is usually used to disable
     * a selection. All objects are redrawn afterwards if they changed
     * their selection mode.
     */
    virtual void select( QPainter *_painter, bool _select );

    /**
     * Selects all objects with an associated URL in this rectangle and
     * deselects all objects outside the rectangle.
     *
     * @param _rect is a rectangle in display coordinates. This means
     *              that the point (0,0) is the upper/left most point of
     *              the widget but must not be this one for the HTML page.
     *              This happens if the widget is being scrolled.
     */
    virtual void select( QRect &_rect );

    /**
     * Select all objects with a URL matching the regular expression.
     *
     * If _painter is null a new painter is created.
     */
    virtual void select( QPainter *_painter, QRegExp& _pattern, bool _select );

    /**
     * Gets a list of all selected URLs. The list may be Null.
     * You can test this using list.isNull().
     */
    virtual void getSelected( QStringList &_list );

    /**
     * Selects all text between ( _x1, _y1 ) and ( _x2, y2 ).  The selection
     * area selects text line by line, NOT by bounding rectangle.
     */
    virtual void selectText( int _x1, int _y1, int _x2, int _y2 );

    /**
     * Get the text the user has marked.
     *
     * @param _str is the QString which will contain the text the user
     * selected.  The selected text is appended to any text currently in
     * _str.
     */
    virtual void getSelectedText( QString &_str );

    /**
     * Get all text.
     *
     * @param _str is the QString which will contain all text.
     * The text is appended to any text currently in _str.
     */
    virtual void getAllText( QString &_str );

    /**
     * Has the user selected any text?  Call @ref #getSelectedText to
     * retrieve the selected text.
     *
     * @return true if there is text selected.
     */
    bool isTextSelected() const
	{	return bIsTextSelected; }

    /**
     * Initiate a text search.
     */
    void findTextBegin();

    /**
     * Find the next occurrance of the expression.
     */
    bool findTextNext( const QRegExp &exp );

    /**
     * end a text search.
     */
    void findTextEnd();

    /**
     * Checks out whether there is a URL under the point and returns a pointer
     * to this URL or 0L if there is none.
     *
     * @param _point the point to test for the presence of a URL.  The
     * point is relative to this widget.
     */
    QString getURL( QPoint &_point );

    /**
     * @return the width of the parsed HTML code. Remember that
     * the documents width depends on the width of the widget.
     */
    int docWidth() const;

    /**
     * @return the height of the parsed HTML code. Remember that
     * the documents height depends on the width of the widget.
     */
    int docHeight() const;

    /**
     * @return the url of this document
     */
//###
    KURL &getDocumentURL()
	{ return actualURL; }

    /**
     * @return the base URL of this document
     */
// ###
    KURL &getBaseURL()
	{ return baseURL; }

    /**
     * @obsolete use contentsX() instead
     * @return the horizontal position the view has been scrolled to.
     */
// ###
    int xOffset() const { return contentsX();}

    /**
     * @obsolete use contentsY() instead
     * @return the vertical position the view has been scrolled to.
     */
// ###
    int yOffset() const { return contentsY(); }

    /**
     * Find the anchor named '_name'. If the anchor is found, the widget
     * scrolls to the closest position. Returns TRUE if the anchor has
     * been found. If the anchor could not be found it is retried when a
     * new part of the document arrives.
     */
    bool gotoAnchor( QString _name );

    /**
     * Replaced.
     * use the appropriate QSrollView methods:
     * virtual void setContentsPos ( int x, int y )
     * void ensureVisible ( int x, int y )
     * void center ( int x, int y )
     *
     * bool gotoXY( int _x_offset, int _y_offset );
     */

    /**
     * Causes the widget contents to scroll automatically.  Call
     * @ref #stopAutoScrollY to stop.  Stops automatically when the
     * top or bottom of the document is reached.
     *
     * @param _delay Time in milliseconds to wait before scrolling the
     * document again.
     * @param _dy The amount to scroll the document when _delay elapses.
     */
    void autoScrollY( int _delay, int _dy );

    /**
     * Stops the document from @ref #autoScrollY ing.
     */
    void stopAutoScrollY();

    /**
     * Returns if the widget is currently auto scrolling.
     */
    bool isAutoScrollingY()
	{ return autoScrollYTimer.isActive(); }

    /**
     * @return TRUE if the currently displayed document is a frame set.
     */
    bool isFrameSet();

    void setIsFrame(bool b) { _isFrame = b; }
    bool isFrame() { return _isFrame; }

    bool selected() { return _isSelected; }
    void setSelected(KHTMLWidget *w = 0);
    
    /**
     * @return a pointer to the currently selected frame ( @ref KHTMLView ) if
     * we are displaying a frameset, otherwise 0L. If this widget is the
     * selected one then @ref htmlView is returned. Otherwise all
     * @ref HTMLFrameSet instances are asked.
     */
    KHTMLWidget* getSelectedFrame();

    /**
     * Sets point sizes to be associated with the HTML-sizes used in
     * <FONT size=Html-Font-Size>
     *
     * Html-Font-Sizes range from 1 (smallest) to 7 (biggest).
     */
    void setFontSizes(const int *newFontSizes, const int *newFixedFontSizes=0);

    /**
     * Gets point sizes to be associated with the HTML-sizes used in
     * <FONT size=Html-Font-Size>
     *
     * Html-Font-Sizes range from 1 (smallest) to 7 (biggest).
     */
    void getFontSizes(int *newFontSizes, int *newFixedFontSizes=0);

    /**
     * Resets the point sizes to be associated with the HTML-sizes used in
     * <FONT size=Html-Font-Size> to their default.
     *
     * Html-Font-Sizes range from 1 (smallest) to 7 (biggest).
     */
    void resetFontSizes( void );

    /**
     * Sets the standard font style.
     *
     * @param name is the font name to use for standard text.
     */
    void setStandardFont( QString name );

    /**
     * Sets the fixed font style.
     *
     * @param name is the font name to use for fixed text, e.g.
     * the <tt>&lt;pre&gt;</tt> tag.
     */
    void setFixedFont( QString name );

    /**
     * Sets the default background color to use when one isn't specified
     * explicitly by <tt>&lt;body bgcolor=...&gt;</tt>
     */
    void setDefaultBGColor( const QColor &col );

    /**
     * Sets the default text colors.
     */
    void setDefaultTextColors( const QColor &normal, const QColor &link,
	const QColor &vlink );

    /**
     * Set whether links are drawn in underlined text.
     */
    void setUnderlineLinks( bool ul );

    /**
     * Sets the cursor to use when the cursor is on a link.
     */
    void setURLCursor( const QCursor &c )
	{ linkCursor = c; }

    /**
     * Returns the cursor which is used when the cursor is on a link.
     */
    const QCursor& getURLCursor() { return linkCursor; }

    /**
     * Cryptic?  This is used to set the number of tokens to parse
     * in one timeslice during background processing.
     *
     * You probably don't need to touch this.
     */
    void setGranularity( int g )
	{   granularity = g; }


    /*
     * If a HTMLObject object needs a file from the web, it
     * calls this function.
     * if update is true, the htmlobj will be continously updated
     * as the file is loaded (via the data function)
     */
    // ########## implement update
    void requestFile( HTMLFileRequester *_obj, QString _url,
		      bool update = false );

    /*
     * Cancels a previous @ref requestFile.
     */
    void cancelRequestFile( HTMLFileRequester *_obj );
    void cancelRequestFile( QString _url );

    /*
     * Cancels all @ref requestFile.
     */
    void cancelAllRequests();

    void setBGColor(const QColor &c)
	{ viewport()->setBackgroundColor(c); }

    /*
     * Set background
     */
    //void setBackground( HTMLBackground *_background);

    /**
     * Returns the URL of the background image or NULL if no background
     * is available.
     */
    //QString getBackground();

    /**
     * @return a list of all frames.
     *
     * @see #frameList
     */
// ######
    //QList<KHTMLWidget>* getFrameList() { return &frameList; }

    /**
     * Set document charset.
     *
     * Any <META ...> setting charsets overrides this setting
     * as long as override isn't true.
     * @return TRUE if successfull
     */
    // ##### FIXME: remove override
    bool setCharset(QString name, bool override = false);

    // Another option to feed image data into the HTML Widget
    void data( QString _url, const char *_data, int _len, bool _eof );

    /**
     * Function used to save the current html-page into a struct
     * This does only work, if the page has a valid URL, pages
     * filled with the write() method are not saveable
     */
    //SavedPage *saveYourself(SavedPage *p = 0);
    /**
     * restore a page previously saved with @ref saveYourself()
     */
    //void restore(SavedPage *);

    /** @internal
     *  forces the repaint of a single element
     */
    void paintElement( NodeImpl *e, bool recursive=false );

    /**
     * For internal use only
     *
     * This is called, when the document has frames.
     * The default implementation does nothing. Use KBrowser, if you want
     * to have frames.
     */
    virtual KHTMLWidget* newView( QWidget *, const char *, int ) { return 0; }


    /**
     * hook to get the Document. Used eg by jscript to manipulate the document.
     */
    HTMLDocument htmlDocument() const { return document; }


signals:
    /**
     * This signal is emitted whenever the Widget wants to
     * change the window's title. Usually this is the text
     * enclosed in <tt>&lt;title&gt;....&lt;/title&gt;</tt>.
     */
    void setTitle( QString );

    /**
     * The user double clicked on a URL.
     *
     * @param _url the URL that the user clicked on.
     * @param _button the mouse button that was used.
     */
    void doubleClick( QString _url, int _button);

    /**
     * Tells the parent, that the widget has scrolled. This may happen if
     * the user selects an <tt>&lt;a href="#anchor"&gt;</tt>.
     */
    void scrollVert( int _y );

    /**
     * Tells the parent, that the widget has scrolled. This may happen if
     * the user selects an &lt;a href="#anchor"&gt;.
     */
    void scrollHorz( int _x );

    /**
     * Signals that a URL has been selected using a single click.
     *
     * @param _url is the URL clicked on.
     * @param _button is the mouse button clicked.
     * @param _target is the target window or 0L if there is none.
     * ( Used to implement frames ).
     */
    void URLSelected( QString _url, int _button, QString _target );

    /**
     * Signals that the mouse cursor has moved on or off a URL.
     *
     * @param _url is the URL that the mouse cursor has moved onto.
     * _url is null if the cursor moved off a URL.
     */
    void onURL( QString _url );

    /**
     * Signal that the user has selected text or the existing selection has
     * become unselected.  The text may be retrieved using
     * @ref #getSelectedText.  This is a good signal to connect to for
     * enabling/disabling the Copy menu item or calling XSetSelectionOwner().
     *
     * @param _selected is true if the user has selected text or false if
     * the current selection has been removed.
     */
    void textSelected( bool _selected );

    /**
     * Indicates the document has changed due to new URL loaded
     * or progressive update.  This signal may be emitted several times
     * while the document is being parsed.  It is an ideal opportunity
     * to update any scrollbars.
     */
    void documentChanged();

    /**
     * This signal is emitted if the widget got a call to @ref #parse
     * or @ref #begin. This indicates that the widget is working.
     * In a Web Browser you can use this to start an animated logo
     * like netscape does. The signal @ref #documentDone will tell
     * you that the widget finished its job.
     *
     * @see #documentDone
     */
    void documentStarted();

    /**
     * This signal is emitted when document is finished parsing
     * and all required images arrived.
     *
     * @see #documentStarted
     */
    void documentDone( KHTMLWidget * );

    /// Emitted if the user pressed the right mouse button
    /**
     * If the user pressed the right mouse button over a URL than _url
     * points to this URL, otherwise _url will be null.
     * The position is in global coordinates.
     */
    void popupMenu( QString _url, const QPoint & );

    /**
     * This signal is emitted if the user presses the mouse. If he clicks on
     * a link you get the URL in '_url'.
     *
     * @param _url is the clicked URL or null is there was none.
     * @param _target is the target frame if one is mentioned otherwise 0L.
     * @param _ev the @ref QMouseEvent.
     */
    void mousePressed( QString _url, QString _target, QMouseEvent *_ev );

    // The widget requests to load a file
    /**
     * KHTMLWidget can only load files from your local disk. If it
     * finds a object which requires a remote file, it will emit this
     * signal. If the file is loaded at some time, call @ref #slotFileLoaded.
     *
     * If the file is not needed any more, the signal @ref #cancelFileRequest
     * is emitted.
     *
     * @param _url is the URL of the image that needs to be loaded.
     */
    void fileRequest( QString _url );

    // Cancels a file that has been requested.
    void cancelFileRequest( QString _url );

    // signal when user has submitted a form
    void formSubmitted( QString _method, QString _url,
			const char *_data, QString _target );

    // signal that the HTML Widget has changed size
    void resized( const QSize &size );

    /**
     * signal is emitted, when an URL redirect
     * (<meta http-equiv="refresh" ...>) tag is encountered
     * delay is the delay in seconds; 0 means immediate redirect,
     */
    void redirect(int delay, QString url);

    virtual void executeScript( const QString &script );

public slots:
    /**
     * Scrolls the document to _y.
     *
     * This is usually connected to a scrollbar.
     */
    void slotScrollVert( int _y );

    /**
     * Scrolls the document to _x.
     *
     * This is usually connected to a scrollbar.
     */
    void slotScrollHorz( int _x );

    /**
     * Call when a file requested by @ref #fileRequest has been loaded.
     *
     * @param _url is the URL of the file that was requested.
     * @param _filename is the name of the file that has been stored on
     * the local filesystem.
     */
     void slotFileLoaded( QString _url, QString _filename );

protected slots:

    /*
     * INTERNAL
     *
     * Called by timer event when the widget is due to autoscroll.
     */
    void slotAutoScrollY();

    /*
     * INTERNAL
     *
     * Used to update the selection when the user has caused autoscrolling
     * by dragging the mouse out of the widget bounds.
     */
    void slotUpdateSelectText( int newy );

public slots:
    /*
     * INTERNAL
     *
     * Called when the user submitted a form.
     */
    void slotFormSubmitted( QString _method, QString _url,
    			    const char *_data, QString _target );

    /**
     * @internal
     */
    void slotExecuteScript( const QString &script )
	{ emit executeScript( script ); }
protected:
    /*
     * INTERNAL
     *
     * Called if this widget displays a frameset and the user selected
     * one of the frames. In this case we have to unselect the
     * currently selected frame if there is one.
     */
    //void slotFrameSelected( KHTMLView *_view );

protected:

    /**
     * The current document displayed in this widget
     */
    HTMLDocumentImpl *document;

    /**
     * decoder used to decode the input stream
     */
    KHTMLDecoder *decoder;

    // overrides QScrollview method
    virtual void drawContents ( QPainter * p, int clipx,
				int clipy, int clipw, int cliph );
public:
    virtual void layout();
protected:

    virtual void viewportMousePressEvent( QMouseEvent * );

    /**
     * This function emits the 'doubleClick' signal when the user
     * double clicks a &lt;a href=...&gt; tag.
     */
    virtual void viewportMouseDoubleClickEvent( QMouseEvent * );

    /**
     * This function is called when the user moves the mouse.
     */
    virtual void viewportMouseMoveEvent(QMouseEvent *);

    /**
     * this function is called when the user releases a mouse button.
     */
    virtual void viewportMouseReleaseEvent(QMouseEvent *);

    /**
     * Called when a URL is encountered.  Overload this method to indicate
     * which links have been visited previously.
     *
     * @return true if the URL has been visited previously.  If true is
     * returned the URL will be rendered in the vlink color.  If false
     * is returned the URL will be rendered in the link color.
     */
    //virtual bool URLVisited( QString _url );

    //virtual void paintEvent( QPaintEvent * );

    virtual void resizeEvent( QResizeEvent * );

    //virtual void keyPressEvent( QKeyEvent * );

    //virtual void timerEvent( QTimerEvent * );

    // we don't want global palette changes affecting us
    virtual void setPalette( const QPalette & ) {}

    // flush key presses from the event queue
    //void flushKeys();

    /*
     * functions used internally for saving and restoring html pages
     */
    //void buildFrameSet(SavedPage *p, QString *s);
    //void buildFrameTree(SavedPage *p, HTMLFrameSet *f);

    // the width the document is layouted to...
    int _width;

    /*
     * funciton used internally for getAllText
     */
    //void getFrameSetText( QString &_str, HTMLFrameSet *f);

    /*
     * This is the URL that the cursor is currently over
     */
    QString overURL;

    /*
     * Reference (anchor) to jump to once it becomes available
     */
    //QString reference;

    /*
     * This painter is created at need, for example to draw
     * a selection or for font metrics stuff.
     */
    QPainter *painter;

    /*
     * The background of the page is handled seperately.
     */
    //HTMLBackground *background;

    /*
     * The amount to auto scroll by.
     */
    int autoScrollDY;

    /*
     * The delay to wait before auto scrolling autoScrollDY
     */
    int autoScrollYDelay;

    /*
     * Timer for autoscroll stuff
     */
    QTimer autoScrollYTimer;

    /*
     * Timer used to schedule paint events
     */
    //QTimer updateTimer;

    /*
     * Used for drag and drop.
     */
    bool pressed;
    int press_x;
    int press_y;

    /*
     * When the user presses the mouse over an URL, this URL is stored
     * here. We might need it if the user just started a drag.
     */
    QString pressedURL;

    /**
     * If the user pressed the mouse button over an URL then this is the name
     * of the target window for this hyper link. Used to implement frames.
     *
     * @see #pressedURL
     */
    QString pressedTarget;

    /*
     * If the user drags a URL out of the widget, by default this pixmap
     * is used for the dragged icon. The pixmap is loaded in the constructor.
     */
    QPixmap dndDefaultPixmap;

    /*
     * Start of selection
     */
    QPoint selectPt1;

    /*
     * End of selection
     */
    QPoint selectPt2;

    /*
     * is any text currently selected
     */
    bool bIsTextSelected;

    /*
     * This is the URL which is visible on the screen. This URL
     * is needed to complete URLs like <a href="classes.html"> since
     * for example 'URLSelected' should provide a complete URL.
     */
    KURL actualURL;

    KURL baseURL;	// this will do for now - MRJ
			// actually we need something like this to implement
			// <base ...>
    /*
     * This sets the baseURL given a URL. It strips of any filename.
     */
    //void setBaseURL( QString _url);

    /*
     * A color context for the current page so that we can free the colors
     * when we close the page.
     */
    int colorContext;

    /*
     * Timer to parse html in background
     */
    //int timerId;

    /*
     * begin() has been called, but not end()
     */
    bool writing;

    bool _isFrame;
    bool _isSelected;
    
    /*
     * Number of tokens parsed in the current time-slice
     */
    int parseCount;
    int granularity;

    /*
     * This is the cusor to use when over a link
     */
    QCursor linkCursor;

    /*
     * Default settings.
     */
    HTMLSettings *defaultSettings;

    /*
     * Current settings for page
     */
    HTMLSettings *settings;
	
    /*
     * Sets new title
     * (Called by parser only)
     */
    void setNewTitle( QString _title);

    /**
     * List of all embeded widgets.
     *
     * @see #addEmbededFrame
     *
     */
    //QList<KHTMLEmbededWidget> embededFrameList;

    /**
     * Adds a new embeded frame.
     */
    //void addEmbededFrame( HTMLFrameSet *_frameSet, KHTMLEmbededWidget* _embed );

    /**
     * Is TRUE if we parsed the complete frame set.
     */
    //bool bFramesComplete;

    /**
     * This is a pointer to the selectede frame. This means that the frame
     * gets a black inner border.
     */
    //HTMLFrameElement *selectedFrame;

    /**
     * Iterator used to find text within the document
     */
    //HTMLIterator *textFindIter;

    /**
     * Keeps a list of all pending file.
     */
    QDict<HTMLPendingFile> mapPendingFiles;

    /**
     * caches images.
     */
    KHTMLCache *cache;

    /**
     * info about current page: used images
     */
    //QStringList usedImageURLs;

    /**
     * info about current page: href's
     */
    //QStringList usedHrefURLs;

public:
    /**
     * Get info about the current page
     * Caller should delete the returned pageinfo structure.
     */
    //HTMLPageInfo *getPageInfo();

    /**
     * Loads an image into the cache and makes it persistant
     */
    static void preloadImage( QString _filename );

    /** returns the imageCache object */
    //KHTMLCache *imageCache() { return cache; }
    /** set the size of the image cache */
    static void setCacheSize( int size );

    static int cacheSize();
};

#endif // HTML
