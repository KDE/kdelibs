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

#ifndef __kparts_browserextension_h__
#define __kparts_browserextension_h__

#include <sys/types.h>

#include <qpoint.h>
#include <qptrlist.h>
#include <qdatastream.h>
#include <qstringlist.h>

#include <kparts/part.h>
#include <kparts/event.h>

class KFileItem;
typedef QPtrList<KFileItem> KFileItemList;
class QString;

namespace KParts {

class BrowserInterface;

struct URLArgsPrivate;

/**
 * URLArgs is a set of arguments bundled into a structure,
 * to allow specifying how a URL should be opened by openURL().
 * In other words, this is like arguments to openURL(), but without
 * have to change the signature of openURL() (since openURL is a
 * generic KParts method).
 * The parts (with a browser extension) who care about urlargs will
 * use those arguments, others will ignore them.
 *
 * This can also be used the other way round, when a part asks
 * for a URL to be opened (with openURLRequest or createNewWindow).
 */
struct URLArgs
{
  URLArgs();
  URLArgs( const URLArgs &args );
  URLArgs &operator=( const URLArgs &args);

  URLArgs( bool reload, int xOffset, int yOffset, const QString &serviceType = QString::null );
  virtual ~URLArgs();

  /**
   * This buffer can be used by the part to save and restore its contents.
   * See KHTMLPart for instance.
   */
  QStringList docState;

  /**
   * @p reload is set when the cache shouldn't be used (forced reload).
   */
  bool reload;
  /**
   * @p xOffset is the horizontal scrolling of the part's widget
   * (in case it's a scrollview). This is saved into the history
   * and restored when going back in the history.
   */
  int xOffset;
  /**
   * @p yOffset vertical scrolling position, @ref xOffset.
   */
  int yOffset;
  /**
   * The servicetype (usually mimetype) to use when opening the next URL.
   */
  QString serviceType;

  /**
   * KHTML-specific field, contents of the HTTP POST data.
   */
  QByteArray postData;
  /**
   * KHTML-specific field, header defining the type of the POST data.
   */
  void setContentType( const QString & contentType );
  /**
   * KHTML-specific field, header defining the type of the POST data.
   */
  QString contentType() const;
  /**
   * KHTML-specific field, whether to do a POST instead of a GET,
   * for the next openURL.
   */
  void setDoPost( bool enable );
  /**
   * KHTML-specific field, whether to do a POST instead of a GET,
   * for the next openURL.
   */
  bool doPost() const;

  /**
   * Whether to lock the history when opening the next URL.
   * This is used during e.g. a redirection, to avoid a new entry
   * in the history.
   */
  void setLockHistory( bool lock );
  bool lockHistory() const;

  /**
   * Meta-data to associate with the next KIO operation
   * @see KIO::TransferJob etc.
   */
  QMap<QString, QString> &metaData();

  /**
   * The frame in which to open the URL. KHTML/Konqueror-specific.
   */
  QString frameName;

  /**
   * If true, the part who asks for a URL to be opened can be 'trusted'
   * to execute applications. For instance, the directory views can be
   * 'trusted' whereas HTML pages are not trusted in that respect.
   */
  bool trustedSource;

  URLArgsPrivate *d;
};

struct WindowArgsPrivate;

/**
 * The WindowArgs are used to specify arguments to the "create new window"
 * call (see the createNewWindow variant that uses WindowArgs).
 * The primary reason for this is the javascript window.open function.
 */
struct WindowArgs
{
    WindowArgs();
    WindowArgs( const WindowArgs &args );
    WindowArgs &operator=( const WindowArgs &args );
    WindowArgs( const QRect &_geometry, bool _fullscreen, bool _menuBarVisible,
                bool _toolBarsVisible, bool _statusBarVisible, bool _resizable );
    WindowArgs( int _x, int _y, int _width, int _height, bool _fullscreen,
                bool _menuBarVisible, bool _toolBarsVisible,
                bool _statusBarVisible, bool _resizable );

