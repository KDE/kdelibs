// -*- c-basic-offset: 2 -*-
/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999-2001 Lars Knoll <knoll@kde.org>
 *                     1999-2001 Antti Koivisto <koivisto@kde.org>
 *                     2000-2001 Simon Hausmann <hausmann@kde.org>
 *                     2000-2001 Dirk Mueller <mueller@kde.org>
 *                     2000 Stefan Schimanski <1Stein@gmx.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef __khtml_part_h__
#define __khtml_part_h__

#include "dom/html_document.h"
#include "dom/dom2_range.h"

#include <kparts/part.h>
#include <kparts/browserextension.h>
#include <kdemacros.h>
#include <kfind.h>
#include <kfinddialog.h>
#include <qregexp.h>

class KHTMLPartPrivate;
class KHTMLPartBrowserExtension;
class KJSProxy;
class KHTMLView;
class KHTMLSettings;
class KJavaAppletContext;
class KJSErrorDlg;

namespace DOM
{
  class HTMLDocument;
  class HTMLDocumentImpl;
  class DocumentImpl;
  class HTMLTitleElementImpl;
  class HTMLElementImpl;
  class HTMLFrameElementImpl;
  class HTMLIFrameElementImpl;
  class HTMLObjectElementImpl;
  class HTMLFormElementImpl;
  class HTMLAnchorElementImpl;
  class HTMLMetaElementImpl;
  class NodeImpl;
  class Node;
  class HTMLEventListener;
  class EventListener;
}

namespace KJS
{
  class Interpreter;
}

namespace khtml
{
  class DocLoader;
  class RenderPart;
  class RenderPartObject;
  class ChildFrame;
  class MouseEvent;
  class MousePressEvent;
  class MouseDoubleClickEvent;
  class MouseMoveEvent;
  class MouseReleaseEvent;
  class DrawContentsEvent;
  class CachedObject;
  class RenderWidget;
  class CSSStyleSelector;
  class HTMLTokenizer;
  class Decoder;
  class XMLTokenizer;
}

namespace KJS {
    class Window;
    class WindowFunc;
    class ExternalFunc;
    class JSEventListener;
    class JSLazyEventListener;
    class JSNodeFilter;
    class DOMDocument;
    class SourceFile;
    class ScheduledAction;
}

namespace KParts
{
  class PartManager;
  class LiveConnectExtension;
}

namespace KWallet
{
  class Wallet;
}

/**
 * This class is khtml's main class. It features an almost complete
 * web browser, and html renderer.
 *
 * The easiest way to use this class (if you just want to display an HTML
 * page at some URL) is the following:
 *
 * \code
 * KURL url = "http://www.kde.org";
 * KHTMLPart *w = new KHTMLPart();
 * w->openURL(url);
 * w->view()->resize(500, 400);
 * w->show();
 * \endcode
 *
 * Java and JavaScript are enabled by default depending on the user's
 * settings. If you do not need them, and especially if you display
 * unfiltered data from untrusted sources, it is strongly recommended to
 * turn them off. In that case, you should also turn off the automatic
 * redirect and plugins:
 *
 * \code
 * w->setJScriptEnabled(false);
 * w->setJavaEnabled(false);
 * w->setMetaRefreshEnabled(false);
 * w->setPluginsEnabled(false);
 * \endcode
 *
 * You may also wish to disable external references.  This will prevent KHTML
 * from loading images, frames, etc,  or redirecting to external sites.
 *
 * \code
 * w->setOnlyLocalReferences(true);
 * \endcode
 *
 * Some apps want to write their HTML code directly into the widget instead of
 * opening an url. You can do this in the following way:
 *
 * \code
 * QString myHTMLCode = ...;
 * KHTMLPart *w = new KHTMLPart();
 * w->begin();
 * w->write(myHTMLCode);
 * ...
 * w->end();
 * \endcode
 *
 * You can do as many calls to write() as you wish.  There are two
 * write() methods, one accepting a QString and one accepting a
 * @p char @p * argument. You should use one or the other
 * (but not both) since the method using
 * the @p char @p * argument does an additional decoding step to convert the
 * written data to Unicode.
 *
 * It is also possible to write content to the HTML part using the
 * standard streaming API from KParts::ReadOnlyPart. The usage of
 * the API is similar to that of the begin(), write(), end() process
 * described above as the following example shows:
 *
 * \code
 * KHTMLPart *doc = new KHTMLPart();
 * doc->openStream( "text/html", KURL() );
 * doc->writeStream( QCString( "<html><body><p>KHTML Rocks!</p></body></html>" ) );
 * doc->closeStream();
 * \endcode
 *
 * @short HTML Browser Widget
 * @author Lars Knoll (knoll@kde.org)
 *
 */
class KHTML_EXPORT KHTMLPart : public KParts::ReadOnlyPart
{
  Q_OBJECT
  friend class KHTMLView;
  friend class DOM::HTMLTitleElementImpl;
  friend class DOM::HTMLFrameElementImpl;
  friend class DOM::HTMLIFrameElementImpl;
  friend class DOM::HTMLObjectElementImpl;
  friend class DOM::HTMLAnchorElementImpl;
  friend class DOM::HTMLMetaElementImpl;
  friend class DOM::NodeImpl;
  friend class KHTMLRun;
  friend class DOM::HTMLFormElementImpl;
  friend class khtml::RenderPartObject;
  friend class KJS::Window;
  friend class KJS::ScheduledAction;
  friend class KJS::JSNodeFilter;
  friend class KJS::WindowFunc;
  friend class KJS::ExternalFunc;
  friend class KJS::JSEventListener;
  friend class KJS::JSLazyEventListener;
  friend class KJS::DOMDocument;
  friend class KJS::SourceFile;
  friend class KJSProxy;
  friend class KHTMLPartBrowserExtension;
  friend class DOM::DocumentImpl;
  friend class DOM::HTMLDocumentImpl;
  friend class KHTMLPartBrowserHostExtension;
  friend class khtml::HTMLTokenizer;
  friend class khtml::XMLTokenizer;
  friend class khtml::RenderWidget;
  friend class khtml::CSSStyleSelector;
  friend class KHTMLPartIface;
  friend class KHTMLPartFunction;
  friend class KHTMLPopupGUIClient;

