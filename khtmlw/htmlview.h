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
//----------------------------------------------------------------------------
//
// KDE HTML Widget
//
// Copyright (c) 1997 The KDE Project
//

#ifndef HTMLVIEW_H
#define HTMLVIEW_H

#include <qscrollbar.h>
#include <qlist.h>
#include <html.h>
#include <drag.h>

/**
 * @short Widget for displaying HTML.  Manages srcollbars and frames.
 *
 * This widget makes displaying HTML text very easy. It handles scrollbars
 * and stuff. It is able to handle most HTML 3.x including tables and frames.
 * To fill the widget with content you should do the follwing:
 * <PRE>
 * view->begin( "file:/tmp/test.html" );
 * view->parse();
 * view->write( "&lt;HTML&gt;&lt;TITLE&gt;...." );
 * .....
 * view->end();
 * view->show();
 * </PRE>
 * The widget will care for resize events and paint events and for scrolling.
 * Have a look at the set of signals emitted by this widget. You should connect
 * to most of them.
 */
class KHTMLView : public QWidget
{
    Q_OBJECT
public:
    /**
     * Created a new HTML View. The widget is empty by default.
     * You must use @ref #begin, @ref #write, @ref #end and @ref #parse
     * to fill the widget with content.
     *
     * @param _name is the name of the widget. Usually this name is only
     *          meaningful for Qt but in this case it is the name of
     *          the HTML window. This means you can reference this name
     *          in the &lt; href=... target=... &gt; tag. If this argument is 0L
     *          then a unique default name is chosen.
     */
    KHTMLView( QWidget *_parent = 0L, const char *_name = 0L, int _flags = 0,
	KHTMLView *_parent_view = 0L );
    /**
     * Destroys the widget and all of its child widgets.
     */
    virtual ~KHTMLView();

    /**
     * Clears the widget and prepares it for new content. If you display
     * for example "file:/tmp/test.html", you can use the following code
     * to get a value for '_url':
     * <PRE>
     * KURL u( "file:/tmp/test.html" );
     * view->begin( u.directoryURL() );
     * </PRE>
     *
     * @param _dx is the initial horizontal scrollbar value. Usually you don't
     *            want to use this.
     * @param _dy is the initial vertical scrollbar value. Usually you don't
     *            want to use this.
     */
    virtual void begin( const char *_url = 0L, int _dx = 0, int _dy = 0 );

    /**
     * Writes another part of the HTML code to the widget. You may call
     * this function many times in sequence. But remember: The less calls
     * the faster the widget is.
     */
    virtual void write( const char *_text );

    /**
     * Call this after your last call to @ref #write.
     */
    virtual void end();

    /**
     * This function will parse the code that has been previously written using
     * the @ref #write function. Call this one after calling @ref #end.
     */
    virtual void parse();

    /**
     * Shows '_url' in this view. Usually a @ref #documentRequest signal is
     * emitted to load the url.
     */
    virtual void openURL( const char *_url );

    /**
     * Prints this view to the printer.
     */
    virtual void print();
    
    /**
     * Creates a new view. This function is not intended to be called by the
     * application that uses this widget. It is intended to be overloaded by
     * some class. If for example you have done this:
     * <PRE>
     * class MyView : public KHTMLView
     * {
     * ...
     *    virtual KHTMLView* newView( QWidget *_parent, const char *_name, int _flags );
     * };
     * </PRE>
     * You may now want to reimplement like this
     * <PRE>
     * KHTMLView* myView::newView( QWidget *_parent, const char *_name, int _flags );
     * {
     *   return new MyView( ... );
     * }
     * </PRE>
     * This will cause that all frames ( if you have some ) will be an instance
     * of MyView, too.<br>
     * FOR INTERNAL USE ONLY.
     */
    virtual KHTMLView* newView( QWidget *_parent = 0L, const char *_name = 0L, int _flags = 0L );
    
