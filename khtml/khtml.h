/* This file is part of the KDE project

   Copyright (C) 1997 Martin Jones (mjones@kde.org)
             (C) 1998, 2000 Waldo Bastian (bastian@kde.org)
             (C) 1998, 1999 Torben Weis (weis@kde.org)
             (C) 1999 Lars Knoll (knoll@kde.org)
	     (C) 1999 Antti Koivisto (koivisto@kde.org)

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

#ifndef KHTML_H
#define KHTML_H

// constants used when dragging a selection rectange outside the browser window
#define AUTOSCROLL_DELAY 150
#define AUTOSCROLL_STEP 20

#define MAX_REQUEST_JOBS 4

// qt includes and classes
#include <qscrollview.h>
#include <qstring.h>
#include <qdatastream.h>

class QPainter;

// khtml includes and classes
#include "dom/html_document.h"

// ### FIXME: get rid of this include
#include "khtmlio.h"

namespace DOM {
    class HTMLDocumentImpl;
    class HTMLElementImpl;
    class HTMLTitleElementImpl;
    class NodeImpl;
};

class KHTMLCache;
class KHTMLDecoder;

namespace khtml {
  class Settings;
};

// JScript class
class KJSWorld;


/**
 * This class is khtml's main class. It features an almost complete
 * web browser, and html renderer.
 *
 * The easiest way to use this class (if you just want to display a HTML
 * page at some URL) is the following:
 *
 * <pre>
 * QString url = "http://www.kde.org";
 * KHTMLWidget *w = new KHTMLWidget();
 * w->openURL(url);
 * w->resize(500, 400);
 * w->show();
 * </pre>
 *
 * By default the Widget behaves as a full browser, so clicking on some link
 * on the page you just opened will lead yu to that page. This is inconvenient,
 * if you want to use the widget to display for example formatted emails, but
 * don't want the widget to open the site in this window in case someone
 * clicks on an embedded link. In this case just use
 * @ref setFollowsLinks(false). You will then get a Signal @ref urlClicked()
 * instead of KHTMLWidget following the links directly.
 *
 * By default Java and JavaScript support is disabled. You can enable it by
 * using the @ref enableJava() and @ref enableJScript() methods.
 *
 * Some apps want to write their HTML code directly into the widget instead of
 * it opening an url. You can also do that in the following way:
 *
 * <pre>
 * QString myHTMLCode = ...;
 * KHTMLWidget *w = new KHTMLWidget();
 * w->begin();
 * w->write(myHTMLCode);
 * ...
 * w->end();
 * </pre>
 *
 * You can do as many calls to write as you want. But there are two @ref write()
 * methods, one accepting a @ref QString one accepting a char * argument. These
 * should not get mixed, since the method usnig the char * argument does an
 * additional decoding step to convert the written data to Unicode.
 *
 * If you derive from KHTMLWidget you must overload the method @ref #createFrame
 *
 * @short HTML Browser Widget
 * @author Lars Knoll (knoll@kde.org)
 * @version $Id$
 *
 */
class KHTMLWidget : public QScrollView
{
    Q_OBJECT

    friend HTMLURLRequestJob;
    friend DOM::HTMLDocumentImpl;
    friend DOM::HTMLElementImpl;
    friend DOM::HTMLTitleElementImpl;

public:

    /**
     * Constructs a KHTMLWidget
     */
    KHTMLWidget( QWidget *parent=0, const char *name=0 );

    /**
     * This constructor is used internally to realize Frames
     */
    KHTMLWidget( QWidget *parent, KHTMLWidget *parentWidget, QString name);
    virtual ~KHTMLWidget();

protected:
    void init();
    void clear();
public:

    /**
     * Tell the widget to display the HTML page referred to by _url.
     *
     * @param xoffset, yoffset Show the page at the specified offset after
     *        loading it.
     * @param _post_data used internally for HTML post request
     */
    virtual void openURL( const QString &_url, bool _reload = false,
		 int _xoffset = 0, int _yoffset = 0, const char* _post_data = 0L );

    /**
     * should the widget follow links automatically, if you click on them?
     * Default is true.
     */
    void setFollowsLinks( bool follow );
    /** does the widget follow links automatically?
     */
    bool followsLinks();

    /**
     * should images be loaded automatically? Default is true.
     * (not implemented at the moment)
     */
    void enableImages( bool enable );
    bool imagesEnabled() const;