    // Position
    int x;
    int y;
    // Size
    int width;
    int height;
    bool fullscreen; //defaults to false
    bool menuBarVisible; //defaults to true
    bool toolBarsVisible; //defaults to true
    bool statusBarVisible; //defaults to true
    bool resizable; //defaults to true

    bool lowerWindow; //defaults to false

    WindowArgsPrivate *d; // yes, I am paranoid :-)
};

class OpenURLEvent : public Event
{
public:
  OpenURLEvent( ReadOnlyPart *part, const KURL &url, const URLArgs &args = URLArgs() );
  virtual ~OpenURLEvent();

  ReadOnlyPart *part() const { return m_part; }
  KURL url() const { return m_url; }
  URLArgs args() const { return m_args; }

  static bool test( const QEvent *event ) { return Event::test( event, s_strOpenURLEvent ); }

private:
  static const char *s_strOpenURLEvent;
  ReadOnlyPart *m_part;
  KURL m_url;
  URLArgs m_args;

  class OpenURLEventPrivate;
  OpenURLEventPrivate *d;
};

class BrowserExtensionPrivate;

 /**
  * The Browser Extension is an extension (yes, no kidding) to
  * KParts::ReadOnlyPart, which allows a better integration of parts
  * with browsers (in particular Konqueror).
  * Remember that ReadOnlyPart only has openURL(KURL), with no other settings.
  * For full-fledged browsing, we need much more than that, including
  * many arguments about how to open this URL (see URLArgs), allowing
  * parts to save and restore their data into the back/forward history,
  * allowing parts to control the location bar URL, to requests URLs
  * to be opened by the hosting browser, etc.
  *
  * The part developer needs to define its own class derived from BrowserExtension,
  * to implement the virtual methods [and the standard-actions slots, see below].
  *
  * The way to associate the BrowserExtension with the part is to simply
  * create the BrowserExtension as a child of the part (in QObject's terms).
  * The hosting application will look for it automatically.
  *
  * Another aspect of the browser integration is that a set of standard
  * actions are provided by the browser, but implemented by the part
  * (for the actions it supports).
  *
  * The following standard actions are defined by the host of the view :
  *
  * [selection-dependent actions]
  * @li @p cut : Copy selected items to clipboard and store 'not cut' in clipboard.
  * @li @p copy : Copy selected items to clipboard and store 'cut' in clipboard.
  * @li @p paste : Paste clipboard into view URL.
  * @li @p pasteTo(const KURL &) : Paste clipboard into given URL.
  * @li @p rename : Rename item in place.
  * @li @p trash : Move selected items to trash.
  * @li @p del : Delete selected items (couldn't call it delete!).
  * @li @p shred : Shred selected items (secure deletion).
  * @li @p properties : Show file/document properties.
  * @li @p editMimeType : show file/document's mimetype properties.
  *
  * [normal actions]
  * @li @p print : Print :-)
  * @li @p reparseConfiguration : Re-read configuration and apply it.
  * @li @p refreshMimeTypes : If the view uses mimetypes it should re-determine them.
  *
  *
  * The view defines a slot with the name of the action in order to implement the action.
  * The browser will detect the slot automatically and connect its action to it when
  * appropriate (i.e. when the view is active).
  *
  *
  * The selection-dependent actions are disabled by default and the view should
  * enable them when the selection changes, emitting @ref enableAction().
  *
  * The normal actions do not depend on the selection.
  * You need to enable 'print' when printing is possible - you can even do that
  * in the constructor.
  *
  * A special case is the configuration slots, not connected to any action directly,
  * and having parameters.
  *
  * [configuration slot]
  * @li @p setSaveViewPropertiesLocally( bool ): If @p true, view properties are saved into .directory
  *                                       otherwise, they are saved globally.
  */
class BrowserExtension : public QObject
{
  Q_OBJECT
  Q_PROPERTY( bool urlDropHandling READ isURLDropHandlingEnabled WRITE setURLDropHandlingEnabled )
public:
  /**
   * Constructor
   *
   * @param parent The KParts::ReadOnlyPart that this extension ... "extends" :)
   * @param name An optional name for the extension.
   */
  BrowserExtension( KParts::ReadOnlyPart *parent,
                    const char *name = 0L );


