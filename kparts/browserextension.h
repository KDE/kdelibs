/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
                      David Faure <faure@kde.org>

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

#ifndef __kbrowser_h__
#define __kbrowser_h__

#include <sys/types.h>

#include <qpoint.h>
#include <qlist.h>
#include <qdatastream.h>

#include <kaction.h>
#include <kparts/part.h>
#include <kparts/event.h>

class KFileItem;
typedef QList<KFileItem> KFileItemList;

class QString;

namespace KParts {

class OpenURLEvent : public Event
{
public:
  OpenURLEvent( ReadOnlyPart *part, const KURL &url )
  : Event( s_strOpenURLEvent ), m_part( part ), m_url( url ) {}

  ReadOnlyPart *part() const { return m_part; }
  KURL url() const { return m_url; }

  static bool test( const QEvent *event ) { return Event::test( event, s_strOpenURLEvent ); }

private:
  static const char *s_strOpenURLEvent;
  ReadOnlyPart *m_part;
  KURL m_url;

  class OpenURLEventPrivate;
  OpenURLEventPrivate *d;
};

struct URLArgsPrivate;

struct URLArgs
{
  URLArgs();
  URLArgs( const URLArgs &args );
  URLArgs( bool reload, int xOffset, int yOffset, const QString &serviceType = QString::null );
  virtual ~URLArgs();

  bool reload;
  int xOffset;
  int yOffset;
  QString serviceType;

  QByteArray postData; //khtml specific stuff
  QString frameName;

  URLArgsPrivate *d;
};

class BrowserExtensionPrivate;

 /**
  * The following standard actions are defined by the host of the view :
  *
  * [selection-dependent actions]
  * cut : copy selected items to clipboard and notifies that a cut has been done, using DCOP
  * copy : copy selected items to clipboard (and notifies it's not a cut)
  * pastecut : called when doing a paste after a cut
  * pastecopy : called when doing a paste after a copy
  * trash : move selected items to trash
  * del : delete selected items (couldn't call it delete!)
  * shred : shred selected items (secure deletion)
  *      <P>
  * [normal actions]
  * print : print :-)
  * saveLocalProperties : save current configuration into .directory
  * savePropertiesAsDefault : save current configuration as default
  * reparseConfiguration : re-read configuration and apply it
  * refreshMimeTypes : if the view uses mimetypes it should re-determine them
  *
  * The view defines a slot with the name of the action in order to implement the action.
  * The browser will detect the slot automatically and connect its action to it when
  * appropriate (i.e. when the view is active)
  *
  *
  * The selection-dependent actions are disabled by default and the view should
  * enable them when the selection changes, emitting @ref enableAction.
  *
  * The normal actions does not depend on the selection. For each slot that is
  * defined in the second list, the action is automatically enabled.
  *
  */
class BrowserExtension : public QObject
{
  Q_OBJECT
public:
  /**
   * Constructor
   *
   * @param parent the KParts::ReadOnlyPart that this extension ... "extends" :)
   * @param name an optional name for the extension
   */
  BrowserExtension( KParts::ReadOnlyPart *parent,
                    const char *name = 0L );


  virtual ~BrowserExtension();

  virtual void setURLArgs( const URLArgs &args );

  virtual URLArgs urlArgs();

  /**
   * @return the current x offset
   * For a scrollview, implement this using contentsX()
   */
  virtual int xOffset();
  /**
   * @return the current y offset
   * For a scrollview, implement this using contentsY()
   */
  virtual int yOffset();

  /**
   * Used by the browser to save the current state of the view
   * (in order to restore it if going back in navigation)
   * If you want to save additionnal properties, reimplement it
   * but don't forget to call the parent method (probably first).
   */
  virtual void saveState( QDataStream &stream );

  /**
   * Used by the browser to restore the view in the state
   * it was when we left it.
   * If you saved additionnal properties, reimplement it
   * but don't forget to call the parent method (probably first).
   */
  virtual void restoreState( QDataStream &stream );

signals:
  /**
   * Enable or disable a standard action held by the browser.
   * See class documentation for the list of standard actions.
   */
  void enableAction( const char * name, bool enabled );

  /**
   * Open @p url in the browser, optionnally forcing @p reload, and
   * optionnally setting the x and y offsets.
   * The @serviceType allows to ...
   */
  void openURLRequest( const KURL &url, const KParts::URLArgs &args = KParts::URLArgs() );

  /**
   * Tell the hosting browser that the part opened a new URL (which can be
   * queried via KParts::Part::url().
   * This helps the browser to update/create an entry in the history.
   * The part may *not* emit this signal together with @ref openURLRequest.
   * Emit openURLRequest if you want the browser to handle an URL the user
   * asked to open (from within your part/document). This signal however is
   * useful if you want to handle URLs all yourself internally, while still
   * telling the hosting browser about new opened URLs, in order to provide
   * a proper history functionality to the user.
   * An example of usage is a html rendering component which wants to emit
   * this signal when a child frame document changed its URL.
   * Conclusion: you probably want to use @ref openURLRequest instead
   */
  void openURLNotify();

  /**
   * Update the URL shown in the browser's location bar to @p url
   */
  void setLocationBarURL( const QString &url );

  /**
   * Ask the hosting browser to open a new window for the given @url.
   * The @p args argument is optional additionnal information for the
   * browser, @see KParts::URLArgs
   */
  void createNewWindow( const KURL &url, const KParts::URLArgs &args = KParts::URLArgs() );

  /**
   * Since the part emits the jobid in the started() signal,
   * progress information is automatically displayed.
   * However, if you don't use a KIO::Job in the part,
   * you can use @ref loadingProgress and @ref speedProgress
   * to display progress information
   */
  void loadingProgress( int percent );
  /**
   * @see loadingProgress
   */
  void speedProgress( int bytesPerSecond );

  /**
   * Emit this to make the browser show a standard popup menu
   * at the point @p global for the files @p items.
   */
  void popupMenu( const QPoint &global, const KFileItemList &items );

  /**
   * Emit this to make the browser show a standard popup menu
   * at the point @p global for the files @p items.
   * The GUI described by @p client is being merged with the popupmenu of the host
   */
  void popupMenu( KXMLGUIClient *client, const QPoint &global, const KFileItemList &items );

  /**
   * Emit this to make the browser show a standard popup menu
   * at the point @p global for the given @p url. Give as much information
   * about this URL as possible, like the @p mimeType and the file type
   * (@p mode: S_IFREG, S_IFDIR...)
   */
  void popupMenu( const QPoint &global, const KURL &url,
                  const QString &mimeType, mode_t mode = -1 );

  /**
   * Emit this to make the browser show a standard popup menu
   * at the point @p global for the given @p url. Give as much information
   * about this URL as possible, like the @p mimeType and the file type
   * (@p mode: S_IFREG, S_IFDIR...)
   * The GUI described by @p client is being merged with the popupmenu of the host
   */
  void popupMenu( KXMLGUIClient *client,
                  const QPoint &global, const KURL &url,
                  const QString &mimeType, mode_t mode = -1 );

  void selectionInfo( const KFileItemList &items );
  void selectionInfo( const QString &text );
  void selectionInfo( const KURL::List &urls );

private slots:
  void slotCompleted();

private:
  KParts::ReadOnlyPart *m_part;
  URLArgs m_args;
  BrowserExtensionPrivate *d;
};

};

#endif