    /**
     * Changes the name of the widget.
     * This name is used in the &lt;a href=.. target=... &gt; tag.
     *
     * @see #findView
     * @see #name
     */
    virtual void setFrameName( const char *_name ) { frameName = _name; }

    /**
     * @return the name of this window.
     *
     * @see #setName
     * @see #name
     * @see #findView
     */
    virtual const char* getFrameName() { return frameName.data(); }

    /**
     * Tells the widget that it is a frameset
     * This is for internal use only.
     * FOR INTERNAL USE ONLY.
     */
    void setIsFrameSet( bool _frameset );

    /**
     * FOR INTERNAL USE ONLY.
     *
     * @return TRUE if the view displays a frameset right now.
     */
    bool isFrameSet();

    /**
     * Tells the widget that it is a frame of some frameset.
     * This is for internal use only.
     * FOR INTERNAL USE ONLY.
     */
    void setIsFrame( bool _frame );

    /**
     * FOR INTERNAL USE ONLY.
     *
     * @return TRUE if the view displays a frame right now.
     */
    bool isFrame();

    /**
     * Find the anchor named '_name'. If the anchor is found, the widget
     * scrolls to the closest position. Returns TRUE if the anchor has
     * been found.
     */
    bool gotoAnchor( const char *_name );

    /**
     * Scrolls to the position (_x, _y). Returns TRUE if succeeded.
     */
    bool gotoXY( int _x, int _y );

    /**
     * Sets the width of the border. This is used to implement the tag
     * &lt;frame frameborder=... &gt; tag. FOR INTERNAL USE ONLY.
     * 
     * @see frameBorder
     */
    void setFrameBorder( int _b ) { frameBorder = _b;  }

    /**
     * @return the width of the border in pixels.
     *
     * @see #setFrameBorder
     */
    int getFrameBorder() { return frameBorder; }

    /**
     * Tells the widget to show/hide the scrollbars. This function will have
     * effect only when called before @ref #begin. It is used to implement the
     * <tt>&lt;frame scrolling=... &gt;</tt> tag.<br>
     * FOR INTERNAL USE ONLY.
     *
     * @param _scroll is 1 for yes, 0 for no and -1 for auto.
     *
     * @see #scrolling
     */  
    void setScrolling( int _scroll ) { scrolling = _scroll;  }

    /**
     * @return 1 for yes, 0 for no and -1 for auto.
     *
     * @see #setScrolling
     */
    int getScrolling() { return scrolling; }
  
    /**
     * Tells the widget wether it should be resizeable or not.
     * The widget may still resize. Its only intention is to provide
     * information for @ref HTMLFrameSet. @ref HTMLFrameSet looks at this flag
     * to determine wether the separator between this frame and another one
     * may be moved by the user. It is used to impement the
     * <tt>&lt;frame noresize &gt;</tt> tag.<br>
     * FOR INTERNAL USE ONLY.
     */
    void setAllowResize( bool _allow ) { bAllowResize = _allow; }

    /**
     * This function is used in @ref HTMLFrameSet. It is for
     * INTERNAL USE ONLY.
     *
     * @return TRUE if the widget may be resized by the user.
     *
     * @see #setAllowResize
     */
    bool allowResize() { return bAllowResize; }

    /**
     * Sets the width of the margin. This function is used to implement
     * the <tt>&lt;frame marginwidth=... &gt;</tt> tag.<br>
     * FOR INTERNAL USE ONLY.
     */
    void setMarginWidth( int _w );

    /**
     * Sets the width of the margin. This function is used to implement
     * the <tt>&lt;frame marginheight=... &gt;</tt> tag.<br>
     * FOR INTERNAL USE ONLY.
     */
    void setMarginHeight( int _h );

    /**
     * Tells the widget that it has been selected. This will result
     * in a black border around the widget. This happens only if this
     * widget represents a frame.
     * FOR INTERNAL USE ONLY.
     */
    void setSelected( bool _selected );

