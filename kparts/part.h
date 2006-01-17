/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef _KPART_H
#define _KPART_H

#include <qstring.h>
#include <qdom.h>
#include <qpointer.h>
#include <kurl.h>
#include <qcoreevent.h>
#include <kxmlguiclient.h>

class KInstance;
class QWidget;
class KAction;
class KActionCollection;
class QEvent;
class QPoint;
struct QUnknownInterface;

namespace KIO {
  class Job;
}

namespace KParts
{

class PartManager;
class Plugin;
class PartPrivate;
class PartActivateEvent;
class PartSelectEvent;
class GUIActivateEvent;
class PartBasePrivate;

/**
 *  Base class for all parts.
 *
 *  @short Base class for all parts.
 */
class KPARTS_EXPORT PartBase : virtual public KXMLGUIClient
{
    friend class PartBasePrivate;
public:

  /**
   *  Constructor.
   */
  PartBase();

  /**
   *  Destructor.
   */
  virtual ~PartBase();

  /**
   *  Internal method. Called by KParts::Part to specify the parent object for plugin objects.
   *
   * @internal
   */
  void setPartObject( QObject *object );
  QObject *partObject() const;

protected:
  /**
   * Set the instance ( KInstance) for this part.
   *
   * Call this *first* in the inherited class constructor,
   * because it loads the i18n catalogs.
   */
  virtual void setInstance( KInstance *instance );

  /**
   * Set the instance ( KInstance) for this part.
   *
   * Call this *first* in the inherited class constructor,
   * because it loads the i18n catalogs.
   */
  virtual void setInstance( KInstance *instance, bool loadPlugins );

  /**
   * We have three different policies, whether to load new plugins or not. The
   * value in the KConfig object of the KInstance object always overrides
   * LoadPlugins and LoadPluginsIfEnabled.
   */
  enum PluginLoadingMode {
    /**
     * Don't load any plugins at all.
     */
    DoNotLoadPlugins = 0,
    /**
     * Load new plugins automatically. Can be
     * overridden by the plugin if it sets
     * EnabledByDefault=false in the corresponding
     * .desktop file.
     */
    LoadPlugins = 1,
    /**
     * New plugins are disabled by default. Can be
     * overridden by the plugin if it sets
     * EnabledByDefault=true in the corresponding
     * .desktop file.
     */
    LoadPluginsIfEnabled = 2
  };

  /**
   * Load the Plugins honoring the PluginLoadingMode.
   *
   * If you call this method in an already constructed GUI (like when the user
   * has changed which plugins are enabled) you need to add the new plugins to
   * the KXMLGUIFactory:
   * \code
   * if( factory() )
   * {
   *   QList<KParts::Plugin *> plugins = KParts::Plugin::pluginObjects( this );
   *   for(int i = 0; i != plugins.size(); ++i) {
   *      factory()->addClient( plugins[i] );
   *   }
   * }
   * \endcode
   */
  void loadPlugins( QObject *parent, KXMLGUIClient *parentGUIClient, KInstance *instance );

  /**
   * Set how plugins should be loaded
   * @param loadingMode see PluginLoadingMode
   *
   * For a KParts::Part: call this before setInstance.
   * For a KParts::MainWindow: call this before createGUI.
   */
  void setPluginLoadingMode( PluginLoadingMode loadingMode );