  Q_PROPERTY( bool javaScriptEnabled READ jScriptEnabled WRITE setJScriptEnabled )
  Q_PROPERTY( bool javaEnabled READ javaEnabled WRITE setJavaEnabled )
  Q_PROPERTY( bool autoloadImages READ autoloadImages WRITE setAutoloadImages )
  Q_PROPERTY( bool dndEnabled READ dndEnabled WRITE setDNDEnabled )
  Q_PROPERTY( bool pluginsEnabled READ pluginsEnabled WRITE setPluginsEnabled )
  Q_PROPERTY( bool onlyLocalReferences READ onlyLocalReferences WRITE setOnlyLocalReferences )
  Q_PROPERTY( Q3CString dcopObjectId READ dcopObjectId )
  Q_PROPERTY( bool modified READ isModified )

public:
  enum GUIProfile { DefaultGUI, BrowserViewGUI /* ... */ };

  /**
   * Constructs a new KHTMLPart.
   *
   * KHTML basically consists of two objects: The KHTMLPart itself,
   * holding the document data (DOM document), and the KHTMLView,
   * derived from QScrollView, in which the document content is
   * rendered in. You can specify two different parent objects for a
   * KHTMLPart, one parent for the KHTMLPart document and on parent
   * for the KHTMLView. If the second @p parent argument is 0L, then
   * @p parentWidget is used as parent for both objects, the part and
   * the view.
   */
  KHTMLPart( QWidget *parentWidget = 0, const char *widgetname = 0,
             QObject *parent = 0, const char *name = 0, GUIProfile prof = DefaultGUI );

  KHTMLPart( KHTMLView *view, QObject *parent = 0, const char *name = 0, GUIProfile prof = DefaultGUI );

  /**
   * Destructor.
   */
  virtual ~KHTMLPart();

  /**
   * Opens the specified URL @p url.
   *
   * Reimplemented from KParts::ReadOnlyPart::openURL .
   */
  virtual bool openURL( const KURL &url );

  /**
   * Stops loading the document and kills all data requests (for images, etc.)
   */
  virtual bool closeURL();

  /**
   * Called when a certain error situation (i.e. connection timed out) occurred.
   * The default implementation either shows a KIO error dialog or loads a more
   * verbose error description a as page, depending on the users configuration.
   * @p job is the job that signaled the error situation
   */
  virtual void showError( KIO::Job* job );

  /**
   * Returns a reference to the DOM HTML document (for non-HTML documents, returns null)
   */
  DOM::HTMLDocument htmlDocument() const;

  /**
   * Returns a reference to the DOM document.
   */
  DOM::Document document() const;

  /**
   * Returns the content of the source document.
   * @since 3.4
   */
   QString documentSource() const;

  /**
   * Returns the node that has the keyboard focus.
   */
  DOM::Node activeNode() const;

  /**
   * Returns a pointer to the KParts::BrowserExtension.
   */
  KParts::BrowserExtension *browserExtension() const;
  KParts::LiveConnectExtension *liveConnectExtension( const khtml::RenderPart *) const;
  KParts::BrowserHostExtension *browserHostExtension() const;

  /**
   * Returns a pointer to the HTML document's view.
   */
  KHTMLView *view() const;

  /**
   * Enable/disable Javascript support. Note that this will
   * in either case permanently override the default usersetting.
   * If you want to have the default UserSettings, don't call this
   * method.
   */
  void setJScriptEnabled( bool enable );

  /**
   * Returns @p true if Javascript support is enabled or @p false
   * otherwise.
   */
  bool jScriptEnabled() const;

  /**
   * Returns the JavaScript interpreter the part is using. This method is
   * mainly intended for applications which embed and extend the part and
   * provides a mechanism for adding additional native objects to the
   * interpreter (or removing the built-ins).
   *
   * One thing people using this method to add things to the interpreter must
   * consider, is that when you start writing new content to the part, the
   * interpreter is cleared. This includes both use of the
   * begin( const KURL &, int, int ) method, and the openURL( const KURL & )
   * method. If you want your objects to have a longer lifespan, then you must
   * retain a KJS::Object yourself to ensure that the reference count of your
   * custom objects never reaches 0. You will also need to re-add your
   * bindings everytime this happens - one way to detect the need for this is
   * to connect to the docCreated() signal, another is to reimplement the
   * begin() method.
   */
  KJS::Interpreter *jScriptInterpreter();

  /**
   * Enable/disable statusbar messages.
   * When this class wants to set the statusbar text, it emits
   * setStatusBarText(const QString & text)
   * If you want to catch this for your own statusbar, note that it returns
   * back a rich text string, starting with "<qt>".  This you need to
   * either pass this into your own QLabel or to strip out the tags
   * before passing it to QStatusBar::message(const QString & message)
   *
   * @see KParts::Part::setStatusBarText( const QString & text )
   */
  void setStatusMessagesEnabled( bool enable );

  /**
   * Returns @p true if status messages are enabled.
   */
  bool statusMessagesEnabled() const;

  /**
   * Enable/disable automatic forwarding by &lt;meta http-equiv="refresh" ....&gt;
   */
  void setMetaRefreshEnabled( bool enable );

  /**
   * Returns @p true if automatic forwarding is enabled.
   */
  bool metaRefreshEnabled() const;