  virtual ~BrowserExtension();

  virtual void setURLArgs( const URLArgs &args );

  URLArgs urlArgs() const;

  /**
   * Returns the current x offset.
   *
   * For a scrollview, implement this using contentsX().
   */
  virtual int xOffset();
  /**
   * Returns the current y offset.
   *
   * For a scrollview, implement this using contentsY().
   */
  virtual int yOffset();

  /**
   * Used by the browser to save the current state of the view
   * (in order to restore it if going back in navigation).
   *
   * If you want to save additionnal properties, reimplement it
   * but don't forget to call the parent method (probably first).
   */
  virtual void saveState( QDataStream &stream );

  /**
   * Used by the browser to restore the view in the state
   * it was when we left it.
   *
   * If you saved additionnal properties, reimplement it
   * but don't forget to call the parent method (probably first).
   */
  virtual void restoreState( QDataStream &stream );

  /**
   * Returns whether url drop handling is enabled.
   * See @ref setURLDropHandlingEnabled for more information about this
   * property.
   */
  bool isURLDropHandlingEnabled() const;

  /**
   * Enables or disables url drop handling. URL drop handling is a property
   * describing whether the hosting shell component is allowed to install an
   * event filter on the part's widget, to listen for URI drop events.
   * Set it to true if you are exporting a BrowserExtension implementation and
   * do not provide any special URI drop handling. If set to false you can be
   * sure to receive all those URI drop events unfiltered. Also note that the
   * implementation as of Konqueror installs the event filter only on the part's
   * widget itself, not on child widgets.
   */
  void setURLDropHandlingEnabled( bool enable );

  void setBrowserInterface( BrowserInterface *impl );
  BrowserInterface *browserInterface() const;

  /**
   * @return the status (enabled/disabled) of an action.
   * When the enableAction signal is emitted, the browserextension
   * stores the status of the action internally, so that it's possible
   * to query later for the status of the action, using this method.
   */
  bool isActionEnabled( const char * name ) const;

  typedef QMap<QCString,QCString> ActionSlotMap;
  /**
   * Returns a map containing the action names as keys and corresponding
   * SLOT()'ified method names as data entries.
   *
   * This is very useful for
   * the host component, when connecting the own signals with the
   * extension's slots.
   * Basically you iterate over the map, check if the extension implements
   * the slot and connect to the slot using the data value of your map
   * iterator.
   * Checking if the extension implements a certain slot can be done like this:
   *
   * <pre>
   *   extension->metaObject()->slotNames().contains( actionName + "()" )
   * </pre>
   *
   * (note that @p actionName is the iterator's key value if already
   *  iterating over the action slot map, returned by this method)
   *
   * Connecting to the slot can be done like this:
   *
   * <pre>
   *   connect( yourObject, SIGNAL( yourSignal() ),
   *            extension, mapIterator.data() )
   * </pre>
   *
   * (where "mapIterator" is your QMap<QCString,QCString> iterator)
   */
  static ActionSlotMap actionSlotMap();

  /**
   * @return a pointer to the static action-slot map. Preferred method to get it.
   * The map is created if it doesn't exist yet
   */
  static ActionSlotMap * actionSlotMapPtr();

  /**
   * Queries @p obj for a child object which inherits from this
   * BrowserExtension class. Convenience method.
   */
  static BrowserExtension *childObject( QObject *obj );

// KDE invents support for public signals...
#undef signals
#define signals public
signals:
#undef signals
#define signals protected
  /**
   * Enables or disable a standard action held by the browser.
   *
   * See class documentation for the list of standard actions.
   */
  void enableAction( const char * name, bool enabled );