    /**
     * Enable disable Java. Default is disabled.
     */
    void enableJava( bool enable );
    /**
     * Is Java enabled?
     */
    bool javaEnabled() const;

    /**
     * enable/disable JScript. Default is disabled.
     */
    void enableJScript( bool enable );
    /**
     * Is JScript enabled?
     */
    bool jScriptEnabled() const;

    /**
     * @return the parent KHTMLWidget of this one or 0L is this is the top level
     * browser. Used for frames.
     */
    KHTMLWidget* parentFrame() { return _parent; }

    /**
     * Returns the top level frame. Never returns 0L.
     */
    KHTMLWidget* topFrame();

    /**
     * Searches for a KHTMLWidget with a specific name as mentioned in the
     * constructor.
     *
     * @see #setName
     * @see #name
     */
    KHTMLWidget* findFrame( const QString &_name );

    /**
     * @return the name of this frame.
     *
     * @see #setName
     * @see #name
     * @see #findView
     */
    virtual QString frameName() { return m_strFrameName; }

    /**
     * This function is used by @ref #newView. Its only purpose is to create
     * a new instance of this class. If you derived from KHTMLWidget you must
     * overload this function to make shure that all frames are of the same
     * derived class.
     */
    virtual KHTMLWidget* createFrame( QWidget *_parent, QString _name );

    /**
     * This function is mainly used internally. It gets the frame with name _name,
     *  if it exists and is a child of this widget, otherwise return 0.
     */
    KHTMLWidget *getFrame( QString _name);

    /**
     * Clears the widget and prepares it for new content.
     * If you want @ref url() to return
     * for example "file:/tmp/test.html", you can use the following code:
     * <PRE>
     * view->begin( QString("file:/tmp/test.html" ) );
     * </PRE>
     *
     * @param _url is the url of the document to be displayed.  Even if you
     * are generating the HTML on the fly, it may be useful to specify
     * a directory so that any pixmaps are found.
     * @param _dx is the initial horizontal scrollbar value. Usually you don't
     * want to use this.
     * @param _dy is the initial vertical scrollbar value. Usually you don't
     * want to use this.
     *
     * All child frames and the old document are removed if you call this method.
     */	
    virtual void begin( const QString &_url = 0L, int _dx = 0, int _dy = 0 );

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
     * Print current HTML page layouted for the printer.
     * (not implemented at the moment)
     */
    // ### add margins???
    void print(QPainter *, int pageHeight, int pageWidth);

    /**
     * Selects all text between ( _x1, _y1 ) and ( _x2, y2 ).  The selection
     * area selects text line by line, NOT by bounding rectangle.
     */
    //virtual void selectText( int _x1, int _y1, int _x2, int _y2 );

    /**
     * Get the text the user has marked.
     */
    virtual QString selectedText();

    /**
     * Has the user selected any text?  Call @ref #selectedText to
     * retrieve the selected text.
     *
     * @return true if there is text selected.
     */
    bool isTextSelected() const
    {	return bIsTextSelected; }

    /**
     * Get all text in the HTML page.
     */
    //virtual QString text();

    /**
     * Initiate a text search.
     */
    void findTextBegin();

    /**
     * Find the next occurrance of the expression.
     */
    bool findTextNext( const QRegExp &exp );

    /**
     * Checks out whether there is a URL under the point and returns a pointer
     * to this URL or 0L if there is none.
     *
     * @param _point the point to test for the presence of a URL.  The
     * point is relative to this widget.
     */
    const QString &url( const QPoint &_point ) const;

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
    const QString &url() { return m_strURL; }

    /**
     * @return the base URL of this document
     *
     * The base url is ususally set by an <base url=...> tag in the document head.
     */
    const QString &baseUrl();

    /**
     * Mainly used internally. Sets the document's base URL
     */
    void setBaseUrl(const QString &base);

    /**
     * @return the base target of this document
     * The base target is ususally set by an <base target=...>
     * tag in the document head.
     */
    const QString &baseTarget() { return _baseTarget; }
    /**
     * Mainly used internally. Sets the document's base target.
     */
    void setBaseTarget(const QString &target) { _baseTarget = target; }

    /**
     * Find the anchor named '_name'. If the anchor is found, the widget
     * scrolls to the closest position. Returns TRUE if the anchor has
     * been found.
     */
    virtual bool gotoAnchor( const QString &_name );