  /**
   * Same as executeScript( const QString & ) except with the Node parameter
   * specifying the 'this' value.
   */
  QVariant executeScript( const DOM::Node &n, const QString &script );

  /**
   * Enables or disables Drag'n'Drop support. A drag operation is started if
   * the users drags a link.
   */
  void setDNDEnabled( bool b );

  /**
   * Returns whether Dragn'n'Drop support is enabled or not.
   */
  bool dndEnabled() const;

  /**
   * Enables/disables Java applet support. Note that calling this function
   * will permanently override the User settings about Java applet support.
   * Not calling this function is the only way to let the default settings
   * apply.
   */
  void setJavaEnabled( bool enable );

  /**
   * Return true if Java applet support is enabled, false if disabled
   */
  bool javaEnabled() const;

  /**
   * Returns the java context of the applets. If no applet exists, 0 is returned.
   */
  KJavaAppletContext *javaContext();

  /**
   * Returns the java context of the applets. If no context exists yet, a
   * new one is created.
   */
  KJavaAppletContext *createJavaContext();

  /**
   * Enables or disables plugins, default is enabled
   */
  void setPluginsEnabled( bool enable );

  /**
   * Returns true if plugins are enabled/disabled.
   */
  bool pluginsEnabled() const;

  /**
   * Specifies whether images contained in the document should be loaded
   * automatically or not.
   *
   * @note Request will be ignored if called before begin().
   */
  void setAutoloadImages( bool enable );
  /**
   * Returns whether images contained in the document are loaded automatically
   * or not.
   * @note that the returned information is unrelieable as long as no begin()
   * was called.
   */
  bool autoloadImages() const;

  /**
   * Security option.
   *
   * Specify whether only file:/ or data:/ urls are allowed to be loaded without
   * user confirmation by KHTML.
   * ( for example referenced by stylesheets, images, scripts, subdocuments, embedded elements ).
   *
   * This option is mainly intended for enabling the "mail reader mode", where you load untrusted
   * content with a file:/ url.
   *
   * Please note that enabling this option currently automatically disables Javascript,
   * Java and Plugins support. This might change in the future if the security model
   * is becoming more sophisticated, so don't rely on this behaviour.
   *
   * ( default false - everything is loaded unless forbidden by KApplication::authorizeURLAction).
   */
  void setOnlyLocalReferences( bool enable );

  /**
   * Returns whether only file:/ or data:/ references are allowed
   * to be loaded ( default false ).  See setOnlyLocalReferences.
   **/
  bool onlyLocalReferences() const;

  /** Returns whether caret mode is on/off.
   * @since 3.2
   */
  bool isCaretMode() const;

  /**
   * Returns @p true if the document is editable, @p false otherwise.
   * @since 3.2
   */
  bool isEditable() const;

  /**
   * Sets the caret to the given position.
   *
   * If the given location is invalid, it will snap to the nearest valid
   * location. Immediately afterwards a @p caretPositionChanged signal
   * containing the effective position is emitted
   * @param node node to set to
   * @param offset zero-based offset within the node
   * @param extendSelection If @p true, a selection will be spanned from the
   *	last caret position to the given one. Otherwise, any existing selection
   *	will be deselected.
   * @since 3.2
   */
  void setCaretPosition(DOM::Node node, long offset, bool extendSelection = false);

  /**
   * Enumeration for displaying the caret.
   * @param Visible caret is displayed
   * @param Invisible caret is not displayed
   * @param Blink caret toggles between visible and invisible
   * @since 3.2
   */
  enum CaretDisplayPolicy {
	CaretVisible, CaretInvisible, CaretBlink
  };

  /**
   * Returns the current caret policy when the view is not focused.
   * @since 3.2
   */
  CaretDisplayPolicy caretDisplayPolicyNonFocused() const;

  /**
   * Sets the caret display policy when the view is not focused.
   *
   * Whenever the caret is in use, this property determines how the
   * caret should be displayed when the document view is not focused.
   *
   * The default policy is CaretInvisible.
   * @param policy new display policy
   * @since 3.2
   */
  void setCaretDisplayPolicyNonFocused(CaretDisplayPolicy policy);

#ifndef KDE_NO_COMPAT
  void enableJScript( bool e ) { setJScriptEnabled(e); }
  void enableJava( bool e ) { setJavaEnabled(e); }
  void enablePlugins( bool e ) { setPluginsEnabled(e); }
  void autoloadImages( bool e ) { setAutoloadImages(e); }
  void enableMetaRefresh( bool e ) { setMetaRefreshEnabled(e); }
  bool setCharset( const QString &, bool ) { return true; }

  KURL baseURL() const;
  QString baseTarget() const;
#endif

  /**
   * Returns the URL for the background Image (used by save background)
   */
  KURL backgroundURL() const;

  /**
   * Schedules a redirection after @p delay seconds.
   */
  void scheduleRedirection( int delay, const QString &url, bool lockHistory = true );

  /**
   * Clears the widget and prepares it for new content.
   *
   * If you want url() to return
   * for example "file:/tmp/test.html", you can use the following code:
   * \code
   * view->begin( KURL("file:/tmp/test.html" ) );
   * \endcode
   *
   * @param url is the url of the document to be displayed.  Even if you
   * are generating the HTML on the fly, it may be useful to specify
   * a directory so that any pixmaps are found.
   *
   * @param xOffset is the initial horizontal scrollbar value. Usually
   * you don't want to use this.
   *
   * @param yOffset is the initial vertical scrollbar value. Usually
   * you don't want to use this.
   *
   * All child frames and the old document are removed if you call
   * this method.
   */
  virtual void begin( const KURL &url = KURL(), int xOffset = 0, int yOffset = 0 );

