#ifndef _KPART_H
#define _KPART_H

#include <qaction.h>
#include <qwidget.h>
#include <qlist.h>
#include <qstring.h>

#include "plugin.h"

class KPart : public QWidget
{
    Q_OBJECT
public:
    KPart( QWidget* parent = 0, const char* name = 0 );
    ~KPart();

    /**
     * @return a pointer to the parent part. If there is no parent
     *         or if the parent is not a Part, then 0 is returned.
     */
    KPart* parentPart();

    QString config();

    QAction* action( const char* name );
    QActionCollection* actionCollection();

    virtual Plugin* plugin( const char* libname );

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

  virtual void openURL( const QString &url ) = 0;
  virtual QString url() = 0;

};

class KReadWritePart : public KReadOnlyPart
{
  Q_OBJECT
public:
  KReadWritePart( QWidget *parent = 0, const char *name = 0 );
  virtual ~KReadWritePart();

  virtual bool isModified() const = 0;
  virtual void setModified( bool modified = true ) = 0;
  
  virtual void save( const QString &url ) = 0;
};

#endif