    /**
     * Causes the widget contents to scroll automatically.  Call
     * @ref #stopAutoScrollY to stop.  Stops automatically when the
     * top or bottom of the document is reached.
     *
     * @param _delay Time in milliseconds to wait before scrolling the
     * document again.
     * @param _dy The amount to scroll the document when _delay elapses.
     *
     * (not implemented)
     */
    void autoScrollY( int _delay, int _dy );

    /**
     * Stops the document from @ref #autoScrollY ing.
     */
    //void stopAutoScrollY();

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

    bool frameIsSelected() { return _isSelected; }
    void setFrameSelected(KHTMLWidget *w = 0);

    /**
     * @return a pointer to the currently selected frame if
     * we are displaying a frameset, otherwise 0L. If this widget is the
     * selected one then @ref htmlView is returned. Otherwise all
     * @ref HTMLFrameSet instances are asked.
     */
    KHTMLWidget* selectedFrame();

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
    void fontSizes(int *newFontSizes, int *newFixedFontSizes=0);

    /**
     * Resets the point sizes to be associated with the HTML-sizes used in
     * <FONT size=Html-Font-Size> to their default.
     *
     * Html-Font-Sizes range from 1 (smallest) to 7 (biggest).
     */
    void resetFontSizes();

    /**
     * Sets the standard font style.
     *
     * @param name is the font name to use for standard text.
     */
    void setStandardFont( const QString &name );

    /**
     * Sets the fixed font style.
     *
     * @param name is the font name to use for fixed text, e.g.
     * the <tt>&lt;pre&gt;</tt> tag.
     */
    void setFixedFont( const QString &name );

    /**
     * Set document charset.
     *
     * Any <META ...> setting charsets overrides this setting
     * as long as override isn't true.
     * @return TRUE if successfull
     *
     * (not implemented at the moment)
     */
    bool setCharset(const QString &name, bool override = false);

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
    const QCursor& urlCursor() { return linkCursor; }

    /*
     * If an Element needs a file from the web, it
     * calls this function.
     * if update is true, the Element will be continously updated
     * as the file is loaded (via the data function)
     */
    void requestFile( HTMLURLRequester *_obj, const QString &_url,
		      bool update = false );

    /*
     * Cancels a previous @ref requestFile.
     */
    void cancelRequestFile( HTMLURLRequester *_obj );
    void cancelRequestFile( const QString &_url );

    /**
     * cancels all file requests. Called from @see #slotCancel()
     */
    void cancelAllRequests();

    /**
     * Function used to save the current html-page into the datastream
     * This does only work, if the page has a valid URL, pages
     * filled with the write() method are not saveable, and might give
     * unwanted results.
     */
    virtual void saveState( QDataStream &stream );

    /**
     * restore a page previously saved with @ref saveState()
     */
    virtual void restoreState( QDataStream &stream );

    /**
     * hook to get the Document. Used eg by jscript to manipulate the document.
     * See also the <a href="http://www.w3.org/TR/REC-DOM-Level-1/">
     * DOM Level 1 recommodation</a> of the <a href=http://www.w3.org>W3C</a>.
     */
    DOM::HTMLDocument htmlDocument() const { return document; }

    /**
     * @return the JavaScript engine, or 0 if JScript is disabled.
     */
    KJSWorld *jScript();

    /**
     * Return the job id of the KIOJob responsible for loading the current
     * document (or 0 if none)
     * @see m_jobId
     */
    int jobId() const { return m_jobId; }

    /**
     * set a margin in x direction
     */
    void setMarginWidth(int x) { _marginWidth = x; }
    /**
     * @return the margin With
     */
    int marginWidth() { return _marginWidth; }

    /**
     * set a margin in y direction
     */
    void setMarginHeight(int y) { _marginHeight = y; }
    /**
     * @return the margin height
     */
    int marginHeight() { return _marginHeight; }

    khtml::Settings *settings();

    void scheduleRedirection(int delay, const QString & url);

public slots:
    /**
     * stops loading the current document
     */
    virtual void slotStop();
    /**
     * reload the current document. Will not reload frames contained in this
     * document in case it is a frameset.
     */
    virtual void slotReload();
    /**
     * reload the current document including all child frames.
     */
    virtual void slotReloadFrames();
    /**
     * used internally to tell the widget, that a form has been submitted.
     */
    void slotFormSubmitted( const QString &_method, const QString &_url,
			    const char *_data, const QString &_target );

