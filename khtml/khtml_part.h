/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *                     2000 Simon Hausmann <hausmann@kde.org>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __khtml_part_h__
#define __khtml_part_h__

#include "dom/html_document.h"
#include "dom/dom2_range.h"

#include <kparts/part.h>
#include <kparts/browserextension.h>

#include <qregexp.h>

class KHTMLPartPrivate;
class KHTMLPartBrowserExtension;
class KJSProxy;
class KHTMLView;
class KHTMLSettings;
class KJavaAppletContext;

namespace DOM
{
  class HTMLDocument;
  class HTMLDocumentImpl;
  class HTMLTitleElementImpl;
  class HTMLElementImpl;
  class HTMLFrameElementImpl;
  class HTMLIFrameElementImpl;
  class HTMLObjectElementImpl;
  class HTMLFormElementImpl;
  class Node;
};

namespace khtml
{
  class RenderPart;
  class RenderPartObject;
  struct ChildFrame;
  class MouseEvent;
  class MousePressEvent;
  class MouseDoubleClickEvent;
  class MouseMoveEvent;
  class MouseReleaseEvent;
  class DrawContentsEvent;
  class CachedObject;
};

namespace KJS {
    class Window;
    class WindowFunc;
};

namespace KParts
{
  class PartManager;
};

/**
 * This class is khtml's main class. It features an almost complete
 * web browser, and html renderer.
 *
 * The easiest way to use this class (if you just want to display a an HTML
 * page at some URL) is the following:
 *
 * <pre>
 * KURL url = "http://www.kde.org";
 * KHTMLPart *w = new KHTMLPart();
 * w->openURL(url);
 * w->view()->resize(500, 400);
 * w->show();
 * </pre>
 *
 * By default Java and JavaScript support are disabled. You can enable them by
 * using the @ref enableJava() and @ref enableJScript() methods.
 *
 * Some apps want to write their HTML code directly into the widget instead of
 * it opening an url. You can also do that in the following way:
 *
 * <pre>
 * QString myHTMLCode = ...;
 * KHTMLPart *w = new KHTMLPart();
 * w->begin();
 * w->write(myHTMLCode);
 * ...
 * w->end();
 * </pre>
 *
 * You can do as many calls to write as you want. But there are two
 * @ref write() methods, one accepting a @ref QString one accepting a
 * @p char @p * argument. You should use one or the other
 * (but not both) since the method using
 * the @p char @p * argument does an additional decoding step to convert the
 * written data to Unicode.
 *
 * @short HTML Browser Widget
 * @author Lars Knoll (knoll@kde.org)
 * @version $Id$
 * */
class KHTMLPart : public KParts::ReadOnlyPart
{
  Q_OBJECT
  friend class KHTMLView;
  friend class DOM::HTMLTitleElementImpl;
  friend class DOM::HTMLFrameElementImpl;
  friend class DOM::HTMLIFrameElementImpl;
  friend class DOM::HTMLObjectElementImpl;
  friend class DOM::HTMLElementImpl;
  friend class KHTMLRun;
  friend class DOM::HTMLFormElementImpl;
  friend class khtml::RenderPartObject;
  friend class KJS::WindowFunc;
  friend class KHTMLPartBrowserExtension;
  friend class KHTMLFontSizeAction;
  Q_PROPERTY( bool javaScriptEnabled READ jScriptEnabled WRITE enableJScript )
  Q_PROPERTY( bool javaEnabled READ javaEnabled WRITE enableJava )
  Q_PROPERTY( bool autoloadImages READ autoloadImages WRITE autoloadImages )
  Q_PROPERTY( bool dndEnabled READ dndEnabled WRITE setDNDEnabled )
public:
  enum GUIProfile { DefaultGUI, BrowserViewGUI /* ... */ };

  /**
   * Construct a new @ref KHTMLPart.
   *
   * KHTML basically consists of two objects: The @ref KHTMLPart itself,
   * holding the document data (DOM document), and the @ref KHTMLView,
   * derived from @ref QScrollview, in which the document content is
   * rendered in. You can specify two different parent objects for a
   * @ref KHTMLPart, one parent for the @ref KHTMLPart document and on parent
   * for the @ref KHTMLView. If the second @p parent argument is 0L, then
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
   * Reimplemented from @ref KParts::ReadOnlyPart::openURL .
   */
  virtual bool openURL( const KURL &url );

  /**
   * Stop loading the document and kill all data requests (for images, etc.)
   */
  virtual bool closeURL();

  /**
   * Retrieve a reference to the DOM document.
   */
  DOM::HTMLDocument htmlDocument() const;