    /**
     * FOR INTERNAL USE ONLY.
     *
     * @return TRUE if this widget represents a frame and if it has been
     *         selected by the user. The selected frame has a black inner
     *         border.
     */
    bool isSelected();
    
    /**
     * FOR INTERNAL USE ONLY.
     *
     * @return the x offset of the widget. You may use this function to remeber
     *          which part of the document the user is currently looking at.
     */
    int xOffset();

    /**
     * @return the y offset of the widget. You may use this function to remeber
     *          which part of the document the user is currently looking at.
     */
    int yOffset();
    
    /**
     * Checks out wether there is a URL under the point p and returns a pointer
     * to this URL or 0L if there is none.
     */
    const char* getURL( QPoint & p );

    /**
     * Seaerches for a KHTMLView with a specific name as mentioned in the
     * constructor.
     * 
     * @see #setName
     * @see #name
     */
    KHTMLView* findView( const char *_name );

    /**
     * @return the view that represents the currently selected frame or 0L
     *         if we dont have frames or a selected one right now.
     */
    virtual KHTMLView* getSelectedView();

    /**
     * Select all objects matching the regular expression.
     *
     * @param _select if TRUE then all matching objects are marked, otherwise
     *                they become unmarked.
     */
    virtual void select( QRegExp& _pattern, bool _select );

    /**
     * Gets a list of all selected URLs. The list may be Null.
     * You can test this using list.isNull().
     */
    virtual void getSelected( QStrList &_list );

    /**
     * Get the text the user has marked.
     *
     * @param _str is the QString which will contain the text the user
     * selected.  The selected text is appended to any text currently in
     * _str.
     */
    virtual void getSelectedText( QString &_str );

    /**
     * Has the user selected any text?  Call @ref #getSelectedText to
     * retrieve the selected text.
     *
     * @return true if there is text selected.
     */
     bool isTextSelected() const;

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
     * This function allows you to customize the behavior of the
     * @ref KHTMLWidget.
     * 
     * @return TRUE if the function handled the event, FALSE otherwise.
     *
     * @see #mousePressedHook
     * @see #mouseReleaseHook
     */
    virtual bool mouseMoveHook( QMouseEvent *_ev );

    /**
     * This function allows you to customize the behavior of the
     * @ref KHTMLWidget.
     * 
     * @return TRUE if the function handled the event, FALSE otherwise.
     *
     * @see #mousePressedHook
     * @see #mouseMoveHook
     */
    virtual bool mouseReleaseHook( QMouseEvent *_ev );

    /**
     * This function is called if the user presses the mouse. If he clicks on
     * a link you get the URL in '_url'.
     *
     * @param _url is the clicked URL or 0L is there was none.
     * @param _target is the target frame if one is mentioned otherwise 0L.
     * @param _ev the @ref QMouseEvent. The coordinates of the mouse contained
     *            in this event are relational to the upper left corner of the
     *            @ref KHTMLWidget, not to this widget!
     * @param _isselected is TRUE of the URL '_url' is already selected.
     *
     * @return TRUE if the function handled the event, FALSE otherwise.
     *
     * @see #mouseMoveHook
     * @see #mouseReleaseHook
     */
    virtual bool mousePressedHook( const char* _url, const char *_target,
	QMouseEvent *_ev, bool _isselected );

    /**
     * This function is called if the user wants to start a DND action.
     * Overload this function and return TRUE to indicate that you processed
     * the event. By default the function returns FALSE. This causes the
     * @ref KHTMLWidget to process the event.
     * Usually this function calls @ref KHTMLWidget::startDrag like this:
     * <TT>view->startDrag(....)</TT>. Dont call the startDrag function of any
     * other window. KDND would not like it :-)
     *
     * @param _url is the URL the user wants to drag around.
     * @param _p is the mouse position in global coordinates.
     */
    virtual bool dndHook( const char *_url, QPoint &_p );
  