    /**
     * @internal
     */
    void slotPost( int );

signals:
    /**
     * Emitted when cancel is emitted, but with more detailed error
     * description.
     */
    void error( int _err, const char* _text );
    /**
     * Emitted if a link is pressed which has an invalid target,
     * or the target <tt>_blank</tt>.
     */
    void newWindow( const QString &_url );

    /**
     * emitted, after @see #begin() has been called.
     */
    void started( const QString &_url );
    /**
     * emitted, when the page (including all children) has been completely loaded.
     */
    void completed();
    /**
     * emitted, when the loading o the page has been canceled.
     */
    void canceled();

    /**
     * this signal is only emitted, when @see followLinks() is set to false.
     * In this case the app has to handle the request.
     */
    void urlClicked( const QString &url , const QString &target, int button);

    /**
     * Signals that the mouse cursor has moved on or off a URL.
     *
     * @param _url is the URL that the mouse cursor has moved onto.
     * _url is null if the cursor moved off a URL.
     */
    void onURL( const QString &_url );

    /**
     * If the user pressed the right mouse button over a URL than _url
     * points to this URL, otherwise _url will be null.
     * The position is in global coordinates.
     */
    void popupMenu( const QString &_url, const QPoint & );

    /**
     * This signal is emitted whenever the Widget wants to
     * change the window's title. Usually this is the text
     * enclosed in <tt>&lt;title&gt;....&lt;/title&gt;</tt>.
     */
    void setTitle( const QString &);

    /**
     * Emitted if the user presses the submit button.
     *
     * @param _url is the <form action=...> value
     * @param _method is the <form method=...> value
     */
    void formSubmitted( const QString &_method, const QString &_url,
			const char *_data, const QString &_target );

    void frameInserted( KHTMLWidget *frame );

    void textSelected( bool );


protected slots:
    void slotData( int _id, const char* data, int _len );
    void data( HTMLURLRequestJob *job, const char *_data, int _len, bool _eof );
    void slotFinished( int _id );
    void slotError( int _id, int _err, const char *_text );

    // gets called for redirection triggered by the http header
    void slotRedirection( int _id, const char *_url );

    // used for <meta> redirection
    void slotRedirect();

    //virtual void slotURLRequest( const QString & _url );
    //virtual void slotCancelURLRequest( const QString & _url );
    //virtual void slotDocumentFinished( KHTMLWidget* _browser );

    /*
     * INTERNAL
     *
     * Called by timer event when the widget is due to autoscroll.
     */
    //void slotAutoScrollY();

    /*
     * INTERNAL
     *
     * Used to update the selection when the user has caused autoscrolling
     * by dragging the mouse out of the widget bounds.
     */
    //void slotUpdateSelectText( int newy );

protected:
    virtual void servePendingURLRequests();
    virtual void urlRequestFinished( HTMLURLRequestJob* _request );

    void paintElement( DOM::NodeImpl *e, bool recursive=false );
    virtual void resizeEvent ( QResizeEvent * event );
    virtual void viewportPaintEvent ( QPaintEvent* pe  );
public:
    void layout();
protected:

    /**
     * This function is called, when a user action triggers a script.
     */
    virtual void executeScript( const QString &script );

    void urlSelected( const QString &_url, int _button, const QString & _target );

    /**
     * @return the parent KHTMLWidget of this one or 0L is this is the top level
     * browser.
     *
     */
    KHTMLWidget* parentView() { return _parent; }

    /**
     * Never returns 0L.
     */
    KHTMLWidget* topView();

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

    void keyPressEvent( QKeyEvent *_ke );

    /**
     * This function is hooked into the event processing of the widget.
     */
    virtual bool mouseMoveHook( QMouseEvent * ) { return false; }
    /**
     * This function is hooked into the event processing of the widget.
     */
    virtual bool mouseReleaseHook( QMouseEvent * ) { return false; }
    /**
     * This function is hooked into the event processing of the widget.
     */
    virtual bool mousePressedHook( const QString &/* _url*/, const QString & /*_target*/, QMouseEvent *, bool /*_isselected*/) { return false; }

public:
    virtual QString completeURL( const QString & _url, const QString &target = QString::null );
protected:

