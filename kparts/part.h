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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef _KPART_H
#define _KPART_H

#include <qstring.h>
#include <qdom.h>
#include <qguardedptr.h>
#include <kurl.h>

#include <kxmlguiclient.h>

class KInstance;
class QWidget;
class KAction;
class KActionCollection;
class QEvent;
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
class PartBase : virtual public KXMLGUIClient
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
   *  Internal method. Called by @ref KParts::Part to specify the parent object for plugin objects.
   *
   * @internal
   */
  void setPartObject( QObject *object );
  QObject *partObject() const;

protected:
  /**
   * Set the instance (@ref KInstance) for this part.
   *
   * Call this *first* in the inherited class constructor,
   * because it loads the i18n catalogues.
   */
  virtual void setInstance( KInstance *instance );

  /**
   * Set the instance (@ref KInstance) for this part.
   *
   * Call this *first* in the inherited class constructor,
   * because it loads the i18n catalogues.
   */
  virtual void setInstance( KInstance *instance, bool loadPlugins );

  enum PluginLoadingMode {
    DoNotLoadPlugins = 0,
    LoadPlugins = 1,         // load new plugins automatically
    LoadPluginsIfEnabled = 2 // new plugins are disabled by default
  };
  /**
   * For a KParts::Part: call this before setInstance.
   * For a KParts::MainWindow: call this before createGUI.
   */
  void setPluginLoadingMode( PluginLoadingMode loadingMode );

private:
    PartBasePrivate *d;
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
 * @sect About the widget
 * Note that KParts::Part does not inherit @ref QWidget.
 * This is due to the fact that the "visual representation"
 * will probably not be a mere @ref QWidget, but an elaborate one.
 * That's why when implementing your KParts::Part (or derived)
 * you should call KParts::Part::setWidget() in your constructor.
 *
 * @sect About the GUI elements
 * Those elements trigger actions, defined by the part (@ref action()).
 * The layout of the actions in the GUI is defined by an XML file (@ref setXMLFile()).
 *
 * See also @ref ReadOnlyPart and @ref ReadWritePart, which define the
 * framework for a "viewer" part and for an "editor"-like part.
 * Use Part directly only if your part doesn't fit into those.
 */
class Part : public QObject, public PartBase
{
    Q_OBJECT

public:

    /**
     *  Constructor.
     *
     *  @param parent Parent object of the part.
     *  @param name   QT-internal name of the part.
     */
    Part( QObject *parent = 0, const char* name = 0 );

    /**
     *  Destructor.
     */
    virtual ~Part();

    /**
     * Embed this part into a host widget.
     *
     * You don't need to do this if you created the widget with the
     * correct parent widget - this is just a @ref QWidget::reparent().
     * Note that the @ref Part is still the holder
     * of the @ref QWidget, meaning that if you delete the @ref Part,
     * then the widget gets destroyed as well, and vice-versa.
     * This method is not recommended since creating the widget with the correct
     * parent is simpler anyway.
     */
    virtual void embed( QWidget * parentWidget );

    /**
     * @return The widget defined by this part, set by @ref setWidget().
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
     * @internal
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

signals:
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
    virtual void customEvent( QCustomEvent *event );

    /**
     * Convenience method which is called when the Part received a @ref PartActivateEvent .
     * Reimplement this if you don't want to reimplement @ref event and test for the event yourself
     * or even install an event filter.
     */
    virtual void partActivateEvent( PartActivateEvent *event );

    /**
     * Convenience method which is called when the Part received a
     * @ref PartSelectEvent .
     * Reimplement this if you don't want to reimplement @ref event and
     * test for the event yourself or even install an event filter.
     */
    virtual void partSelectEvent( PartSelectEvent *event );

    /**
     * Convenience method which is called when the Part received a
     * @ref GUIActivateEvent .
     * Reimplement this if you don't want to reimplement @ref event and
     * test for the event yourself or even install an event filter.
     */
    virtual void guiActivateEvent( GUIActivateEvent *event );

    /**
     * Convenience method for @ref KXMLGUIFactory::container.
     * @return a container widget owned by the Part's GUI.
     */
    QWidget *hostContainer( const QString &containerName );

private slots:
    void slotWidgetDestroyed();

private:
    QGuardedPtr<QWidget> m_widget;

