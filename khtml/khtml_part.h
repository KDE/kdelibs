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

#include <kparts/part.h>
#include <kparts/browserextension.h>

class KHTMLPartPrivate;
class KHTMLPartBrowserExtension;
class KJSProxy;
class KHTMLWidget;

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

class KHTMLPart : public KParts::ReadOnlyPart
{
  Q_OBJECT
  friend class KHTMLWidget;
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
  KHTMLWidget *htmlWidget() const;

  void enableJScript( bool enable );
  bool jScriptEnabled() const;
  KJSProxy *jScript();
  void executeScript( const QString &script );

  void enableJava( bool enable );
  bool javaEnabled() const;

  virtual void begin( const KURL &url, int xOffset = 0, int yOffset = 0 );
  virtual void write( const char *str, int len );
  virtual void write( const QString &str );
  virtual void end();

  const khtml::Settings *settings() const;

  void setBaseURL( const KURL &url );
  KURL baseURL() const;
  void setBaseTarget( const QString &target );
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
   * Html-Font-Sizes range from 1 (smallest) to 7 (biggest).
   *
   * They are related to the CSS font sizes by 1 == xx-small to 7 == xx-large.
   */
  void setFontSizes(const int *newFontSizes, const int *newFixedFontSizes=0);

  /**
   * Gets point sizes to be associated with the HTML-sizes used in
   * <FONT size=Html-Font-Size>
   *
   * Html-Font-Sizes range from 1 (smallest) to 7 (biggest).
   *
   * They are related to the CSS font sizes by 1 == xx-small to 7 == xx-large.
   */
  void fontSizes(const int *&fontSizes, const int *&fixedFontSizes) const;

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

  KParts::PartManager *partManager();

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
  friend class KHTMLWidget;
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