    virtual KHTMLWidget* findChildView( const QString & _target );

    struct Child
    {
	Child( KHTMLWidget *_b, bool _r ) { m_pBrowser = _b; m_bReady = _r; }

	KHTMLWidget* m_pBrowser;
	bool m_bReady;
    };

    virtual void childCompleted( KHTMLWidget* _browser );

    // checks if loading is done and emits the completed() signal if true
    void checkCompleted();

    // ------------------------------------- member variables ------------------------------------

    KHTMLWidget* _parent;
    QString m_strFrameName;
    DOM::HTMLDocumentImpl *document;


    /**
     * A list containing all direct child views. Usually every frame in a frameset becomes
     * a child view. This list is used to update the contents of all children ( see @ref #slotUpdateView ).
     *
     * @see newView
     */
    QList<Child> m_lstChildren;

    /*
     * List of all open browsers.
     */
    static QList<KHTMLWidget> *lstViews;

    /**
     * This flag is TRUE if we are in the middle of a selection using a
     * rectangular rubber band.
     */
    bool m_bStartedRubberBand;
    /**
     * This flag is true if the rubber band is currently visible.
     */
    bool m_bRubberBandVisible;
    /**
     * Painter used for the rubber band.
     */
    QPainter* m_pRubberBandPainter;

    /**
     * This is just a temporary variable. It stores the URL the user clicked
     * on, until he releases the mouse again.
     *
     * @ref #mouseMoveHook
     * @ref #mousePressedHook
     */
    QString m_strSelectedURL;

    QString m_strURL;

    /**
     * Once we received the first data of our new HTML page we clear this
     * variable and copy its content to @ref #m_strURL
     */
    QString m_strWorkingURL;

    /**
     * One can pass offsets to @ref #openURL to indicate which part of the
     * HTML page should be visible. This variable is used to store the XOffset.
     * It is used when the method @ref #begin is called which happens once the
     * first data package of the new HTML page arrives.
     */
    int m_iNextXOffset;
    /**
     * One can pass offsets to @ref #openURL to indicate which part of the
     * HTML page should be visible. This variable is used to store the XOffset.
     * It is used when the method @ref #begin is called which happens once the
     * first data package of the new HTML page arrives.
     */
    int m_iNextYOffset;

    /**
     * This is the id of the job that fetches the HTML page for us.
     * A value of 0 indicates that we have no running job.
     */
    int m_jobId;

    /**
     * This flag is set to true after calling the method @ref #begin and
     * before @ref #end is called.
     */
    bool m_bParsing;


    QDict<HTMLURLRequest> m_lstPendingURLRequests;
    QList<HTMLURLRequestJob>  m_lstURLRequestJobs;


    /**
     * This flag is set to false after a call to @ref #openURL and set to true
     * once the document is parsed and displayed and if all children ( frames )
     * are complete, too. On completion the signal @ref #completed is emitted
     * and the parent browser ( @ref #m_pParentBrowser ) is notified.
     * If an fatal error occurs, this flag is set to true, too.
     */
    bool m_bComplete;

    /**
     * Tells wether the last call to @ref #openURL had the reload flag set or not.
     * This is needed to use the same cache policy for loading images and stuff.
     */
    bool m_bReload;

    /**
     * This flag indicates whether images should be loaded automatically.
     * The default value is TRUE.
     * see also: @ref enableImages and @ref imagesEnabled
     */
    bool m_bEnableImages;

private:

    khtml::Settings *defaultSettings;
    khtml::Settings *_settings;

    bool bIsTextSelected;
    bool _isFrame;
    bool _isSelected;
    QTimer autoScrollYTimer;
    QCursor linkCursor;
    int press_x;
    int press_y;

    QString _baseURL;
    QString _baseTarget;

    KHTMLCache *cache;
    KHTMLDecoder *decoder;

    bool pressed;
    QString overURL;
    int _width;
    bool _followLinks;
    bool _javaEnabled;
    bool _jScriptEnabled;

    KJSWorld *_jscript;

    int _marginWidth;
    int _marginHeight;

    // helpers for find functionality
    int findPos;
    DOM::NodeImpl *findNode;

    QCString post_data;

    static QPixmap* paintBuffer;

    int m_delayRedirect;
    QString m_strRedirectUrl;
};

#endif