  /**
   * Retrieve a pointer to the @ref KParts::BrowserExtension
   */
  KParts::BrowserExtension *browserExtension() const;

  /**
   * Retrieve a pointer to the HTML document's view.
   */
  KHTMLView *view() const;

  /**
   * Enable/disable Javascript support. Note that this will
   * in either case permanently override the default usersetting.
   * If you want to have the default UserSettings, don't call this
   * method.
   */
  void enableJScript( bool enable );

  /**
   * Returns @p true if Javascript support is enabled or @p false
   * otherwise.
   */
  bool jScriptEnabled() const;

  /**
   * Execute the specified snippet of JavaScript code.
   *
   * Returns @p true if JavaScript was enabled, no error occured
   * and the code returned true itself or @p false otherwise.
   * @deprecated, use the one below.
   */
  QVariant executeScript( const QString &script );
  /**
   * Same as above except the Node parameter specifying the 'this' value.
   */
  QVariant executeScript( const DOM::Node &n, const QString &script );

  void setDNDEnabled( bool b );
  bool dndEnabled() const;

  /**
   * Enable/disable Java applet support. Note that calling this function
   * will permanently override the User settings about Java applet support.
   * Not calling this function is the only way to let the default settings
   * apply.
   */
  void enableJava( bool enable );

  /**
   * Return if Java applet support is enabled/disabled.
   */
  bool javaEnabled() const;

  /**
   * Return the java context of the applets. If no applet exists, 0 is returned.
   */
  KJavaAppletContext *javaContext();

  /**
   * Return the java context of the applets. If no context exists yet, a new one is
   * created.
   */
  KJavaAppletContext *createJavaContext();

  /**
   * Should images be loaded automatically? Default is @p true.
   *
   * (not implemented at the moment)
   */
  void autoloadImages( bool enable );
  bool autoloadImages() const;

  /**
   * Clear the widget and prepares it for new content.
   *
   * If you want @ref url() to return
   * for example "file:/tmp/test.html", you can use the following code:
   * <PRE>
   * view->begin( KURL("file:/tmp/test.html" ) );
   * </PRE>
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
   * Write another part of the HTML code to the widget.
   *
   * You may call
   * this function many times in sequence. But remember: The fewer calls
   * you make, the faster the widget will be.
   *
   * The HTML code is send through a decoder which decodes the stream to
   * Unicode.
   *
   * The @p len parameter is needed for streams encoded in utf-16,
   * since these can have \0 chars in them. In case the encoding
   * you're using isn't utf-16, you can safely leave out the length
   * parameter.
   *
   * Attention: Don't mix calls to @ref write( const char *) with calls
   * to @ref write( const QString & ).
   *
   * The result might not be what you want.
   */
  virtual void write( const char *str, int len = -1 );

  /**
   * Write another part of the HTML code to the widget.
   *
   * You may call
   * this function many times in sequence. But remember: The fewer calls
   * you make, the faster the widget will be.
   */
  virtual void write( const QString &str );

  /**
   * Call this after your last call to @ref write().
   */
  virtual void end();

  /*
   * Print current HTML page layed out for the printer.
   *
   * (not implemented at the moment)
   */
  //    void print(QPainter *, int pageHeight, int pageWidth);

  /**
   * Mainly used internally.
   *
   * Sets the document's base URL.
   */
  void setBaseURL( const KURL &url );

  /**
   * Retrieve the base URL of this document
   *
   * The base URL is ususally set by a <base url=...>
   * tag in the document head.
   */
  KURL baseURL() const;

  /**
   * Mainly used internally.
   *
   *Sets the document's base target.
   */
  void setBaseTarget( const QString &target );

  /**
   * Retrieve the base target of this document.
   *
   * The base target is ususally set by a <base target=...>
   * tag in the document head.
   */
  QString baseTarget() const;

  /**
   * @internal
   */
  KURL completeURL( const QString &url, const QString &target = QString::null );

  /**
   * @internal
   */
  void scheduleRedirection( int delay, const QString &url );

  /**
   * Set the charset to use for displaying HTML pages.
   *
   * If override is @p true,
   * it will override charset specifications of the document.
   */
  bool setCharset( const QString &name, bool override = false );

  /**
   * Set the encoding the page uses.
   *
   * This can be different from the charset. The widget will try to reload the current page in the new
   * encoding, if url() is not empty.
   */
  bool setEncoding( const QString &name, bool override = false );

  /**
   * return the encoding the page currently uses.
   *
   * Note that the encoding might be different from the charset.
   */
  QString encoding();