  /**
   * If you change the binary interface offered by your part, you can avoid crashes
   * from old plugins lying around by setting X-KDE-InterfaceVersion=2 in the
   * .desktop files of the plugins, and calling setPluginInterfaceVersion( 2 ), so that
   * the old plugins are not loaded. Increase both numbers every time a
   * binary incompatible change in the application's plugin interface is made.
   *
   * @param version the interface version that plugins must have in order to be loaded.
   *
   * For a KParts::Part: call this before setInstance.
   * For a KParts::MainWindow: call this before createGUI.
   */
  void setPluginInterfaceVersion( int version );

private:
  PartBasePrivate* const d;
  QObject *m_obj;
};

/**
 * Base class for parts.
 *
 * A "part" is a GUI component, featuring:
 * @li A widget embeddedable in any application.
 * @li GUI elements that will be merged in the "host" user interface
 * (menubars, toolbars... ).
 *
 * <b>About the widget:</b>\n
 *
 * Note that KParts::Part does not inherit QWidget.
 * This is due to the fact that the "visual representation"
 * will probably not be a mere QWidget, but an elaborate one.
 * That's why when implementing your KParts::Part (or derived)
 * you should call KParts::Part::setWidget() in your constructor.
 *
 * <b>About the GUI elements:</b>\n
 *
 * Those elements trigger actions, defined by the part ( action()).
 * The layout of the actions in the GUI is defined by an XML file ( setXMLFile()).
 *
 * See also ReadOnlyPart and ReadWritePart, which define the
 * framework for a "viewer" part and for an "editor"-like part.
 * Use Part directly only if your part doesn't fit into those.
 */
class KPARTS_EXPORT Part : public QObject, public PartBase
{
    Q_OBJECT

public:

    /**
     *  Constructor.
     *
     *  @param parent Parent object of the part.
     */
    Part( QObject *parent = 0 );

    /**
     *  Destructor.
     */
    virtual ~Part();

    /**
     * Embed this part into a host widget.
     *
     * You don't need to do this if you created the widget with the
     * correct parent widget - this is just a QWidget::reparent().
     * Note that the Part is still the holder
     * of the QWidget, meaning that if you delete the Part,
     * then the widget gets destroyed as well, and vice-versa.
     * This method is not recommended since creating the widget with the correct
     * parent is simpler anyway.
     */
    virtual void embed( QWidget * parentWidget );

    /**
     * @return The widget defined by this part, set by setWidget().
     */
    virtual QWidget *widget();

    /**
     * @internal
     * Used by the part manager.
     */
    virtual void setManager( PartManager * manager );

    /**
     * Returns the part manager handling this part, if any (0L otherwise).
     */
    PartManager * manager() const;

    /**
     * Returns the part (this, or a child part) at the given global position.
     * This is called by the part manager to ask whether a part should be activated
     * when clicking somewhere. In most cases the default implementation is enough.
     * Reimplement this if your part has child parts in some areas (like in khtml or koffice)
     * @param widget the part widget being clicked - usually the same as widget(), except in koffice.
     * @param globalPos the mouse coordinates in global coordinates
     */
    virtual Part *hitTest( QWidget *widget, const QPoint &globalPos );

    /**
     *  @param selectable Indicates whether the part is selectable or not.
     */
    virtual void setSelectable( bool selectable );

    /**
     *  Returns whether the part is selectable or not.
     */
    bool isSelectable() const;

Q_SIGNALS:
    /**
     * Emitted by the part, to set the caption of the window(s)
     * hosting this part
     */
    void setWindowCaption( const QString & caption );
    /**
     * Emited by the part, to set a text in the statusbar of the window(s)
     * hosting this part
     */
    void setStatusBarText( const QString & text );

protected:

    /**
     * Set the main widget.
     *
     * Call this in the Part-inherited class constructor.
     */
    virtual void setWidget( QWidget * widget );

    /**
     * @internal
     */
    virtual void customEvent( QEvent *event );

    /**
     * Convenience method which is called when the Part received a PartActivateEvent .
     * Reimplement this if you don't want to reimplement event and test for the event yourself
     * or even install an event filter.
     */
    virtual void partActivateEvent( PartActivateEvent *event );

    /**
     * Convenience method which is called when the Part received a
     * PartSelectEvent .
     * Reimplement this if you don't want to reimplement event and
     * test for the event yourself or even install an event filter.
     */
    virtual void partSelectEvent( PartSelectEvent *event );