  /**
   * Writes another part of the HTML code to the widget.
   *
   * You may call
   * this function many times in sequence. But remember: The fewer calls
   * you make, the faster the widget will be.
   *
   * The HTML code is send through a decoder which decodes the stream to
   * Unicode.
   *
   * The @p len parameter is needed for streams encoded in utf-16,
   * since these can have \\0 chars in them. In case the encoding
   * you're using isn't utf-16, you can safely leave out the length
   * parameter.
   *
   * Attention: Don't mix calls to write( const char *) with calls
   * to write( const QString & ).
   *
   * The result might not be what you want.
   */
  virtual void write( const char *str, int len = -1 );

  /**
   * Writes another part of the HTML code to the widget.
   *
   * You may call
   * this function many times in sequence. But remember: The fewer calls
   * you make, the faster the widget will be.
   */
  virtual void write( const QString &str );

  /**
   * Call this after your last call to write().
   */
  virtual void end();

  /*
   * Prints the current HTML page laid out for the printer.
   *
   * (not implemented at the moment)
   */
  //    void print(QPainter *, int pageHeight, int pageWidth);

  /**
   * Paints the HTML page to a QPainter. See KHTMLView::paint for details
   */
  void paint( QPainter *, const QRect &, int = 0, bool * = 0 );

  /**
   * Sets the encoding the page uses.
   *
   * This can be different from the charset. The widget will try to reload the current page in the new
   * encoding, if url() is not empty.
   */
  bool setEncoding( const QString &name, bool override = false );

  /**
   * Returns the encoding the page currently uses.
   *
   * Note that the encoding might be different from the charset.
   */
  QString encoding() const;

  /**
   * Sets a user defined style sheet to be used on top of the HTML 4
   * default style sheet.
   *
   * This gives a wide range of possibilities to
   * change the layout of the page.
   *
   * To have an effect this function has to be called after calling begin().
   */
  void setUserStyleSheet( const KURL &url );

  /**
   * Sets a user defined style sheet to be used on top of the HTML 4
   * default style sheet.
   *
   * This gives a wide range of possibilities to
   * change the layout of the page.
   *
   * To have an effect this function has to be called after calling begin().
   */
  void setUserStyleSheet( const QString &styleSheet );

public:

  /**
   * Sets the standard font style.
   *
   * @param name The font name to use for standard text.
   */
  void setStandardFont( const QString &name );

  /**
   * Sets the fixed font style.
   *
   * @param name The font name to use for fixed text, e.g.
   * the <tt>&lt;pre&gt;</tt> tag.
   */
  void setFixedFont( const QString &name );

  /**
   * Finds the anchor named @p name.
   *
   * If the anchor is found, the widget
   * scrolls to the closest position. Returns @p if the anchor has
   * been found.
   */
  bool gotoAnchor( const QString &name );

  /**
   * Go to the next anchor
   *
   * This is useful to navigate from outside the navigator
   * @since 3.2
   */
  bool nextAnchor();

  /**
   * Go to previous anchor
   * @since 3.2
   */
  bool prevAnchor();

  /**
   * Sets the cursor to use when the cursor is on a link.
   */
  void setURLCursor( const QCursor &c );

  /**
   * Returns the cursor which is used when the cursor is on a link.
   */
  QCursor urlCursor() const;

  /**
   * Extra Find options that can be used when calling the extended findText().
   * @since 3.3
   */
  enum FindOptions
  {
  	FindLinksOnly   = 1 * KFind::MinimumUserOption,
  	FindNoPopups    = 2 * KFind::MinimumUserOption
  	//FindIncremental = 4 * KFind::MinimumUserOption
  };

  /**
   * Starts a new search by popping up a dialog asking the user what he wants to
   * search for.
   * @since 3.3
   */
  void findText();

  /**
   * Starts a new search, but bypasses the user dialog.
   * @param str The string to search for.
   * @param options Find options.
   * @param parent Parent used for centering popups like "string not found".
   * @param findDialog Optionally, you can supply your own dialog.
   * @since 3.3
   */
  void findText( const QString &str, long options, QWidget *parent = 0,
                 KFindDialog *findDialog = 0 );

  /**
   * Initiates a text search.
   */
  void findTextBegin();

  /**
   * Finds the next occurrence of the string or expression.
   * If isRegExp is true then str is converted to a QRegExp, and caseSensitive is ignored.
   * @deprecated, use findText( str, options, parent, findDialog )
   */
  bool findTextNext( const QString &str, bool forward, bool caseSensitive, bool isRegExp );

  /**
   * Finds the next occurence of a string set by @ref findText()
   * @param reverse if true, revert seach direction (only if no find dialog is used)
   * @return true if a new match was found.
   * @since 3.5
   */
  bool findTextNext( bool reverse = false );

  /**
   * Sets the Zoom factor. The value is given in percent, larger values mean a
   * generally larger font and larger page contents. It is not guaranteed that
   * all parts of the page are scaled with the same factor though.
   *
   * The given value should be in the range of 20..300, values outside that
   * range are not guaranteed to work. A value of 100 will disable all zooming
   * and show the page with the sizes determined via the given lengths in the
   * stylesheets.
   */
  void setZoomFactor(int percent);

  /**
   * Returns the current zoom factor.
   */
  int zoomFactor() const;

  /**
   * Returns the text the user has marked.
   */
  virtual QString selectedText() const;

  /**
   * Return the text the user has marked.  This is guaranteed to be valid xml,
   * and to contain the \<html> and \<body> tags.
   *
   * FIXME probably should make virtual for 4.0 ?
   *
   * @since 3.4
   */
  QString selectedTextAsHTML() const;

  /**
   * Returns the selected part of the HTML.
   */
  DOM::Range selection() const;

  /**
   * Returns the selected part of the HTML by returning the starting and end
   * position.
   *
   * If there is no selection, both nodes and offsets are equal.
   * @param startNode returns node selection starts in
   * @param startOffset returns offset within starting node
   * @param endNode returns node selection ends in
   * @param endOffset returns offset within end node.
   * @since 3.2
   */
  void selection(DOM::Node &startNode, long &startOffset,
  		DOM::Node &endNode, long &endOffset) const;

