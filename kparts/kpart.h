#ifndef _KPART_H
#define _KPART_H

#include <qaction.h>
#include <qwidget.h>
#include <qlist.h>
#include <qstring.h>
#include <kurl.h>

#include "kplugin.h"

class KPart : public QWidget
{
    Q_OBJECT
public:
    KPart( QWidget* parent = 0, const char* name = 0 );
    ~KPart();

    QString config();

    QAction* action( const char* name );
    QActionCollection* actionCollection();

    virtual KPlugin* plugin( const char* libname );

protected:
    virtual QString configFile() const = 0;
    virtual QString readConfigFile( const QString& filename ) const;

private:
    QString m_config;
    QActionCollection m_collection;
};

class KReadOnlyPart : public KPart
{
  Q_OBJECT
public:
  KReadOnlyPart( QWidget *parent = 0, const char *name = 0 );
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
  
  // Remote url
  KURL m_url;
  // Local file
  QString m_file;
};

class KReadWritePart : public KReadOnlyPart
{
  Q_OBJECT
public:
  KReadWritePart( QWidget *parent = 0, const char *name = 0 );
  virtual ~KReadWritePart();

  virtual bool isModified() const = 0;
  virtual void setModified( bool modified = true ) = 0;

  virtual bool save( const QString &url ) = 0;
  // to re_upload the file
};

#endif