    /**
     * Convenience method which is called when the Part received a
     * GUIActivateEvent .
     * Reimplement this if you don't want to reimplement event and
     * test for the event yourself or even install an event filter.
     */
    virtual void guiActivateEvent( GUIActivateEvent *event );

    /**
     * Convenience method for KXMLGUIFactory::container.
     * @return a container widget owned by the Part's GUI.
     */
    QWidget *hostContainer( const QString &containerName );

Q_SIGNALS:
    void leaveModality();
private Q_SLOTS:
    void slotWidgetDestroyed();

private:
    QPointer<QWidget> m_widget;

    PartManager * m_manager;

    PartPrivate* const d;
};

class ReadWritePart;
class ReadOnlyPartPrivate;
class BrowserExtension;

/**
 * Base class for any "viewer" part.
 *
 * This class takes care of network transparency for you,
 * in the simplest way (synchronously).
 * To use the built-in network transparency, you only need to implement
 * openFile(), not openURL().
 * To prevent network transparency, or to implement it another way
 * (e.g. asynchronously), override openURL().
 *
 * KParts Application can use the signals to show feedback while the URL is being loaded.
 *
 * ReadOnlyPart handles the window caption by setting it to the current URL
 * (set in openURL(), and each time the part is activated).
 * If you want another caption, set it in openFile() and
 * (if the part might ever be used with a part manager) in guiActivateEvent()
 */
class KPARTS_EXPORT ReadOnlyPart : public Part
{
  Q_OBJECT
  friend class ReadWritePart;
public:
  /**
   * Constructor
   * See also Part for the setXXX methods to call.
   */
  ReadOnlyPart( QObject *parent = 0 );

  /**
   * Destructor
   */
  virtual ~ReadOnlyPart();

  /**
   * Call this to turn off the progress info dialog used by
   * the internal KIO job. Use this if you provide another way
   * of displaying progress info (e.g. a statusbar), using the
   * signals emitted by this class, and/or those emitted by
   * the Job given by started.
   */
  void setProgressInfoEnabled( bool show );

  /**
   * Returns whether the part shows the progress info dialog used by internal
   * KIO job.
   */
  bool isProgressInfoEnabled() const;

#ifndef KDE_NO_COMPAT
  void showProgressInfo( bool show );
#endif

public Q_SLOTS:
  /**
   * Only reimplement openURL if you don't want synchronous network transparency
   * Otherwise, reimplement openFile() only .
   *
   * If you reimplement it, don't forget to set the caption, usually with
   * emit setWindowCaption( url.prettyURL() );
   */
  virtual bool openURL( const KUrl &url );

public:
  /**
   *  Returns the currently in part used URL.
   *
   *  @return The current used URL.
   */
  KUrl url() const { return m_url; }

  /**
   * Called when closing the current url (e.g. document), for instance
   * when switching to another url (note that openURL() calls it
   * automatically in this case).
   * If the current URL is not fully loaded yet, aborts loading.
   * Deletes the temporary file used when the url is remote.
   * @return always true, but the return value exists for reimplementations
   */
  virtual bool closeURL();

  /**
   * This convenience method returns the browserExtension for this part,
   * or 0 if there isn't any.
   * @since 4.0
   */
  BrowserExtension* browserExtension() const;

public:
  /**
   * Initiate sending data to this part.
   * This is an alternative to openURL, which allows the user of the part
   * to load the data itself, and send it progressively to the part.
   *
   * @param mimeType the type of data that is going to be sent to this part.
   * @param url the URL representing this data. Although not directly used,
   * every ReadOnlyPart has a URL (see url()), so this simply sets it.
   * @return true if the part supports progressive loading and accepts data, false otherwise.
   */
  bool openStream( const QString& mimeType, const KUrl& url );

  /**
   * Send some data to the part. openStream must have been called previously,
   * and must have returned true.
   * @return true if the data was accepted by the part. If false is returned,
   * the application should stop sending data, and doesn't have to call closeStream.
   */
  bool writeStream( const QByteArray& data );