  /**
   * Sets the current selection.
   */
  void setSelection( const DOM::Range & );

  /**
   * Has the user selected anything?
   *
   *  Call selectedText() to
   * retrieve the selected text.
   *
   * @return @p true if there is text selected.
   */
  bool hasSelection() const;

  /**
   * Marks all text in the document as selected.
   */
  void selectAll();

  /**
   * Convenience method to show the document's view.
   *
   * Equivalent to widget()->show() or view()->show() .
   */
  void show();

  /**
   * Convenience method to hide the document's view.
   *
   * Equivalent to widget()->hide() or view()->hide().
   */
  void hide();

  /**
   * Returns a reference to the partmanager instance which
   * manages html frame objects.
   */
  KParts::PartManager *partManager();

  /**
   * Saves the KHTMLPart's complete state (including child frame
   * objects) to the provided QDataStream.
   *
   * This is called from the saveState() method of the
   * browserExtension().
   */
  virtual void saveState( QDataStream &stream );
  /**
   * Restores the KHTMLPart's previously saved state (including
   * child frame objects) from the provided QDataStream.
   *
   * @see saveState()
   *
   * This is called from the restoreState() method of the
   * browserExtension() .
   **/
  virtual void restoreState( QDataStream &stream );

  /**
   * Returns the @p Node currently under the mouse.
   *
   * The returned node may be a shared node (e. g. an \<area> node if the
   * mouse is hovering over an image map).
   */
  DOM::Node nodeUnderMouse() const;

  /**
   * Returns the @p Node currently under the mouse that is not shared.
   *
   * The returned node is always the node that is physically under the mouse
   * pointer (irrespective of logically overlying elements like, e. g.,
   * \<area> on image maps).
   * @since 3.3
   */
  DOM::Node nonSharedNodeUnderMouse() const;

  /**
   * @internal
   */
  const KHTMLSettings *settings() const;

  /**
   * Returns a pointer to the parent KHTMLPart if the part is a frame
   * in an HTML frameset.
   *
   *  Returns 0L otherwise.
   */
  KHTMLPart *parentPart();

  /**
   * Returns a list of names of all frame (including iframe) objects of
   * the current document. Note that this method is not working recursively
   * for sub-frames.
   */
  QStringList frameNames() const;

  Q3PtrList<KParts::ReadOnlyPart> frames() const;

  /**
   * Finds a frame by name. Returns 0L if frame can't be found.
   */
  KHTMLPart *findFrame( const QString &f );

  /**
   * Recursively finds the part containing the frame with name @p f
   * and checks if it is accessible by @p callingPart
   * Returns 0L if no suitable frame can't be found.
   * Returns parent part if a suitable frame was found and
   * frame info in @p *childFrame
   * @since 3.3
   */
  KHTMLPart *findFrameParent( KParts::ReadOnlyPart *callingPart, const QString &f, khtml::ChildFrame **childFrame=0 );

  /**
   * Return the current frame (the one that has focus)
   * Not necessarily a direct child of ours, framesets can be nested.
   * Returns "this" if this part isn't a frameset.
   */
  KParts::ReadOnlyPart *currentFrame() const;

  /**
   * Returns whether a frame with the specified name is exists or not.
   * In contrary to the findFrame method this one also returns true
   * if the frame is defined but no displaying component has been
   * found/loaded, yet.
   */
  bool frameExists( const QString &frameName );

  /**
   * Returns child frame framePart its script interpreter
   */
  KJSProxy *framejScript(KParts::ReadOnlyPart *framePart);

  /**
   * Finds a frame by name. Returns 0L if frame can't be found.
   */
  KParts::ReadOnlyPart *findFramePart( const QString &f );
  /**
   * Called by KJS.
   * Sets the StatusBarText assigned
   * via window.status
   */
  void setJSStatusBarText( const QString &text );

  /**
   * Called by KJS.
   * Sets the DefaultStatusBarText assigned
   * via window.defaultStatus
   */
  void setJSDefaultStatusBarText( const QString &text );

  /**
   * Called by KJS.
   * Returns the StatusBarText assigned
   * via window.status
   */
  QString jsStatusBarText() const;

  /**
   * Called by KJS.
   * Returns the DefaultStatusBarText assigned
   * via window.defaultStatus
   */
  QString jsDefaultStatusBarText() const;

  /**
   * Referrer used for links in this page.
   */
  QString referrer() const;

  /**
   * Referrer used to obtain this page.
   */
  QString pageReferrer() const;

  /**
   * Last-modified date (in raw string format), if received in the [HTTP] headers.
   */
  QString lastModified() const;

  /**
   * Loads a style sheet into the stylesheet cache.
   */
  void preloadStyleSheet( const QString &url, const QString &stylesheet );

  /**
   * Loads a script into the script cache.
   */
  void preloadScript( const QString &url, const QString &script );

  /**
   * @internal
   */
  bool restored() const;

  // ### KDE4 remove me
  enum FormNotification { NoNotification = 0, Before, Only, Unused=255 };
  /**
   * Determine if signal should be emitted before, instead or never when a
   * submitForm() happens.
   * @since 3.2
   * ### KDE4 remove me
   */
  void setFormNotification(FormNotification fn);

  /**
   * Determine if signal should be emitted before, instead or never when a
   * submitForm() happens.
   * ### KDE4 remove me
   * @since 3.2
   */
  FormNotification formNotification() const;

  /**
   * Returns the toplevel (origin) URL of this document, even if this
   * part is a frame or an iframe.
   *
   * @return the actual original url.
   * @since 3.2
   */
  KURL toplevelURL();

