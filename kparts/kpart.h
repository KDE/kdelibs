#ifndef _KPART_H
#define _KPART_H

#include <qstring.h>
#include <qdom.h>
#include <qguardedptr.h>
#include <kurl.h>

#include "kxmlgui.h"

class KPlugin;
class QWidget;
class QAction;
class QActionCollection;

class KPart : public QObject
{
public:
    KPart( const char* name = 0 );
    virtual ~KPart();

    QString config();

    QAction* action( const char* name );
    QActionCollection* actionCollection();

    virtual KPlugin* plugin( const char* libname );

    /**
	 * Embed this part into a host widget.
     * Note that the KPart is still the holder 
	 * of the QWidget, meaning that if you delete the KPart,
     * then the widget gets destroyed as well, and vice-versa (TODO).
     */
	virtual void embed( QWidget * parentWidget );

    virtual QWidget *widget() { return m_widget; }

protected:
	/**
	 * Call this in the KPart-inherited class constructor
	 * to set the main widget
	 */
	virtual void setWidget( QWidget * widget ) { m_widget = widget; }

    virtual QString configFile() const = 0;
    virtual QString readConfigFile( const QString& filename ) const;

private:
	QGuardedPtr<QWidget> m_widget;
    QString m_config;
    QActionCollection m_collection;
};

class KReadOnlyPart : public KPart
{
  Q_OBJECT
public:
  KReadOnlyPart( const char *name = 0 );
  virtual ~KReadOnlyPart();

  virtual void init();

  // Don't reimplement this one, reimplement openFile
  virtual bool openURL( const QString &url );
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
  // Reimplement this, to open m_file
  virtual bool openFile() = 0;
  virtual void closeURL();
  
  // Remote (or local) url
  KURL m_url;
  // Local file
  QString m_file;
  // If true, m_file is a temp file that needs to be deleted later
  bool m_bTemp;
};

class KReadWritePart : public KReadOnlyPart
{
  Q_OBJECT
public:
  KReadWritePart( const char *name = 0 );
  virtual ~KReadWritePart();

  virtual bool isModified() const = 0;
  virtual void setModified( bool modified = true ) = 0;

  virtual bool save( const QString &url ) = 0;
  // to re_upload the file
};

class KPartGUIServant : public QObject, public KXMLGUIServant
{
  Q_OBJECT
 public:
  KPartGUIServant( KPart *part );

  virtual QAction *action( const QDomElement &element );

  virtual QString xml();

 private:
  KPart *m_part;
};

#endif