    /**
     * Selects all objects in this rectangle and deselects all objects
     * outside the rectangle. 
     *
     * @param _rect is a rectangle in display coordinates. This means
     *              that the point (0,0) is the upper/left most point of
     *              the widget but must not be this one for the HTML page.
     *              This happens if the widget is being scrolled.
     * @param _painter is a @ref QPainter or 0L. If it is 0L a new painter is
     *                 created.
     */
    virtual void select( QPainter * _painter, QRect &_rect );

    /**
     * Selects or deselects all objects.
     *
     * @param _painter is a @ref QPainter or 0L. If it is 0L a new painter is
     *                 created.
     */
    virtual void select( QPainter * _painter, bool _select );

    /**
     * Selects all objects which refer to '_url'. All selected objects
     * are redrawn if they changed their selection mode.
     *
     * @param _painter may be 0L. In this case a new @ref QPainter is created
     *                 and destroyed afterwards if no painter already exists.
     */
    void selectByURL( QPainter * _painter, const char *_url, bool _select );
  
    /**
     * @return a pointer to the low-level KHTMLWidget.  You may use this
     * to set default colours, etc. supported by the widget.
     *
     * @see #view
     */
    KHTMLWidget* getKHTMLWidget() { return view; }

    /**
     * @return the parent KHTMLView of this one or 0L is this is the top level
     * view.
     *
     * @see #parentView
     */
    KHTMLView* getParentView() { return parentView; }
    
    /**
     * Never returns 0L.
     */
    KHTMLView* topView();
  
    /**
     * Called when a URL is encountered.  Overload this method to indicate
     * which links have been visited previously.
     *
     * @return true if the URL has been visited previously.  If true is
     * returned the URL will be rendered in the vlink color.  If false
     * is returned the URL will be rendered in the link color.
     */
    virtual bool URLVisited( const char * /*_url*/ ) { return false; }

    /**
     *
     * Sets charset for the View
     */
    void setCharset( const char *_c); 
    
    /**
     * set a charset, which will override the <meta ...> setting
     */
    void setOverrideCharset(const char *name = 0);

    /**
     * For internal use only.
     *
     * @see #cookie
     */
    const char* getCookie() { return cookie.data(); }

    /**
     * For internal use only.
     *
     * @see #cookie
     */
    void setCookie( const char *_c )
	{ printf("Setting cookie '%s'\n",_c); cookie = _c; }

    void cancelAllRequests();

    // Another option to feed image data into the HTML Widget
    void data( const char *_url, const char *_data, int _len, bool _eof );

signals:
    /**
     * This signal is emitted if we deal with frames ( or one of the child
     * widgets! ).
     * It tells the owner of the widget to provide a certain URL for a widget.
     * The owner has to call <PRE>
     * _widget->begin( .. );
     * _widget->parse();
     * _widget->write( .. ); ....
     * _widget->end();
     * </PRE>
     * This may happen before returning ( directly after receiving this
     * signal ), or some time later on. You may be notified that the URL
     * is no longer wanted. This is done with the
     * signal @ref #cancelDocumentRequest.
     *
     * @param _widget is the widget that requests the document.
     * @param _url is the URL where the document can be found.
     *
     * @see #cancelDocumentRequest
     * @see #slotDocumentRequest
     */
    void documentRequest( KHTMLView* _widget, const char* _url );
    
    /**
     * This signal is emitted if the widget does not need the document any more.
     *
     * @param _url is the URL of the document that we dont need any more.
     */
    void cancelDocumentRequest( KHTMLView *_view, const char *_url );
 
    /**
     * This signal is emitted whenever the widget wants to
     * change the windows title. Usually this is the text
     * enclosed in the title tag.
     *
     * @param _title is a string of ASCII characters.
     */
    void setTitle( const char *_title );

    /**
     * Signals that the URL '_url' has been selected. The user used
     * the mouse button '_button' for this.
     *
     * @param _url is a full qualified URL.
     * @param _button is <tt>LeftButton, MiddleButton or RightButton</tt>
     * @param _target is the target window or 0L if none is specified.
     */
    void URLSelected( KHTMLView *_view, const char *_url, int _button,
	const char *_target );