  /**
   * Checks whether the page contains unsubmitted form changes.
   *
   * @return true if form changes exist
   * @since 3.3
   */
  bool isModified() const;

  /**
   * Shows or hides the suppressed popup indicator
   * @deprecated
   * @since 3.4
   */
  void setSuppressedPopupIndicator( bool enable );

  /**
   * Shows or hides the suppressed popup indicator
   * @since 3.5
   */
  void setSuppressedPopupIndicator( bool enable, KHTMLPart *originPart );

  /**
   * @internal
   * @since 3.5
   */
  bool inProgress() const;

signals:
  /**
   * Emitted if the cursor is moved over an URL.
   */
  void onURL( const QString &url );

  /**
   * Emitted when the user clicks the right mouse button on the document.
   */
  void popupMenu( const QString &url, const QPoint &point );

  /**
   * This signal is emitted when the selection changes.
   */
  void selectionChanged();

  /**
   * This signal is emitted when an element retrieves the
   * keyboard focus. Note that the signal argument can be
   * a null node if no element is active, meaning a node
   * has explicitly been deactivated without a new one
   * becoming active.
   */
  void nodeActivated( const DOM::Node & );

  /**
   * @internal */
  void docCreated();

  /**
   * This signal is emitted whenever the caret position has been changed.
   *
   * The signal transmits the position the DOM::Range way, the node and
   * the zero-based offset within this node.
   * @param node node which the caret is in. This can be null if the caret
   *	has been deactivated.
   * @param offset offset within the node. If the node is null, the offset
   *	is meaningless.
   * @since 3.2
   */
  void caretPositionChanged(const DOM::Node &node, long offset);


  /**
   * If form notification is on, this will be emitted either for a form
   * submit or before the form submit according to the setting.
   * @since 3.2
   * ### KDE4 remove me
   */
  void formSubmitNotification(const char *action, const QString& url,
                  const QByteArray& formData, const QString& target,
                  const QString& contentType, const QString& boundary);


protected:

  /**
   * returns a KURL object for the given url. Use when
   * you know what you're doing.
   */
  KURL completeURL( const QString &url );

  /**
   * presents a detailed error message to the user.
   * @p errorCode kio error code, eg KIO::ERR_SERVER_TIMEOUT.
   * @p text kio additional information text.
   * @p url the url that triggered the error.
   */
  void htmlError( int errorCode, const QString& text, const KURL& reqUrl );

  virtual void customEvent( QEvent *event );

  /**
   * Eventhandler of the khtml::MousePressEvent.
   */
  virtual void khtmlMousePressEvent( khtml::MousePressEvent *event );
  /**
   * Eventhandler for the khtml::MouseDoubleClickEvent.
   */
  virtual void khtmlMouseDoubleClickEvent( khtml::MouseDoubleClickEvent * );
  /**
   * Eventhandler for the khtml::MouseMouseMoveEvent.
   */
  virtual void khtmlMouseMoveEvent( khtml::MouseMoveEvent *event );
  /**
   * Eventhandler for the khtml::MouseMouseReleaseEvent.
   */
  virtual void khtmlMouseReleaseEvent( khtml::MouseReleaseEvent *event );
  /**
   * Eventhandler for the khtml::DrawContentsEvent.
   */
  virtual void khtmlDrawContentsEvent( khtml::DrawContentsEvent * );

  /**
   * Internal reimplementation of KParts::Part::guiActivateEvent .
   */
  virtual void guiActivateEvent( KParts::GUIActivateEvent *event );

  /**
   * Internal empty reimplementation of KParts::ReadOnlyPart::openFile .
   */
  virtual bool openFile();

  virtual void urlSelected( const QString &url, int button, int state,
                            const QString &_target, KParts::URLArgs args = KParts::URLArgs());

  /**
   * This method is called when a new embedded object (include html frames) is to be created.
   * Reimplement it if you want to add support for certain embeddable objects without registering
   * them in the KDE wide registry system (KSyCoCa) . Another reason for re-implementing this
   * method could be if you want to derive from KTHMLPart and also want all html frame objects
   * to be a object of your derived type, in which case you should return a new instance for
   * the mimetype 'text/html' .
   */
  virtual KParts::ReadOnlyPart *createPart( QWidget *parentWidget, const char *widgetName,
                                            QObject *parent, const char *name,
                                            const QString &mimetype, QString &serviceName,
                                            QStringList &serviceTypes, const QStringList &params);

  // This is for RenderPartObject. We want to ask the 'download plugin?'
  // question only once per mimetype
  bool pluginPageQuestionAsked( const QString& mimetype ) const;
  void setPluginPageQuestionAsked( const QString& mimetype );

  enum PageSecurity { NotCrypted, Encrypted, Mixed };
  void setPageSecurity( PageSecurity sec );

  /**
   * Implements the streaming API of KParts::ReadOnlyPart.
   */
  virtual bool doOpenStream( const QString& mimeType );

  /**
   * Implements the streaming API of KParts::ReadOnlyPart.
   */
  virtual bool doWriteStream( const QByteArray& data );

  /**
   * Implements the streaming API of KParts::ReadOnlyPart.
   */
  virtual bool doCloseStream();

public slots:

  /**
   * Sets the focused node of the document to the specified node. If the node is a form control, the control will
   * receive focus in the same way that it would if the user had clicked on it or tabbed to it with the keyboard. For
   * most other types of elements, there is no visual indication of whether or not they are focused.
   *
   * See activeNode
   *
   * @param node The node to focus
   */
  void setActiveNode( const DOM::Node &node );

  /**
   * Stops all animated images on the current and child pages
   */
  void stopAnimations();

  Q3CString dcopObjectId() const;