  /**
   * Terminate the sending of data to the part.
   * With some data types (text, html...) closeStream might never actually be called,
   * in the case of continuous streams, for instance plain text or HTML data.
   */
  bool closeStream();

private: // Makes no sense for inherited classes to call those. But make it protected there.

  /**
   * Called by openStream to initiate sending of data.
   * Parts which implement progress loading should check the @p mimeType
   * parameter, and return true if they can accept a data stream of that type.
   */
  virtual bool doOpenStream( const QString& /*mimeType*/ ) { return false; }
  /**
   * Receive some data from the hosting application.
   * In this method the part should attempt to display the data progressively.
   * With some data types (text, html...) closeStream might never actually be called,
   * in the case of continuous streams. This can't happen with e.g. images.
   */
  virtual bool doWriteStream( const QByteArray& /*data*/ ) { return false; }
  /**
   * This is called by closeStream(), to indicate that all the data has been sent.
   * Parts should ensure that all of the data is displayed at this point.
   * @return whether the data could be displayed correctly.
   */
  virtual bool doCloseStream() { return false; }

Q_SIGNALS:
  /**
   * The part emits this when starting data.
   * If using a KIO::Job, it sets the job in the signal, so that
   * progress information can be shown. Otherwise, job is 0.
   **/
  void started( KIO::Job * );

  /**
   * Emit this when you have completed loading data.
   * Hosting apps will want to know when the process of loading the data
   * is finished, so that they can access the data when everything is loaded.
   **/
  void completed();

  /**
   * Same as the above signal except it indicates whether there is
   * a pending action to be executed on a delay timer. An example of
   * this is the meta-refresh tags on web pages used to reload/redirect
   * after a certain period of time. This signal is useful if you want
   * to give the user the ability to cancel such pending actions.
   *
   * @p pendingAction true if a pending action exists, false otherwise.
   */
  void completed( bool pendingAction );

  /**
   * Emit this if loading is canceled by the user or by an error.
   * @param errMsg the error message, empty if the user canceled the loading voluntarily.
   */
  void canceled( const QString &errMsg );

protected Q_SLOTS:
  void slotJobFinished( KIO::Job * job );

protected:
  /**
   * If the part uses the standard implementation of openURL(),
   * it must reimplement this, to open @p m_file.
   * Otherwise simply define it to { return false; }
   */
  virtual bool openFile() = 0;

  /**
   * @internal
   */
  void abortLoad();

  /**
   * Reimplemented from Part, so that the window caption is set to
   * the current url (decoded) when the part is activated
   * This is the usual behavior in 99% of the apps
   * Reimplement if you don't like it - test for event->activated() !
   *
   * Technical note : this is done with GUIActivateEvent and not with
   * PartActivateEvent because it's handled by the mainwindow
   * (which gets the even after the PartActivateEvent events have been sent)
   */
  virtual void guiActivateEvent( GUIActivateEvent *event );

  /**
   * Remote (or local) url - the one displayed to the user.
   */
  KUrl m_url;
  /**
   * Local file - the only one the part implementation should deal with.
   */
  QString m_file;
  /**
   * If @p true, @p m_file is a temporary file that needs to be deleted later.
   */
  bool m_bTemp;

private:
  ReadOnlyPartPrivate* const d;
};

/**
 * Base class for an "editor" part.
 *
 * This class handles network transparency for you.
 * Anything that can open a URL, allow modifications, and save
 * (to the same URL or a different one).
 *
 * A read-write part can be set to read-only mode, using setReadWrite().
 *
 * Part writers :
 * Any part inheriting ReadWritePart should check isReadWrite
 * before allowing any action that modifies the part.
 * The part probably wants to reimplement setReadWrite, disable those
 * actions. Don't forget to call the parent setReadWrite.
 */
class KPARTS_EXPORT ReadWritePart : public ReadOnlyPart
{
  Q_OBJECT
public:
  /**
   * Constructor
   * See parent constructor for instructions.
   */
  ReadWritePart( QObject *parent = 0 );
  /**
   * Destructor
   * Applications using a ReadWritePart should make sure, before
   * destroying it, to call closeURL().
   * In KMainWindow::queryClose(), for instance, they should allow
   * closing only if the return value of closeURL() was true.
   * This allows to cancel.
   */
  virtual ~ReadWritePart();