    PartManager * m_manager;

    PartPrivate *d;
};

class ReadOnlyPartPrivate;

/**
 * Base class for any "viewer" part.
 *
 * This class takes care of network transparency for you,
 * in the simplest way (synchronously).
 * To use the built-in network transparency, you only need to implement
 * @ref openFile(), not @ref openURL().
 * To prevent network transparency, or to implement it another way
 * (e.g. asynchronously), override openURL().
 *
 * KParts Application can use the signals to show feedback while the URL is being loaded.
 *
 * ReadOnlyPart handles the window caption by setting it to the current URL
 * (set in @ref #openURL(), and each time the part is activated).
 * If you want another caption, set it in @ref openFile() and
 * (if the part might ever be used with a part manager) in @ref guiActivateEvent()
 */
class ReadOnlyPart : public Part
{
  Q_OBJECT
public:
  /**
   * Constructor
   * See also @ref Part for the setXXX methods to call.
   */
  ReadOnlyPart( QObject *parent = 0, const char *name = 0 );

  /**
   * Destructor
   */
  virtual ~ReadOnlyPart();

  /**
   * Call this to turn off the progress info dialog used by
   * the internal KIO job. Use this if you provide another way
   * of displaying progress info (e.g. a statusbar), using the
   * signals emitted by this class, and/or those emitted by
   * the Job given by @ref started.
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

public slots:
  /**
   * Only reimplement openURL if you don't want synchronous network transparency
   * Otherwise, reimplement @ref openFile() only .
   *
   * If you reimplement it, don't forget to set the caption, usually with
   * emit setWindowCaption( url.prettyURL() );
   */
  virtual bool openURL( const KURL &url );

public:
  /**
   *  Returns the currently in part used URL.
   *
   *  @return The current used URL.
   */
  KURL url() const { return m_url; }

  /**
   * Called when closing the current url (e.g. document), for instance
   * when switching to another url (note that @ref #openURL() calls it
   * automatically in this case).
   * If the current URL is not fully loaded yet, aborts loading.
   * Deletes the temporary file used when the url is remote.
   * @return always true, but the return value exists for reimplementations
   */
  virtual bool closeURL();

public:
  /**
   * Initiate sending data to this part.
   * This is an alternative to openURL, which allows the user of the part
   * to load the data itself, and send it progressively to the part.
   *
   * @param url the URL representing this data. Although not directly used,
   * every ReadOnlyPart has a URL (see @ref url()), so this simply sets it.
   * @param mimeType the type of data that is going to be sent to this part.
   * @return true if the part supports progressive loading and accepts data, false otherwise.
   */
  bool openStream( const QString& mimeType, const KURL& url );

  /**
   * Send some data to the part. @ref openStream must have been called previously,
   * and must have returned true.
   * @return true if the data was accepted by the part. If false is returned,
   * the application should stop sending data, and doesn't have to call @ref closeStream.
   */
  bool writeStream( const QByteArray& data );

  /**
   * Terminate the sending of data to the part.
   * With some data types (text, html...) @ref closeStream might never actually be called,
   * in the case of continous streams, for instance plain text or HTML data.
   */
  bool closeStream();

private: // Makes no sense for inherited classes to call those. But make it protected there.

  /**
   * Called by @ref openStream to initiate sending of data.
   * Parts which implement progress loading should check the @p mimeType
   * parameter, and return true if they can accept a data stream of that type.
   */
  virtual bool doOpenStream( const QString& /*mimeType*/ ) { return false; }
  /**
   * Receive some data from the hosting application.
   * In this method the part should attempt to display the data progressively.
   * With some data types (text, html...) @ref closeStream might never actually be called,
   * in the case of continous streams. This can't happen with e.g. images.
   */
  virtual bool doWriteStream( const QByteArray& /*data*/ ) { return false; }
  /**
   * This is called by @ref closeStream(), to indicate that all the data has been sent.
   * Parts should ensure that all of the data is displayed at this point.
   * @return whether the data could be displayed correctly.
   */
  virtual bool doCloseStream() { return false; }

signals:
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
   * Same as the above signal except besides indicating that the data has
   * been completely loaded it also informs the host, by setting the flag,
   * that a pending action has been generated as a result of completing the
   * requested task (loading the data).  An example of this is meta-refresh
   * tags on HTML pages which result in the page either being refreshed or
   * the viewer being redirected to another page.  By emitting this signal
   * after appropriately setting the flag, the part can tell the host of the
   * pending scheduled action inorder to give it a chance to accept or cancel
   * that action.
   *
   * @p pendingAction  if true, a pending action exists (ex: a scheduled refresh)
   */
  void completed( bool pendingAction );