  /**
   * Execute the specified snippet of JavaScript code.
   *
   * Returns @p true if JavaScript was enabled, no error occurred
   * and the code returned true itself or @p false otherwise.
   * @deprecated, use executeString( DOM::Node(), script)
   */
  QVariant executeScript( const QString &script );

  /**
   * Enables/disables caret mode.
   *
   * Enabling caret mode displays a caret which can be used to navigate
   * the document using the keyboard only. Caret mode is switched off by
   * default.
   *
   * @param enable @p true to enable, @p false to disable caret mode.
   * @since 3.2
   */
  void setCaretMode(bool enable);

  /**
   * Makes the document editable.
   *
   * Setting this property to @p true makes the document, and its
   * subdocuments (such as frames, iframes, objects) editable as a whole.
   * FIXME: insert more information about navigation, features etc. as seen fit
   *
   * @param enable @p true to set document editable, @p false to set it
   *	read-only.
   * @since 3.2 (pending, do not use)
   */
  void setEditable(bool enable);

  /**
   * Sets the visibility of the caret.
   *
   * This methods displays or hides the caret regardless of the current
   * caret display policy (see setCaretDisplayNonFocused), and regardless
   * of focus.
   *
   * The caret will be shown/hidden only under at least one of
   * the following conditions:
   * @li the document is editable
   * @li the document is in caret mode
   * @li the document's currently focused element is editable
   *
   * @param show @p true to make visible, @p false to hide.
   * @since 3.2
   */
  void setCaretVisible(bool show);

  // ### KDE4 FIXME:
  //          Remove this and make the one below protected+virtual slot.
  //          Warning: this is effectively "internal".  Be careful.
  // @since 3.2
  void submitFormProxy( const char *action, const QString &url,
                        const QByteArray &formData,
                        const QString &target,
                        const QString& contentType = QString::null,
                        const QString& boundary = QString::null );

private slots:

  /**
   * @internal
   */
  void reparseConfiguration();

  /**
   * @internal
   */
  void slotData( KIO::Job*, const QByteArray &data );
  /**
  * @internal
  */
  void slotInfoMessage( KIO::Job*, const QString& msg );
  /**
   * @internal
   */
  void slotRestoreData( const QByteArray &data );
  /**
   * @internal
   */
  void slotFinished( KIO::Job* );
  /**
   * @internal
   */
  void slotFinishedParsing();
  /**
   * @internal
   */
  void slotRedirect();
  /**
   * @internal
   */
  void slotRedirection( KIO::Job*, const KURL& );
  /**
   * @internal
   */
  void slotDebugScript();
  /**
   * @internal
   */
  void slotDebugDOMTree();
  /**
   * @internal
   */
  void slotDebugRenderTree();
  /**
   * @internal
   */
  void slotStopAnimations();
  /**
   * @internal
   */
  virtual void slotViewDocumentSource();
  /**
   * @internal
   */
  virtual void slotViewFrameSource();
  /**
   * @internal
   */
  void slotViewPageInfo();
  /**
   * @internal
   */
  virtual void slotSaveBackground();
  /**
   * @internal
   */
  virtual void slotSaveDocument();
  /**
   * @internal
   */
  virtual void slotSaveFrame();
  /**
   * @internal
   */
  virtual void slotSecurity();
  /**
   * @internal
   */
  virtual void slotSetEncoding();

  /**
   * @internal
   */
  virtual void slotUseStylesheet();

  virtual void slotFind();
  virtual void slotFindDone(); // ### remove me
  virtual void slotFindDialogDestroyed();
  void slotFindNext();
  void slotFindPrev();
  void slotFindAheadText();
  void slotFindAheadLink();

  void slotIncZoom();
  void slotDecZoom();
  void slotIncZoomFast();
  void slotDecZoomFast();

  void slotLoadImages();
  void slotWalletClosed();
  void launchWalletManager();
  void walletMenu();

  /**
   * @internal
   */
  void submitFormAgain();

  /**
   * @internal
   */
  void updateActions();
  /**
   * @internal
   */
  void slotPartRemoved( KParts::Part *part );
  /**
   * @internal
   */
  void slotActiveFrameChanged( KParts::Part *part );
  /**
   * @internal
   */
  void slotChildStarted( KIO::Job *job );
  /**
   * @internal
   */
  void slotChildCompleted();
  /**
   * @internal
   */
  void slotChildCompleted( bool );
  /**
   * @internal
   */
  void slotParentCompleted();
  /**
   * @internal
   */
  void slotChildURLRequest( const KURL &url, const KParts::URLArgs &args );
  /**
   * @internal
   */
  void slotChildDocCreated();
  /**
   * @internal
   */
  void slotRequestFocus( KParts::ReadOnlyPart * );
  void slotLoaderRequestStarted( khtml::DocLoader*, khtml::CachedObject* obj);
  void slotLoaderRequestDone( khtml::DocLoader*, khtml::CachedObject *obj );
  void checkCompleted();

  /**
   * @internal
   */
  void slotAutoScroll();

  void slotPrintFrame();

  void slotSelectAll();

  /**
   * @internal
   */
  void slotProgressUpdate();

  /*
   * @internal
   */
  void slotJobPercent( KIO::Job*, unsigned long );

  /*
   * @internal
   */
  void slotJobDone( KIO::Job* );

  /*
   * @internal
   */
  void slotUserSheetStatDone( KIO::Job* );

  /*
   * @internal
   */
  void slotJobSpeed( KIO::Job*, unsigned long );

  /**
   * @internal
   */
  void slotClearSelection();

  /**
   * @internal
   */
  void slotZoomView( int );

  /**
   * @internal
   */
  void slotHighlight( const QString &, int index, int length );

  /**
   * @internal
   */
  void slotAutomaticDetectionLanguage( int _id );

  /**
   * @internal
   */
  void slotToggleCaretMode();

  /**
   * @internal
   */
  void suppressedPopupMenu();