    /**
     * Signals that the mouse cursor is over URL '_url'.
     * If '_url' is null then cursor moved off a URL
     *
     * @param _url is a full qualified URL.
     */
    void onURL( KHTMLView *_view, const char *_url );

    /**
     * Signal that the user has selected text or the existing selection has
     * become unselected.  The text may be retrieved using
     * @ref #getSelectedText.  This is a good signal to connect to for
     * enabling/disabling the Copy menu item or calling XSetSelectionOwner().
     *
     * @param _selected is true if the user has selected text or false if
     * the current selection has been removed.
     */
    void textSelected( KHTMLView *_view, bool _selected );

    /**
     * This signal is Emitted if the user pressed the right mouse button
     * over an URL. '_url' may be 0L to indicate the the user pressed
     * the right mouse button over an area that does not have an anchor
     * tag.
     *
     * @param _url is a full qualified URL or 0L.
     * @param _point is the position where the user pressed the mouse button.
     *               This point is already in global cooredinates.
     */
    void popupMenu( KHTMLView *_view, const char *_url, const QPoint &_point );

    /**
     * This signal is emitted if the widget requests to load an image.
     * KHTMLWidget can only load image from your local disk. If it
     * finds an image with another protocol in its URL, it will emit this
     * signal. If the image is loaded at some time, call @ref #slotImageLoaded.
     * If the image is not needed any more, the signal @ref #cancelImageRequest
     * is emitted. It may happen that not the widget itself but one of its
     * children requests the image. To distibuish that you get '_view'.
     *
     * @see #cancelImageRequest
     */
    void imageRequest( KHTMLView *_view, const char *_url );

    /**
     * This signal is only emitted if this widget and not one of its children
     * requests an image.
     */
    void imageRequest( const char *_url );
    
    /**
     * Cancels an image that has been requested before.
     *
     * @see #imageRequest
     */
    void cancelImageRequest( KHTMLView *_view, const char *_url );

    /**
     * This signal is only emitted if this widget and not one of its children
     * cancels a requested image.
     */
    void cancelImageRequest( const char *_url );

    /**
     * The user pressed a forms submit button.
     *
     * @param _url is the <form action=...> value
     * @param _method is the <form method=...> value
     * @param _data is the encoded data to send 
     */
    void formSubmitted( KHTMLView *_view, const char *_method,
	const char *_url, const char *_data );

    /**
     * Emitted if the user presses the mouse button over this widget and if
     * the widget is a frame. In this case this frame became the selected one
     * and this signal is used to tell our parent about this.
     *
     * @see #setSeleced
     */
    void frameSelected( KHTMLView *_view );

    /**
     * The widget started working. You may use this signal to implement an
     * animated logo like netscape has one.
     *
     * @see #documentDone
     */
    void documentStarted( KHTMLView *_view );

    /**
     * This signal is emitted if the widget has parsed and if all images
     * arrived. Mention that this is only true or '_view'. The parent widget
     * or any child widget may still parse or wait for an image.
     * This signal is the complement to @ref #documentStarted
     */
    void documentDone( KHTMLView *_view );

    /**
     * The user pressed ALT + Up
     */
    void goUp( KHTMLView *_view );
    void goUp();
    /**
     * The user pressed ALT + Right
     */
    void goRight( KHTMLView *_view );
    void goRight();
    /**
     * The user pressed ALT + Left
     */
    void goLeft( KHTMLView *_view );
    void goLeft();
  
public slots:
    void slotScrollVert( int _dy );
    void slotScrollHorz( int _dx );
  
    /** Call this slot if an requested image is available.
     * The rquested image is named '_url' and is stored on the local disk
     * in the file named '_filename'.
     *
     * @param _url is the full qualified URL that was passedd to you by the
     *             ref #imageRequest signal.
     * @param _filename is a usual UNIX filename like "/tmp/tmpimage".
     */
    void slotImageLoaded( const char *_url, const char *_filename );