  /**
   * Asks the host (browser) to open @p url.
   * To set a reload, the x and y offsets, the service type etc., fill in the
   * appropriate fields in the @p args structure.
   * Hosts should not connect to this signal but to @ref openURLRequestDelayed.
   */
  void openURLRequest( const KURL &url, const KParts::URLArgs &args = KParts::URLArgs() );

  /**
   * This signal is emitted when openURLRequest is called, after a 0-seconds timer.
   * This allows the caller to terminate what it's doing first, before (usually)
   * being destroyed. Parts should never use this signal, hosts should only connect
   * to this signal.
   */
  void openURLRequestDelayed( const KURL &url, const KParts::URLArgs &args = KParts::URLArgs() );

  /**
   * Tells the hosting browser that the part opened a new URL (which can be
   * queried via @ref KParts::Part::url().
   *
   * This helps the browser to update/create an entry in the history.
   * The part may @em not emit this signal together with @ref openURLRequest().
   * Emit @ref openURLRequest() if you want the browser to handle a URL the user
   * asked to open (from within your part/document). This signal however is
   * useful if you want to handle URLs all yourself internally, while still
   * telling the hosting browser about new opened URLs, in order to provide
   * a proper history functionality to the user.
   * An example of usage is a html rendering component which wants to emit
   * this signal when a child frame document changed its URL.
   * Conclusion: you probably want to use @ref openURLRequest() instead.
   */
  void openURLNotify();

  /**
   * Updates the URL shown in the browser's location bar to @p url.
   */
  void setLocationBarURL( const QString &url );

  /**
   * Sets the URL of an icon for the currently displayed page.
   */
  void setIconURL( const KURL &url );

  /**
   * Asks the hosting browser to open a new window for the given @p url.
   *
   * The @p args argument is optional additionnal information for the
   * browser,
   * @see KParts::URLArgs
   */
  void createNewWindow( const KURL &url, const KParts::URLArgs &args = KParts::URLArgs() );

  /**
   * Asks the hosting browser to open a new window for the given @p url
   * and return a reference to the content part.
   * The request for a reference to the part is only fullfilled/processed
   * if the serviceType is set in the @p args . (otherwise the request cannot be
   * processed synchroniously.
   */
  void createNewWindow( const KURL &url, const KParts::URLArgs &args,
                        const KParts::WindowArgs &windowArgs, KParts::ReadOnlyPart *&part );

  /**
   * Since the part emits the jobid in the @ref started() signal,
   * progress information is automatically displayed.
   *
   * However, if you don't use a @ref KIO::Job in the part,
   * you can use @ref loadingProgress() and @ref speedProgress()
   * to display progress information.
   */
  void loadingProgress( int percent );
  /**
   * @see loadingProgress
   */
  void speedProgress( int bytesPerSecond );

  void infoMessage( const QString & );

  /**
   * Emit this to make the browser show a standard popup menu
   * at the point @p global for the files @p items.
   */
  void popupMenu( const QPoint &global, const KFileItemList &items );

  /**
   * Emit this to make the browser show a standard popup menu
   * at the point @p global for the files @p items.
   *
   * The GUI described by @p client is being merged with the popupmenu of the host
   */
  void popupMenu( KXMLGUIClient *client, const QPoint &global, const KFileItemList &items );

  /**
   * Emit this to make the browser show a standard popup menu
   * at the point @p global for the given @p url.
   *
   * Give as much information
   * about this URL as possible, like the @p mimeType and the file type
   * (@p mode: S_IFREG, S_IFDIR...)
   */
  void popupMenu( const QPoint &global, const KURL &url,
                  const QString &mimeType, mode_t mode = (mode_t)-1 );

  /**
   * Emit this to make the browser show a standard popup menu
   * at the point @p global for the given @p url.
   *
   * Give as much information
   * about this URL as possible, like the @p mimeType and the file type
   * (@p mode: S_IFREG, S_IFDIR...)
   * The GUI described by @p client is being merged with the popupmenu of the host
   */
  void popupMenu( KXMLGUIClient *client,
                  const QPoint &global, const KURL &url,
                  const QString &mimeType, mode_t mode = (mode_t)-1 );

