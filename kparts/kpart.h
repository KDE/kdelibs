#ifndef _KPART_H
#define _KPART_H

#include <qstring.h>
#include <qdom.h>
#include <qguardedptr.h>
#include <kurl.h>

#include "kxmlgui.h"

class KPartHost;
class KPlugin;
class QWidget;
class QAction;
class QActionCollection;

class KPart : public QObject
{
  Q_OBJECT
public:
    KPart( const char* name = 0 );
    virtual ~KPart();

    QString config();

    QAction* action( const char* name );
    QActionCollection* actionCollection();

    virtual KPlugin* plugin( const char* libname );

    /**
     * Embed this part into a host widget.
     * You don't need to do this if you created the widget with the
     * correct parent widget - this is just a reparent().
     * Note that the KPart is still the holder
     * of the QWidget, meaning that if you delete the KPart,
     * then the widget gets destroyed as well, and vice-versa.
     */
    virtual void embed( QWidget * parentWidget );

    virtual QWidget *widget() { return m_widget; }

    void setHost( KPartHost *host ) { m_host = host; }
    KPartHost *host() { return m_host; }

protected:
    /**
     * Call this in the KPart-inherited class constructor
     * to set the main widget
     */
    virtual void setWidget( QWidget * widget );

    virtual QString configFile() const = 0;
    virtual QString readConfigFile( const QString& filename ) const;

    KPartHost * m_host; // Couldn't keep the QGuardedPtr here (because KPartHost isn't a QObject anymore) (David)

private slots:
    void slotWidgetDestroyed();

private:
    QGuardedPtr<QWidget> m_widget;
    QString m_config;
    QActionCollection m_collection;
};

/**
 * Base class for any "viewer" part.
 * You need to implement openFile().
 */
class KReadOnlyPart : public KPart
{
  Q_OBJECT
public:
  KReadOnlyPart( const char *name = 0 );
  virtual ~KReadOnlyPart();

  virtual void init();

  // Don't reimplement this one, reimplement openFile
  virtual bool openURL( const KURL &url );
  virtual const KURL & url() const { return m_url; }

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
   * @internal
   * Automatically called before opening another URL
   */
  virtual void closeURL();

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
};

/**
 * Base class for an "editor" part.
 * Anything that can open a URL, allow modifications, and save
 * (to the same URL or a different one)
 */
class KReadWritePart : public KReadOnlyPart
{
  Q_OBJECT
public:
  KReadWritePart( const char *name = 0 );
  virtual ~KReadWritePart();

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


/**
 * This interface allows a Part to access what its "host" has to offer it :
 * containers (menubar, toolbars, ...), getting and setting the window caption...
 */
class KPartHost
{
public:
  KPartHost() {}
  virtual ~KPartHost() {}

  virtual QWidget *topLevelContainer( const QString &name ) = 0;

  virtual QString windowCaption() = 0;
  virtual void setWindowCaption( const QString &caption ) = 0;
};

/**
 * @internal
 */
class KPartGUIServant : public QObject, public KXMLGUIServant
{
  Q_OBJECT
 public:
  KPartGUIServant( KPart *part );

  virtual QAction *action( const QDomElement &element );

  virtual QDomDocument document();

 private:
  KPart *m_part;
};

#endif