    /**
     * This slot is connected to all children. It emits the signal
     * @ref #documentRequest if called. <b>This slot is for internal use
     * only</b>.
     */
    void slotDocumentRequest( KHTMLView* _view, const char* _url );

    /**
     * This slot is connected to all children. It emits the signal
     * @ref #cancelDocumentRequest if called. <b>This slot is for internal use
     * only</b>.
     */
    void slotCancelDocumentRequest( KHTMLView* _view, const char* _url );

    /**
     * This slot is connected to all children. It emits the signal
     * @ref #URLSelected if called. <b>This slot is for internal use only</b>.
     */
    void slotURLSelected( KHTMLView* _view, const char* _url, int _button,
	const char *_target );

    /**
     * This slot is connected to all children. It emits the signal
     * @ref #setTitle if called. <b>This slot is for internal use only</b>.
     */
    void slotSetTitle( const char* _text );

    /**
     * This slot is connected to all children. It emits the signal
     * @ref #onURL if called. <b>This slot is for internal use only</b>.
     */
    void slotOnURL( KHTMLView *_view, const char *_url );

    /**
     * This slot is connected to all children. It emits the signal
     * @ref #popupMenu if called. <b>This slot is for internal use only</b>.
     */
    void slotPopupMenu( KHTMLView *_view, const char *_url,
	const QPoint &_point );

    /**
     * This slot is connected to all children. It emits the signal
     * @ref #imageRequest if called. <b>This slot is for internal use only</b>.
     */
    void slotImageRequest( KHTMLView *_view, const char *_url );

    /**
     * This slot is connected to all children. It emits the signal
     * @ref #cancelImageRequest if called. <b>This slot is for internal use
     * only</b>.
     */
    void slotCancelImageRequest( KHTMLView *_view, const char *_url );

    /**
     * This slot is connected to all children. It emits the signal
     * @ref #cancelImageRequest if called. <b>This slot is for internal use
     * only</b>.
     *
     * @param _method is the <form method=...> value
     * @param _url is the <form action=...> value
     * @param _data is the encoded from data submitted
     */
    void slotFormSubmitted( KHTMLView *_view, const char *_method,
	const char *_url, const char *_data );

    /**
     * This slot is connected to all children. It emits the signal
     * @ref #documentStarted if called. <b>This slot is for internal use
     * only</b>.
     */
    void slotDocumentStarted( KHTMLView *_view );

    /**
     * This slot is connected to all children. It emits the signal
     * @ref #documentDone if called. <b>This slot is for internal use only</b>.
     */
    void slotDocumentDone( KHTMLView *_view );

    /**
     * You can move the vertical scrollbar by calling this slot.
     */
    void slotVertSubtractLine();

    /**
     * You can move the vertical scrollbar by calling this slot.
     */
    void slotVertAddLine();

    /**
     * You can move the vertical scrollbar by calling this slot.
     */
    void slotVertSubtractPage();

    /**
     * You can move the vertical scrollbar by calling this slot.
     */
    void slotVertAddPage();

protected slots:    

    /**
     * This slot is connected to @ref KHTMLWidget. It emits the signal
     * @ref #URLSelected if called. <b>This slot is for internal use only</b>.
     */
    virtual void slotURLSelected( const char* _url, int _button,
	const char *_target );

    /**
     * This slot is connected to @ref KHTMLWidget. It emits the signal
     * @ref #onURL if called. <b>This slot is for internal use only</b>.
     */
    virtual void slotOnURL( const char *_url );

    /*
     * This slot is connected to @ref KHTMLWidget. It emits the signal
     * @ref #textSelected if called.  This slot is for internal use only.
     */
    virtual void slotTextSelected( bool _selected );

