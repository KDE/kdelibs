/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *		       2000 Simon Hausmann <hausmann@kde.org>
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

namespace DOM
{
  class HTMLDocument;
  class HTMLDocumentImpl;
  class HTMLTitleElementImpl;
  class HTMLFrameElementImpl;
  class HTMLFormElementImpl;
};

namespace khtml
{
  class Settings;
  class RenderFrame;
  struct ChildFrame;
};

namespace KParts
{
  class PartManager;
};

/**
 * This class is khtml's main class. It features an almost complete
 * web browser, and html renderer.
 *
 * The easiest way to use this class (if you just want to display a HTML
 * page at some URL) is the following:
 *
 * <pre>
 * QString url = "http://www.kde.org";
 * KHTMLPart *w = new KHTMLPart();
 * w->openURL(url);
 * w->view()->resize(500, 400);
 * w->show();
 * </pre>
 *
 * By default the Widget behaves as a full browser, so clicking on some link
 * on the page you just opened will lead yu to that page. This is inconvenient,
 * if you want to use the widget to display for example formatted emails, but
 * don't want the widget to open the site in this window in case someone
 * clicks on an embedded link. In this case just use
 * @ref setFollowsLinks(false). You will then get a Signal @ref urlClicked()
 * instead of KHTMLPart following the links directly.
 *
 * By default Java and JavaScript support is disabled. You can enable it by
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
 * You can do as many calls to write as you want. But there are two @ref write()
 * methods, one accepting a @ref QString one accepting a char * argument. These
 * should not get mixed, since the method usnig the char * argument does an
 * additional decoding step to convert the written data to Unicode.
 *
 * @short HTML Browser Widget
 * @author Lars Knoll (knoll@kde.org)
 * @version $Id$
 *
 */
class KHTMLPart : public KParts::ReadOnlyPart
{
  Q_OBJECT
  friend class KHTMLView;
  friend class DOM::HTMLTitleElementImpl;
  friend class DOM::HTMLFrameElementImpl;
  friend class KHTMLRun;
  friend class DOM::HTMLFormElementImpl;
public:
  KHTMLPart( QWidget *parentWidget = 0, const char *widgetname = 0, QObject *parent = 0, const char *name = 0 );
  virtual ~KHTMLPart();

  virtual bool openURL( const KURL &url );

  virtual bool closeURL();

  DOM::HTMLDocument htmlDocument() const;
  KHTMLPartBrowserExtension *browserExtension() const;
  KHTMLView *view() const;

  void enableJScript( bool enable );
  bool jScriptEnabled() const;
  KJSProxy *jScript();
  void executeScript( const QString &script );

  void enableJava( bool enable );
  bool javaEnabled() const;

  /**
   * should images be loaded automatically? Default is true.
   * (not implemented at the moment)
   */
    void autoloadImages( bool enable );
    bool autoloadImages() const;



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
  virtual void begin( const KURL &url, int xOffset = 0, int yOffset = 0 );

    /**
     * Writes another part of the HTML code to the widget. You may call
     * this function many times in sequence. But remember: The less calls
     * the faster the widget is.
     *
     * The html code is send through a decoder, which decodes the stream to
     * unicode.
     *
     * The len parameter is needed for streams encoded in utf-16, since these
     * can have \0 chars in them. In case the encoding you're using isn't
     * utf-16, you can safely leave out the length parameter.
     *
     * Attention: Don't mix calls to write( const char *) with calls
     * to write( const QString & ). The result might not be what you want.
     */
    virtual void write( const char *str, int len = -1 );

    /**
     * Writes another part of the HTML code to the widget. You may call
     * this function many times in sequence. But remember: The less calls
     * the faster the widget is.
     */
    virtual void write( const QString &str );

    /**
     * Call this after your last call to @ref #write.
     */
  virtual void end();

    /**
     * Print current HTML page layouted for the printer.
     * (not implemented at the moment)
     */
    //    void print(QPainter *, int pageHeight, int pageWidth);

    const khtml::Settings *settings() const;

    /**
     * Mainly used internally. Sets the document's base URL
     */
  void setBaseURL( const KURL &url );
    /**
     * @return the base URL of this document
     *
     * The base url is ususally set by an <base url=...> tag in the document head.
     */
  KURL baseURL() const;

    /**
     * Mainly used internally. Sets the document's base target.
     */
    void setBaseTarget( const QString &target );
    /**
     * @return the base target of this document
     * The base target is ususally set by an <base target=...>
     * tag in the document head.
     */
  QString baseTarget() const;

  KURL completeURL( const QString &url, const QString &target = QString::null );

  void scheduleRedirection( int delay, const KURL &url );

  /**
   * sets the charset to use for displaying html pages. If override is true,
   * it will override charset specifications of the document.
   */
  bool setCharset( const QString &name, bool override = false );

  /**
   * set the encoding the page uses. This can be different form the charset.
   */
  bool setEncoding( const QString &name, bool override = false );

  /**
   * set a user defined style sheet to be used on top of the HTML 4 default style
   * sheet. This gives a wide range of possibilities to change the layout of the
   * page
   */
  void setUserStyleSheet(const KURL &url);