  /**
   * Emit this if loading is canceled by the user or by an error.
   **/
  void canceled( const QString &errMsg );

protected slots:
  void slotJobFinished( KIO::Job * job );

protected:
  /**
   * If the part uses the standard implementation of @ref #openURL(),
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
   * This is the usual behaviour in 99% of the apps
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
  KURL m_url;
  /**
   * Local file - the only one the part implementation should deal with.
   */
  QString m_file;
  /**
   * If @p true, @p m_file is a temporary file that needs to be deleted later.
   */
  bool m_bTemp;

private:
  ReadOnlyPartPrivate *d;
};

/**
 * Base class for an "editor" part.
 *
 * This class handles network transparency for you.
 * Anything that can open a URL, allow modifications, and save
 * (to the same URL or a different one).
 *
 * A read-write part can be set to read-only mode, using @ref setReadWrite().
 *
 * Part writers :
 * Any part inheriting ReadWritePart should check @ref isReadWrite
 * before allowing any action that modifies the part.
 * The part probably wants to reimplement @ref setReadWrite, disable those
 * actions. Don't forget to call the parent @ref setReadWrite.
 */
class ReadWritePart : public ReadOnlyPart
{
  Q_OBJECT
public:
  /**
   * Constructor
   * See parent constructor for instructions.
   */
  ReadWritePart( QObject *parent = 0, const char *name = 0 );
  /**
   * Destructor
   * Applications using a ReadWritePart should make sure, before
   * destroying it, to call @ref #closeURL().
   * In @ref KMainWindow::queryClose(), for instance, they should allow
   * closing only if the return value of @ref #closeURL() was true.
   * This allows to cancel.
   */
  virtual ~ReadWritePart();

  /**
   * @return true if the part is in read-write mode
   */
  bool isReadWrite() const { return m_bReadWrite; }

  /**
   * Changes the behaviour of this part to readonly or readwrite.
   * @param readwrite set to true to enable readwrite mode
   */
  virtual void setReadWrite ( bool readwrite = true );

  /**
   * @return true if the document has been modified.
   */
  bool isModified() const { return m_bModified; }

  /**
   * Called when closing the current url (e.g. document), for instance
   * when switching to another url (note that @ref openURL() calls it
   * automatically in this case).
   * If the current URL is not fully loaded yet, aborts loading.
   * Reimplemented from ReadOnlyPart, to handle modified parts
   * (and suggest saving in this case, with yes/no/cancel).
   * @return false on cancel
   */
  virtual bool closeURL();

  /**
   * Save the file to a new location.
   *
   * Calls @ref save(), no need to reimplement
   */
  virtual bool saveAs( const KURL &url );

  /**
   *  Sets the modified flag of the part.
   */
  virtual void setModified( bool modified );

public slots:
  /**
   * Call @ref setModified() whenever the contents get modified.
   * This is a slot for convenience, so that you can connect it
   * to a signal, like textChanged().
   */
  virtual void setModified();

  /**
   * Save the file in the location from which it was opened.
   * You can connect this to the "save" action.
   * Calls @ref saveFile() and @ref saveToURL(), no need to reimplement.
   */
  virtual bool save();

protected:
  /**
   * Save to a local file.
   * You need to implement it, to save to @p m_file.
   * The framework takes care of re-uploading afterwards.
   */
  virtual bool saveFile() = 0;

  /**
   * Save the file.
   *
   * Uploads the file, if @p m_url is remote.
   * This will emit @ref started(), and either @ref completed() or @ref canceled(),
   * in case you want to provide feedback.
   */
  virtual bool saveToURL();

protected slots:
  /**
   * @internal
   */
  void slotUploadFinished( KIO::Job * job );

private:
  bool m_bModified;
  bool m_bReadWrite;
  bool m_bClosing;
};

};

#endif