  /**
   * @internal
   */
  void togglePopupPassivePopup();

  /**
   * @internal
   */
  void showSuppressedPopups();

  /**
   * @internal
   */
  void launchJSConfigDialog();

  /**
   * @internal
   */
  void launchJSErrorDialog();

  /**
   * @internal
   */
  void removeJSErrorExtension();

  /**
   * @internal
   */
  void disableJSErrorExtension();

  /**
   * @internal
   */
  void jsErrorDialogContextMenu();

  /**
   * @internal
   * used to restore or reset the view's scroll position (including positioning on anchors)
   * once a sufficient portion of the document as been laid out.
   */
  void restoreScrollPosition();

  void walletOpened(KWallet::Wallet*);

private:

  KJSErrorDlg *jsErrorExtension();

  enum StatusBarPriority { BarDefaultText, BarHoverText, BarOverrideText };
  void setStatusBarText( const QString& text, StatusBarPriority p);

  bool restoreURL( const KURL &url );
  void resetFromScript();
  void emitSelectionChanged();
  // Returns whether callingHtmlPart may access this part
  bool checkFrameAccess(KHTMLPart *callingHtmlPart);
  bool openURLInFrame( const KURL &url, const KParts::URLArgs &urlArgs );
  bool urlSelectedIntern( const QString &url, int button, int state,
                          const QString &_target, KParts::URLArgs args = KParts::URLArgs());
  void startAutoScroll();
  void stopAutoScroll();
  void overURL( const QString &url, const QString &target, bool shiftPressed = false );
  void resetHoverText(); // Undo overURL and reset HoverText

  bool processObjectRequest( khtml::ChildFrame *child, const KURL &url, const QString &mimetype );

  KWallet::Wallet* wallet();

  void openWallet(DOM::HTMLFormElementImpl*);
  void saveToWallet(const QString& key, const QMap<QString,QString>& data);
  void dequeueWallet(DOM::HTMLFormElementImpl*);
  
  void enableFindAheadActions(bool);

  /**
   * @internal
   */
  // ### KDE4 FIXME:
  //          It is desirable to be able to filter form submissions as well.
  //          For instance, forms can have a target and an inheriting class
  //          might want to filter based on the target.  Make this protected
  //          and virtual, or provide a better solution.
  //          See the web_module for the sidebar for an example where this is
  //          necessary.
  void submitForm( const char *action, const QString &url, const QByteArray &formData,
                   const QString &target, const QString& contentType = QString::null,
                   const QString& boundary = QString::null );

  void popupMenu( const QString &url );

  void init( KHTMLView *view, GUIProfile prof );


  void clear();

  bool scheduleScript( const DOM::Node &n, const QString& script);

  QVariant crossFrameExecuteScript(const QString& target, const QString& script);
  QVariant executeScheduledScript();

  bool requestFrame( khtml::RenderPart *frame, const QString &url, const QString &frameName,
                     const QStringList &args = QStringList(), bool isIFrame = false );

  /**
   * @internal returns a name for a frame without a name.
   * This function returns a sequence of names.
   * All names in a sequence are different but the sequence is
   * always the same.
   * The sequence is reset in clear().
   */
  QString requestFrameName();

  bool requestObject( khtml::RenderPart *frame, const QString &url, const QString &serviceType,
                      const QStringList &args = QStringList() );

  bool requestObject( khtml::ChildFrame *child, const KURL &url, const KParts::URLArgs &args = KParts::URLArgs() );

  DOM::EventListener *createHTMLEventListener( QString code, QString name, DOM::NodeImpl *node );

  DOM::HTMLDocumentImpl *docImpl() const;
  DOM::DocumentImpl *xmlDocImpl() const;
  khtml::ChildFrame *frame( const QObject *obj );

  khtml::ChildFrame *recursiveFrameRequest( KHTMLPart *callingHtmlPart, const KURL &url, const KParts::URLArgs &args, bool callParent = true );

  bool checkLinkSecurity( const KURL &linkURL,const QString &message = QString::null, const QString &button = QString::null );
  QVariant executeScript( const QString& filename, int baseLine, const DOM::Node &n, const QString& script );

  KJSProxy *jScript();

  KHTMLPart *opener();
  long cacheId() const;
  void setOpener( KHTMLPart *_opener );
  bool openedByJS();
  void setOpenedByJS( bool _openedByJS );

  void checkEmitLoadEvent();
  void emitLoadEvent();

  bool initFindNode( bool selection, bool reverse, bool fromCursor );

  void extendSelection( DOM::NodeImpl* node, int offset, DOM::Node& selectionNode, long& selectionOffset, bool right, bool paragraph );
  /** extends the current selection to the given content-coordinates @p x, @p y
   * @param x content x-coordinate
   * @param y content y-coordinate
   * @param absX absolute x-coordinate of @p innerNode
   * @param absY absolute y-coordinate of @p innerNode
   * @param innerNode node from which to start extending the selection. The
   *	caller has to ensure that the node has a renderer.
   * @internal
   */
  void extendSelectionTo(int x, int y, int absX, int absY, const DOM::Node &innerNode);
  /** checks whether a selection is extended.
   * @return @p true if a selection is extended by the mouse.
   */
  bool isExtendingSelection() const;
  khtml::Decoder *createDecoder();
  QString defaultEncoding() const;

  /** .html, .xhtml or .xml */
  QString defaultExtension() const;

  /** @internal
   * generic zoom in
   */
  void zoomIn(const int stepping[], int count);
  /** @internal
   * generic zoom out
   */
  void zoomOut(const int stepping[], int count);

  void emitCaretPositionChanged(const DOM::Node &node, long offset);

  void setDebugScript( bool enable );

  void runAdFilter();

  KHTMLPartPrivate *d;
  friend class KHTMLPartPrivate;
};


#endif