  /**
   * Inform the hosting application about the current selection.
   * Used when a set of files/URLs is selected (with full information
   * about those URLs, including size, permissions etc.)
   */
  void selectionInfo( const KFileItemList &items );
  /**
   * Inform the hosting application about the current selection.
   * Used when some text is selected.
   */
  void selectionInfo( const QString &text );
  /**
   * Inform the hosting application about the current selection.
   * Used when a set of URLs is selected.
   */
  void selectionInfo( const KURL::List &urls );

private slots:
  void slotCompleted();
  void slotOpenURLRequest( const KURL &url, const KParts::URLArgs &args );
  void slotEmitOpenURLRequestDelayed();
  void slotEnableAction( const char *, bool );

private:
  KParts::ReadOnlyPart *m_part;
  URLArgs m_args;
public:
  typedef QMap<QCString,int> ActionNumberMap;

private:
  static ActionNumberMap * s_actionNumberMap;
  static ActionSlotMap * s_actionSlotMap;
  static void createActionSlotMap();
protected:
  virtual void virtual_hook( int id, void* data );
private:
  BrowserExtensionPrivate *d;
};

/**
 * An extension class for container parts, i.e. parts that contain
 * other parts.
 * For instance a KHTMLPart hosts one part per frame.
 */
class BrowserHostExtension : public QObject
{
  Q_OBJECT
public:
  BrowserHostExtension( KParts::ReadOnlyPart *parent,
                        const char *name = 0L );

  virtual ~BrowserHostExtension();

  /**
   * Returns a list of the names of all hosted child objects.
   *
   * Note that this method does not query the child objects recursively.
   */
  virtual QStringList frameNames() const;

  /**
   * Returns a list of pointers to all hosted child objects.
   *
   * Note that this method does not query the child objects recursively.
   */

  virtual const QPtrList<KParts::ReadOnlyPart> frames() const;

  /**
   * Opens the given url in a hosted child frame. The frame name is specified in the
   * frameName variable in the urlArgs argument structure (see @ref KParts::URLArgs ) .
   */
  virtual bool openURLInFrame( const KURL &url, const KParts::URLArgs &urlArgs );

  /**
   * Queries @p obj for a child object which inherits from this
   * BrowserHostExtension class. Convenience method.
   */
  static BrowserHostExtension *childObject( QObject *obj );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class BrowserHostExtensionPrivate;
  BrowserHostExtensionPrivate *d;
};

/**
 * An extension class for LiveConnect, i.e. a call from JavaScript
 * from a HTML page which embeds this part.
 * A part can have an object hierarchie by using objid as a reference 
 * to an object.
 */
class LiveConnectExtension : public QObject
{
  Q_OBJECT
public:
  enum Type { 
      TypeVoid=0, TypeBool, TypeFunction, TypeNumber, TypeObject, TypeString 
  };
  typedef QValueList<QPair<Type, QString> > ArgList;

  LiveConnectExtension( KParts::ReadOnlyPart *parent, const char *name = 0L );

  virtual ~LiveConnectExtension() {}
  /**
   * get a field value from objid, return true on success
   */
  virtual bool get( const unsigned long objid, const QString & field, Type & type, unsigned long & retobjid, QString & value );
  /**
   * put a field value in objid, return true on success
   */
  virtual bool put( const unsigned long objid, const QString & field, const QString & value );
  /**
   * calls a function of objid, return true on success
   */
  virtual bool call( const unsigned long objid, const QString & func, const QStringList & args, Type & type, unsigned long & retobjid, QString & value );
  /**
   * notifies the part that there is no reference anymore to objid
   */
  virtual void unregister( const unsigned long objid );

  static LiveConnectExtension *childObject( QObject *obj );
signals:
  /**
   * notify a event from the part of object objid
   */
  virtual void partEvent( const unsigned long objid, const QString & event, const ArgList & args );
};

};

#endif