    /**
     * This slot is connected to @ref KHTMLWidget. It emits the signal
     * @ref #popupMenu if called. <b>This slot is for internal use only</b>.
     */
    virtual void slotPopupMenu( const char *_url, const QPoint &_point );

    /**
     * This slot is connected to @ref KHTMLWidget. It emits the signal
     * @ref #imageRequest if called. <b>This slot is for internal use only</b>.
     */
    virtual void slotImageRequest( const char *_url );

    /**
     * This slot is connected to @ref KHTMLWidget. It emits the signal
     * @ref #cancelImageRequest if called. <b>This slot is for internal use
     * only</b>.
     */
    virtual void slotCancelImageRequest( const char *_url );

    /**
     * This slot is connected to @ref KHTMLWidget. It emits the signal
     * if the form is submitted
     *
     * @param _url is the <form action=...> value
     * @param _method is the <form method=...> value
     * @param _data is the encoded form data submitted 
     */
    virtual void slotFormSubmitted( const char *_url, const char *_method, const char *_data );

    /**
     * This slot is connected to @ref KHTMLWidget. It emits the signal
     * if the document in the HTML Widget changes.<br>
     * <b>For Internal use only</b>.
     */
    virtual void slotDocumentChanged();

    /**
     * This slot is connected to @ref KHTMLWidget. It emits the signal
     * @ref #documentStarted if called. <b>This slot is for internal use
     * only</b>.
     */
    virtual void slotDocumentStarted();

    /**
     * This slot is connected to @ref KHTMLWidget. It emits the signal
     * @ref #documentDone if called. <b>This slot is for internal use only</b>.
     */
    virtual void slotDocumentDone();
       
    /**
     * Connected to the HTML widget.
     */
    void slotInternScrollVert( int );

    /**
     * Connected to the HTML widget
     */
    void slotInternScrollHorz( int );

    /**
     * The user pressed ALT + Up
     */
    void slotGoUp();
    /**
     * The user pressed ALT + Right
     */
    void slotGoRight();
    /**
     * The user pressed ALT + Left
     */
    void slotGoLeft();
    
protected:
    virtual void resizeEvent( QResizeEvent* _ev );
    virtual void closeEvent( QCloseEvent *e );

    virtual void initGUI();

    /*
     * Determiens wether scrollbars are needed and displays or hides them
     */
    virtual void calcScrollBars();
  
    /*
     * The horizontal scrollbar
     */    
    QScrollBar *horz;

    /*
     * The vertical scrollbar
     */    
    QScrollBar *vert;

    /*
     * The HTML Widget.
     */
    KHTMLWidget *view;

    /**
     * The currently displayed URL.
     * This is usually the same URL as the one passed to the Constructor of
     * @ref KIDWIindow::KIDWindow.
     */
    QString url;

    /*
     * Flag that indicates wether we show a horizontal scrollbar.
     *
     * @see horz
     */
    bool displayHScroll;

    /*
     * Flag that indicates wether we show a vertictal scrollbar.
     *
     * @see vert
     */
    bool displayVScroll;

    /*
     * List of all open html views.
     */
    static QList<KHTMLView> *viewList;

    /*
     * The name of the window. You can refer to this name in the
     * &lt;a href=... target=... &gt> tag.
     *
     * @see #findView
     * @see #setName
     * @see #getName
     */
    QString frameName;

    /*
     * @see #setAllowResize
     */
    bool bAllowResize;

    /*
     * @see #setScrolling
     */
    int scrolling;

    /*
     * @see #setFrameBorder
     */
    int frameBorder;

    /*
     * Pointer to the parent KHTMLView. If this view is the toplevel frame or
     * if there are no frames at all then this pointer is 0L.
     */
    KHTMLView *parentView;

    /*
     * This string holds some temporary data. It is used by @ref KHTMLWidget
     *
     * @see #setCookie
     * @see #getCookie
     */
    QString cookie;              

    int scrollToX;
    int scrollToY;
};

#endif