  /**
   * set a user defined style sheet to be used on top of the HTML 4 default style
   * sheet. This gives a wide range of possibilities to change the layout of the
   * page
   */
  void setUserStyleSheet(const QString &styleSheet);


  /**
   * Sets point sizes to be associated with the HTML-sizes used in
   * <FONT size=Html-Font-Size>
   *
   * Html-Font-Sizes range from 0 (smallest) to 6 (biggest), but you can specify up to 15 font sizes,
   * the bigger ones will get used, if <font size=+1> extends over 7, or if a 'font-size: larger' style
   * declaration gets into this region.
   *
   * They are related to the CSS font sizes by 0 == xx-small to 6 == xx-large.
   */
  void setFontSizes(const int *newFontSizes, int numFontSizes = 7);

  /**
   * Gets point sizes to be associated with the HTML-sizes used in
   * <FONT size=Html-Font-Size>
   *
   * Html-Font-Sizes range from 0 (smallest) to 6 (biggest).
   *
   * They are related to the CSS font sizes by 0 == xx-small to 6 == xx-large.
   */
  const int *fontSizes() const;

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
     * Find the anchor named '_name'. If the anchor is found, the widget
     * scrolls to the closest position. Returns TRUE if the anchor has
     * been found.
     */
  bool gotoAnchor( const QString &name );

  /**
   * Sets the cursor to use when the cursor is on a link.
   * (convenience function, just calls htmlWidget->setURLCursor() )
   */
  void setURLCursor( const QCursor &c );

  /**
   * Returns the cursor which is used when the cursor is on a link.
   * (convenience function, just cals htmlWidget->urlCursor() )
   */
  const QCursor& urlCursor();

  /**
   * Initiate a text search.
   */
  void findTextBegin();

  /**
   * Find the next occurrance of the expression.
   */
  bool findTextNext( const QRegExp &exp );

  /**
   * Get the text the user has marked.
   */
  virtual QString selectedText() const;

    /**
     * @returns the selected part of the HTML
     */
    DOM::Range selection() const;

  /**
   * Has the user selected anything?  Call @ref #selectedText to
   * retrieve the selected text.
   *
   * @return true if there is text selected.
   */
  bool hasSelection() const;

  void show();
  void hide();

  KParts::PartManager *partManager();

  virtual void saveState( QDataStream &stream );
  virtual void restoreState( QDataStream &stream );

signals:
  void onURL( const QString &url );

protected:
  virtual bool openFile();

  DOM::HTMLDocumentImpl *docImpl() const;

  virtual void overURL( const QString &url );
  virtual void urlSelected( const QString &url, int button = 0, const QString &_target = QString::null );

  virtual void childRequest( khtml::RenderFrame *frame, const QString &url, const QString &frameName );

  virtual void childRequest( khtml::ChildFrame *child, const KURL &url, const KParts::URLArgs &args = KParts::URLArgs() );

  virtual void processChildRequest( khtml::ChildFrame *child, const KURL &url, const QString &mimetype );

  virtual void submitForm( const char *action, const QString &url, const QCString &formData, const QString &target );

  virtual void popupMenu( const QString &url );

  virtual KParts::ReadOnlyPart *createFrame( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const QString &mimetype, QStringList &serviceTypes );

    virtual bool keyPressHook(QKeyEvent *) { return false; }
    virtual bool keyReleaseHook(QKeyEvent*) {return false; }
    
    virtual bool mousePressHook( QMouseEvent * ){ return false; }
    virtual bool mouseDoubleClickHook( QMouseEvent * ){ return false; }
    virtual bool mouseMoveHook(QMouseEvent *){ return false; }
    virtual bool mouseReleaseHook(QMouseEvent *){ return false; }


protected slots:
  void slotData( KIO::Job*, const QByteArray &data );
  void slotFinished( KIO::Job* );

  void slotRedirect();

  virtual void slotViewDocumentSource();
  virtual void slotViewFrameSource();
  virtual void slotSaveBackground();
  virtual void slotSaveDocument();
  virtual void slotSaveFrame();

private slots:
  void updateActions();
  void slotChildStarted( KIO::Job *job );
  void slotChildCompleted();
  void slotChildURLRequest( const KURL &url, const KParts::URLArgs &args );

private:
  void clear();
  void checkCompleted();

  khtml::ChildFrame *frame( const QObject *obj );

  KHTMLPart *parentPart();

  khtml::ChildFrame *recursiveFrameRequest( const KURL &url, const KParts::URLArgs &args, bool callParent = true, bool newWin = true );

  KHTMLPartPrivate *d;
};

class KHTMLPartBrowserExtension : public KParts::BrowserExtension
{
  Q_OBJECT
  friend class KHTMLPart;
  friend class KHTMLView;
public:
  KHTMLPartBrowserExtension( KHTMLPart *parent, const char *name = 0L );

  virtual int xOffset();
  virtual int yOffset();

  virtual void saveState( QDataStream &stream );
  virtual void restoreState( QDataStream &stream );
private:
  KHTMLPart *m_part;
};

#endif