  /**
   * Set a user defined style sheet to be used on top of the HTML 4
   * default style sheet.
   *
   * This gives a wide range of possibilities to
   * change the layout of the page.
   */
  void setUserStyleSheet(const KURL &url);

  /**
   * Set a user defined style sheet to be used on top of the HTML 4
   * default style sheet.
   *
   * This gives a wide range of possibilities to
   * change the layout of the page.
   */
  void setUserStyleSheet(const QString &styleSheet);

  /**
   * Set point sizes to be associated with the HTML-sizes used in
   * <FONT size=Html-Font-Size>
   *
   * Html-Font-Sizes range from 0 (smallest) to 6 (biggest), but you
   * can specify up to 15 font sizes, the bigger ones will get used,
   * if <font size=+1> extends over 7, or if a 'font-size: larger'
   * style declaration gets into this region.
   *
   * They are related to the CSS font sizes by 0 == xx-small to 6 == xx-large.  */
  void setFontSizes(const QValueList<int> &newFontSizes );

  /**
   * Get point sizes to be associated with the HTML-sizes used in
   * <FONT size=Html-Font-Size>
   *
   * Html-Font-Sizes range from 0 (smallest) to 6 (biggest).
   *
   * They are related to the CSS font sizes by 0 == xx-small to 6 == xx-large.
   */
  QValueList<int> fontSizes() const;

  /**
   * Reset the point sizes to be associated with the HTML-sizes used in
   * <FONT size=Html-Font-Size> to their default.
   *
   * Html-Font-Sizes range from 1 (smallest) to 7 (biggest).
   */
  void resetFontSizes();

  /**
   * Set the standard font style.
   *
   * @param name The font name to use for standard text.
   */
  void setStandardFont( const QString &name );

  /**
   * Set the fixed font style.
   *
   * @param name The font name to use for fixed text, e.g.
   * the <tt>&lt;pre&gt;</tt> tag.
   */
  void setFixedFont( const QString &name );

  /**
   * Find the anchor named @p name.
   *
   * If the anchor is found, the widget
   * scrolls to the closest position. Returns @p if the anchor has
   * been found.
   */
  bool gotoAnchor( const QString &name );

  /**
   * Set the cursor to use when the cursor is on a link.
   */
  void setURLCursor( const QCursor &c );

  /**
   * Retrieve the cursor which is used when the cursor is on a link.
   */
  const QCursor& urlCursor() const;

  /**
   * Initiate a text search.
   */
  void findTextBegin();

  /**
   * Find the next occurrance of the expression.
   */
  bool findTextNext( const QRegExp &exp, bool forward );

  /**
   * Find the next occurence of the string.
   */
  bool findTextNext( const QString &str, bool forward, bool caseSensitive );

  /**
   * Get the text the user has marked.
   */
  virtual QString selectedText() const;

  /**
   * Retrieve the selected part of the HTML.
   */
  DOM::Range selection() const;

  /**
   * Has the user selected anything?
   *
   *  Call @ref selectedText() to
   * retrieve the selected text.
   *
   * @return @p true if there is text selected.
   */
  bool hasSelection() const;

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
   * @internal
   */
  KParts::PartManager *partManager();

  /**
   * Save the @ref KHTMLPart's complete state (including child frame
   * objects) to the provided @ref QDataStream.
   *
   * You can use this method to provide history functionality.
   *
   * This is called from the @ref saveState() method of the
   *  @ref browserExtension().
   */
  virtual void saveState( QDataStream &stream );
  /**
   * Restore the @ref KHTMLPart's previously saved state (including
   * child frame objects) from the provided QDataStream.
   *
   * @see saveState()
   *
   * This is called from the @ref restoreState() method of the
   * @ref browserExtension() .
   **/
  virtual void restoreState( QDataStream &stream );

  bool restoreURL( const KURL &url );

  /**
   * Retrieve the @p Node currently under the mouse
   */
  DOM::Node nodeUnderMouse() const;

  /**
   * @internal
   */
  const KHTMLSettings *settings() const;

  /**
   * Retrieve a pointer to the parent @ref KHTMLPart if the part is a frame
   * in an HTML frameset.
   *
   *  Returns 0L otherwise.
   */
  KHTMLPart *parentPart();

  QStringList frameNames() const;

  const QList<KParts::ReadOnlyPart> frames() const;

  /**
   * Find a frame by name. Returns 0L if frame can't be found.
   */
  KHTMLPart *findFrame( const QString &f );

  /**
   * Returns whether a frame with the specified name is exists or not.
   * In contrary to the @findFrame method this one also returns true
   * if the frame is defined but no displaying component has been
   * found/loaded, yet.
   */
  bool frameExists( const QString &frameName );

