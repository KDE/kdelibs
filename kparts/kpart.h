#ifndef _KPART_H
#define _KPART_H

#include <qstring.h>
#include <qdom.h>
#include <qguardedptr.h>
#include <kurl.h>

#include "kxmlgui.h"

class KInstance;
class QWidget;
class QAction;
class QActionCollection;

namespace KParts
{

class PartManager;
class Plugin;
class PartPrivate;

/**
 * A "part" is a GUI component, featuring 
 * - a widget embeddedable in any application 
 * - GUI elements that will be merged in the "host" user interface 
 * (menubars, toolbars... )
 *
 * About the widget : 
 * Note that KParts::Part does not inherit QWidget. 
 * This is due to the fact that the "visual representation"
 * will probably not be a mere QWidget, but an elaborate one.
 * That's why when implementing your KParts::Part (or derived)
 * you should call @ref KParts::Part::setWidget() in your constructor.
 *
 * About the GUI elements :
 * Those elements trigger actions, defined by the part (@see action).
 * The layout of the actions in the GUI is defined by an XML file (@see setXMLFile).
 *
 * This class is an abstract interface, you need to inherit from it.
 * See also @ref PartReadOnly and @ref PartReadWrite, which define the
 * framework for a "viewer" part and for an "editor"-like part.
 * Use Part directly only if your part doesn't fit into those.
 */
class Part : public QObject, public XMLGUIServant
{
  Q_OBJECT
public:
    Part( const char* name = 0 );
    virtual ~Part();

    QAction* action( const char* name );
    QActionCollection* actionCollection();

    virtual Plugin* plugin( const char* libname );

    virtual QAction *action( const QDomElement &element );

    /**
     * Embed this part into a host widget.
     * You don't need to do this if you created the widget with the
     * correct parent widget - this is just a reparent().
     * Note that the Part is still the holder
     * of the QWidget, meaning that if you delete the Part,
     * then the widget gets destroyed as well, and vice-versa.
     * This method is deprecated since creating the widget with the correct
     * parent is simpler anyway.
     */
    virtual void embed( QWidget * parentWidget );

    /**
     * @return the widget defined by this part, set by @ref setWidget
     */
    virtual QWidget *widget() { return m_widget; }

    /**
     * @return the instance (@see KInstance) for this part
     * This is a virtual method, so obviously you need to implement it.
     * A typical implementation is, in the constructor :
     *
     * m_instance = new KInstance( "mypartname" );
     *
     * and return m_instance in @ref instance
     */
    virtual KInstance *instance() = 0;

    virtual void updatePlugins();

    /**
     * @return a list of filenames containing plugin xml documents
     */
    virtual QStringList plugins();

    /**
     * @return a list of QDomDocuments, containing the parsed xml documents returned by @ref plugins
     */
    virtual QValueList<QDomDocument> pluginDocuments();

    /**
     * @return a (cached) list of @ref XMLGUIServants, serving the plugin documents returned by
               @ref pluginDocuments()
     */
    virtual const QList<XMLGUIServant> *pluginServants();

    /**
     * @return a (cached) @ref XMLGUIServant for the part.
     * Implementation note : it's the part itself.
     */
    virtual XMLGUIServant *servant() { return this; }

    // Only called by PartManager - should be protected and using friend ?
    void setManager( PartManager * manager ) { m_manager = manager; }
    /**
     * @return the part manager handling this part
     */
    PartManager * manager() { return m_manager; }

    /**
     * @return the parsed XML in a QDomDocument, set by @ref setXMLFile or @ref setXML
     */
    virtual QDomDocument document() const;

protected:
    /**
     * Call this in the Part-inherited class constructor
     * to set the main widget
     */
    virtual void setWidget( QWidget * widget );

    /**
     * Call this in the Part-inherited class constructor
     * to set the name of the rc file containing the XML for the part
     *
     * @param file Either an absolute path for the file, or simply the filename,
     *             which will then be assumed to be installed in the "data" resource,
     *             under a directory named like the instance.
     */
    virtual void setXMLFile( QString file );

    //TODO: return bool, to make it possible for the part to check whether its xml is invalid (Simon)
    virtual void setXML( const QString &document );

private slots:
    void slotWidgetDestroyed();

private:
    QGuardedPtr<QWidget> m_widget;
    /**
     * Holds the contents of the config file
     */
    QString m_config;
    QActionCollection m_collection;
    PartManager * m_manager;

    PartPrivate *d;
};

class ReadOnlyPartPrivate;

/**
 * Base class for any "viewer" part.
 * This class takes care of network transparency for you.
 * You only need to implement @ref openFile, not @ref openURL.
 * Use the signals to show feedback while the URL is being loaded.
 */
class ReadOnlyPart : public Part
{
  Q_OBJECT
public:
  ReadOnlyPart( const char *name = 0 );
  virtual ~ReadOnlyPart();

  virtual void init();

  // Don't reimplement this one, reimplement openFile
  virtual bool openURL( const KURL &url );
  virtual const KURL & url() const { return m_url; }

  virtual void closeURL();

signals:
  // hosting apps will want to know when the process of loading the data is finished, so that they can access the data when everything is loaded
  void started();
  void completed();
  void canceled( const QString &errMsg );

protected slots:
  void slotJobFinished( int _id );
  void slotJobError( int, int, const char * );

protected:
  /**
   * Reimplement this, to open m_file
   */
  virtual bool openFile() = 0;

  /**
   * Remote (or local) url - the one displayed to the user
   */
  KURL m_url;
  /**
   * Local file - the only one the part implementation should deal with
   */
  QString m_file;
  /**
   * If true, m_file is a temp file that needs to be deleted later
   */
  bool m_bTemp;

private:
  ReadOnlyPartPrivate *d;
};

/**
 * Base class for an "editor" part.
 * This class handles network transparency for you.
 * Anything that can open a URL, allow modifications, and save
 * (to the same URL or a different one)
 */
class ReadWritePart : public ReadOnlyPart
{
  Q_OBJECT
public:
  ReadWritePart( const char *name = 0 );
  virtual ~ReadWritePart();

  /**
   * @return true if the document has been modified
   */
  virtual bool isModified() { return m_bModified; }

  /**
   * Call setModified() whenever the contents gets modified
   */
  virtual void setModified( bool modified = true ) { m_bModified = modified; }

  /**
   * Save the file at the location it was opened.
   * In the implementation, save to m_file and then "return saveToURL();"
   * (You might want to ask for a confirmation first...)
   * saveToURL() takes care of resetting the modified flag as well, so
   * you should really call it :)
   */
  virtual bool save() = 0;
  /**
   * Save the file to a new location.
   * Calls save(), no need to reimplement
   */
  virtual bool saveAs( const KURL &url );

protected:
  /**
   * Uploads the file, if m_url is remote.
   * This will emit started(), and completed() or canceled(),
   * in case you want to provide feedback
   */
  virtual bool saveToURL();

protected slots:
  void slotUploadFinished( int _id );
  void slotUploadError( int, int, const char * );

private:
  bool m_bModified;
};

};

#endif