  /**
   * @return true if the part is in read-write mode
   */
  bool isReadWrite() const { return m_bReadWrite; }

  /**
   * Changes the behavior of this part to readonly or readwrite.
   * @param readwrite set to true to enable readwrite mode
   */
  virtual void setReadWrite ( bool readwrite = true );

  /**
   * @return true if the document has been modified.
   */
  bool isModified() const { return m_bModified; }

  /**
   * If the document has been modified, ask the user to save changes.
   * This method is meant to be called from KMainWindow::queryClose().
   * It will also be called from closeURL().
   *
   * @return true if closeURL() can be called without the user losing
   * important data, false if the user chooses to cancel.
   *
   * @since 3.2
   */
  virtual bool queryClose();

  /**
   * Called when closing the current url (e.g. document), for instance
   * when switching to another url (note that openURL() calls it
   * automatically in this case).
   *
   * If the current URL is not fully loaded yet, aborts loading.
   *
   * If isModified(), queryClose() will be called.
   *
   * @return false on cancel
   */
  virtual bool closeURL();

  /**
   * Call this method instead of the above if you need control if
   * the save prompt is shown. For example, if you call queryClose()
   * from KMainWindow::queryClose(), you would not want to prompt
   * again when closing the url.
   *
   * Equivalent to promptToSave ? closeURL() : ReadOnlyPart::closeURL()
   *
   * @since 3.2
   */
  virtual bool closeURL( bool promptToSave );

  /**
   * Save the file to a new location.
   *
   * Calls save(), no need to reimplement
   */
  virtual bool saveAs( const KUrl &url );

  /**
   *  Sets the modified flag of the part.
   */
  virtual void setModified( bool modified );

Q_SIGNALS:
   /**
    * @since 3.2, remove in KDE 4, when queryClose is made virtual
    *
    * set handled to true, if you don't want the default handling
    * set abortClosing to true, if you handled the request,
    * but for any reason don't  want to allow closing the document
    */
   void sigQueryClose(bool *handled, bool* abortClosing);

public Q_SLOTS:
  /**
   * Call setModified() whenever the contents get modified.
   * This is a slot for convenience, so that you can connect it
   * to a signal, like textChanged().
   */
  virtual void setModified();

  /**
   * Save the file in the location from which it was opened.
   * You can connect this to the "save" action.
   * Calls saveFile() and saveToURL(), no need to reimplement.
   */
  virtual bool save();

  /**
   * Waits for any pending upload job to finish and returns whether the
   * last save() action was successful.
   */
  bool waitSaveComplete();

protected:
  /**
   * Save to a local file.
   * You need to implement it, to save to @p m_file.
   * The framework takes care of re-uploading afterwards.
   *
   * @return true on success, false on failure.
   * On failure the function should inform the user about the
   * problem with an appropriate message box. Standard error
   * messages can be constructed using KIO::buildErrorString()
   * in combination with the error codes defined in kio/global.h
   */
  virtual bool saveFile() = 0;

  /**
   * Save the file.
   *
   * Uploads the file, if @p m_url is remote.
   * This will emit started(), and either completed() or canceled(),
   * in case you want to provide feedback.
   * @return true on success, false on failure.
   */
  virtual bool saveToURL();

protected Q_SLOTS:
  /**
   * @internal
   */
  void slotUploadFinished( KIO::Job * job );

private:
  void prepareSaving();

private:
  bool m_bModified;
  bool m_bReadWrite;
  bool m_bClosing;
};

} // namespace

#endif