  bool openURLInFrame( const KURL &url, const KParts::URLArgs &urlArgs );

signals:
  /**
   * Eemitted if the cursor is moved over an URL.
   */
  void onURL( const QString &url );

  /**
   * Emitted when the user clicks the right mouse button on the document.
   */
  void popupMenu(const QString &url, const QPoint &point);

  void selectionChanged();

protected:

  virtual bool event( QEvent *event );

  virtual void khtmlMousePressEvent( khtml::MousePressEvent *event );
  virtual void khtmlMouseDoubleClickEvent( khtml::MouseDoubleClickEvent * );
  virtual void khtmlMouseMoveEvent( khtml::MouseMoveEvent *event );
  virtual void khtmlMouseReleaseEvent( khtml::MouseReleaseEvent *event );
  virtual void khtmlDrawContentsEvent( khtml::DrawContentsEvent * );

  virtual void guiActivateEvent( KParts::GUIActivateEvent *event );

  /**
   * Internal empty reimplementation of @ref KParts::ReadOnlyPart::openFile .
   */
  virtual bool openFile();

  /**
   * @internal
   */
  virtual void overURL( const QString &url, const QString &target );
  /**
   * @internal
   */
  virtual void urlSelected( const QString &url, int button = 0, int state = 0, const QString &_target = QString::null );

  /**
   * @internal
   */
  bool processObjectRequest( khtml::ChildFrame *child, const KURL &url, const QString &mimetype );

  /**
   * @internal
   */
  virtual void submitForm( const char *action, const QString &url, const QByteArray &formData, const QString &target, const QString& contentType = QString::null, const QString& boundary = QString::null );

  /**
   * @internal
   */
  virtual void popupMenu( const QString &url );

  /**
   * @internal
   */
  virtual KParts::ReadOnlyPart *createPart( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name,
                                            const QString &mimetype, QString &serviceName, QStringList &serviceTypes,
                                            const QStringList &params);

  /**
     * @internal
     */
  void updateFontSize( int add );

  /**
    * @internal
    */
  void setFontBaseInternal( int base, bool absolute );

protected slots:
  /**
   * Internal. Called by the @ref BrowserExtension .
   */
  void reparseConfiguration();

private slots:
  /**
   * @internal
   */
  void slotData( KIO::Job*, const QByteArray &data );
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
  void slotRedirection(KIO::Job*, const KURL&);

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

  virtual void slotFind();

  void slotFindDone(); // BCI: make virtual
  void slotFindDialogDestroyed();

  void slotIncFontSizes();
  void slotDecFontSizes();

  void slotLoadImages();

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
  void slotChildURLRequest( const KURL &url, const KParts::URLArgs &args );

  void slotLoaderRequestDone( const DOM::DOMString &baseURL, khtml::CachedObject *obj );
  void checkCompleted();

  /**
   * @internal
   */
  void slotShowDocument( const QString &url, const QString &target );

  /**
   * @internal
   */
  void slotAutoScroll();

  void slotPrintFrame();

  void slotSelectAll();

private:
  void startAutoScroll();
  void stopAutoScroll();

  void emitSelectionChanged();

  void init( KHTMLView *view, GUIProfile prof );

  void clear();

  bool scheduleScript( const DOM::Node &n, const QString& script);

  QVariant executeScheduledScript();

  /**
   * @internal
   */
  bool requestFrame( khtml::RenderPart *frame, const QString &url, const QString &frameName,
                     const QStringList &args = QStringList() );

  /**
   * @internal returns a name for a frame without a name.
   * This function returns a sequence of names.
   * All names in a sequence are different but the sequence is
   * always the same.
   * The sequence is reset in clear().
   */
  QString requestFrameName();

  /**
   * @internal
   */
  bool requestObject( khtml::RenderPart *frame, const QString &url, const QString &serviceType,
                      const QStringList &args = QStringList() );

  /**
   * @internal
   */
  bool requestObject( khtml::ChildFrame *child, const KURL &url, const KParts::URLArgs &args = KParts::URLArgs() );


  DOM::HTMLDocumentImpl *docImpl() const;
  khtml::ChildFrame *frame( const QObject *obj );

  khtml::ChildFrame *recursiveFrameRequest( const KURL &url, const KParts::URLArgs &args, bool callParent = true );

  /**
   * @internal
   */
  bool checkLinkSecurity(KURL linkURL);

  /**
   * @internal
   */
  KJSProxy *jScript();

  KHTMLPartPrivate *d;
  friend class KHTMLPartPrivate;
};


#endif
