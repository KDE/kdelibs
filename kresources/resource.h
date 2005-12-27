/*
    This file is part of libkresources

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
#ifndef KRESOURCES_RESOURCE_H
#define KRESOURCES_RESOURCE_H

#include <qmutex.h>

#include <qwidget.h>

#include <klibloader.h>

class KConfig;

namespace KRES {

class ConfigWidget;

/**
  \mainpage The KDE Resource library
 
  The KDE Resource framework can be used to manage resources of
  different types, organized in families. The Resource framework
  is for example used for addressbook resources in libkabc and for
  calendar resources in libkcal.
 
  When you want to use the framework for a new family, you need to
  <ul><li>Define a name for your resource family</li>
  <li>subclass Resource and add the fields and method that are needed
  in your application</li>
  <li>If needed, override the doOpen() and doClose() methods.
  <li>In your application, you can use ResourceManager to keep track
  of the resources in your family, and you can use ResourceSelectDialog
  to let the user select a single resource.</li>
  </ul>
 
  When you want to add a new resource type to an existing resource family,
  you need to
  <ul><li>Further subclass the family-specific Resource to implement
  resource type-specific operation</li>
  <li>Subclass ResourceConfigWidget to provide a configuration widget
  for your new resource type</li>
  <li>Provide a .desktop file so that the new resource type can be found
  automatically by the ResourceManager</li>
  </ul>
 
  Example:
 
  <B>resourceexample.h</B>:
\code
#include <kconfig.h>
#include <kresources/resource.h>

class ResourceExample : public KRES::Resource
{
  public:
    ResourceExample( const KConfig * );
    ~ResourceExample();
    void writeConfig( KConfig *config );

  private:
    QString mLocation;
    QString mPassword;
}
\endcode

  <B>resourceexample.cpp</B>:
\code
#include <kconfig.h>

#include "resourceexample.h"

ResourceExample::ResourceExample( const KConfig *config )
    : Resource( config )
{
  if ( config ) {
    mLocation = config->readPathEntry( "Location" );
    mPassword = KStringHandler::obscure( config->readEntry( "Password" ) );
  } else {
    mLocation = ""; // Or some sensible default
    mPassword = "";
  }
}

void ResourceExample::writeConfig( KConfig *config )
{
  KRES::Resource::writeConfig( config );
  config->writePathEntry( "Location", mLocation );
  config->writeEntry( "Password", KStringHandler::obscure( mPassword ) );
}

extern "C"
{
  KRES::ResourceExample *config_widget( QWidget *parent ) {
    return new ResourceExampleConfig( parent, "Configure Example Resource" );
  }

  KRES::Resource *resource( const KConfig *config ) {
    return new ResourceExample( config );
  }
}
\endcode

  <B>resourceexampleconfig.h</B>:
\code
#include <klineedit.h>
#include <kresources/resourceconfigwidget.h>

#include "resourceexample.h"

class ResourceExampleConfig : public KRES::ResourceConfigWidget
{
    Q_OBJECT
  public:
    ResourceExampleConfig( QWidget *parent = 0, const char *name = 0 );

  public slots:
    virtual void loadSettings( KRES::Resource *resource);
    virtual void saveSettings( KRES::Resource *resource );

  private:
    KLineEdit *mLocationEdit;
    KLineEdit *mPasswordEdit;
};
\endcode

  <B>resourceexampleconfig.cpp</B>:
\code
#include <qlayout.h>
#include <qlabel.h"
#include <kresources/resourceconfigwidget.h>
#include <QList>
#include "resourceexample.h"
#include "resourceexampleconfig.h"

ResourceExampleConfig::ResourceExampleConfig( QWidget *parent,  const char *name )
  : KRES::ResourceConfigWidget( parent, name )
{
  QGridLayout *mainLayout = new QGridLayout( this, 2, 2 );

  QLabel *label = new QLabel( i18n( "Location:" ), this );
  mHostEdit = new KLineEdit( this );
  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mHostEdit, 1, 1 );

  label = new QLabel( i18n( "Password:" ), this );
  mPasswordEdit = new KLineEdit( this );
  mPasswordEdit->setEchoMode( QLineEdit::Password );
  mainLayout->addWidget( label, 2, 0 );
  mainLayout->addWidget( mPasswordEdit, 2, 1 );
}

void ResourceExampleConfig::loadSettings( KRES::Resource *resource )
{
  ResourceExample *res = dynamic_cast<ResourceExample *>( resource );
  if ( res ) {
    mHostEdit->setText( res->host() );
    mPasswordEdit->setText( res->password() );
  } else
    kdDebug() << "ERROR: ResourceExampleConfig::loadSettings(): no ResourceExample, cast failed" << endl;
}

void ResourceExampleConfig::saveSettings( KRES::Resource *resource )
{
  ResourceExample *res = dynamic_cast<ResourceExample *>( resource );
  if ( res ) {
    res->setHost( mHostEdit->text() );
    res->setPassword( mPasswordEdit->text() );
  } else
    kdDebug() << "ERROR: ResourceExampleConfig::saveSettings(): no ResourceExample, cast failed" << endl;
}
\endcode

  <B>resourceexample.desktop</B>:
\code
[Desktop Entry]
Type=Service

[Misc]
Encoding=UTF-8
Name=Example Resource

[Plugin]
Type=exchange
X-KDE-Library=resourceexample
\endcode

  <B>Makefile.am</B>
\code
kde_module_LTLIBRARIES = resourceexample.la

resourceexample_la_SOURCES = resourceexample.cpp resourceexampleconfig.cpp
resourceexample_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
resourceexample_la_LIBADD = -lkresources

servicedir = $(kde_datadir)/resources/example
service_DATA = resourceexample.desktop
\endcode

*/

/**
  This class provides a resource which is managed in a general way.
  
  A Resource represents the concept of an object with the following attributes:

  - Applications operate on sets of one or more Resource objects.
  - Creation and deletetion of Resource objects is done in a general way,
    independent of concrete functionality of the Resource.
  - The end user has control over creation, deletion and configuration of
    Resource object.
  - Properties, behaviour and configuration of different Resource objects can
    widely differ.
  - Resources can be active or inactive.
  - There is one special Resource which is the standard Resource. This can for
    example be used as default destination for newly created object managed
    by a certain Resource family.
  - Activation of Resources can be covered by a two step process of being opened
    and then loaded. Deactivation corresponds to saving and closing.
  - Different application ususally share the same set of Resources.

  The Resource base class provides the management functionality. Classes
  inheriting from Resource automatically appear in the general kresources
  kcontrol module.

  Concrete functionality of Resources is specified per family by a subclass of
  Resource. This classes in turn have subclasses which implement the different
  flavours of the functionality represented by the family.
 
  A subclass should reimplement at least the constructor and the
  writeConfig method.

  An example for a Resource subclass hierarchy would be the "calendar" family.
  The ResourceCalendar subclass would specify an API for accessing calendar
  data. Subclasses of ResourceCalendar would implement this API for local files,
  remote files, specific calendar servers etc.
*/
class KRESOURCES_EXPORT Resource : public QObject
{
    friend class Factory;
    friend class ManagerImpl;

    Q_OBJECT
  public:
    typedef QList<Resource *> List;

    /**
     * Constructor. Construct resource from config.
     * @param config Configuration to read persistence information from.
     *               If config is 0, create object using default settings.
     */
    Resource( const KConfig *config );

    /**
     * Destructor.
     */
    virtual ~Resource();

    /**
     * Write configuration information for this resource to a configuration
     * file. If you override this method, remember to call Resource::writeConfig
     * or Terrible Things(TM) will happen.
     * @param config Configuration to write persistence information to.
     */
    virtual void writeConfig( KConfig *config );

    /**
     * Open this resource, if it not already open. Increase the open
     * count of this object, and open the resource by calling doOpen().
     * This method may block while another thread is concurrently opening
     * or closing the resource.
     *
     * Returns true if the resource was already opened or if it was opened
     * successfully; returns false if the resource was not opened successfully.
     */
    bool open();

    /**
     * Decrease the open count of this object, and if the count reaches
     * zero, close this resource by calling doClose().
     * This method may block while another thread is concurrently closing
     * or opening the resource.
     */
    void close();

    /**
     * Returns whether the resource is open or not.
     */
    bool isOpen() const;

    /**
     * Returns a unique identifier. The identifier is unique for this resource.
     * It is created when the resource is first created, and it is retained
     * in the resource family configuration file for this resource.
     * @return This resource's identifier
     */
    QString identifier() const;

    /**
     * Returns the type of this resource.
     */
    QString type() const;

    /**
     * Mark the resource as read-only. You can override this method,
     * but also remember to call Resource::setReadOnly().
     */
    virtual void setReadOnly( bool value );

    /**
     * Returns, if the resource is read-only.
     */
    virtual bool readOnly() const;

    /**
     * Set the name of resource. You can override this method,
     * but also remember to call Resource::setResourceName().
     */
    virtual void setResourceName( const QString &name );

    /**
     * Returns the name of resource.
     */
    virtual QString resourceName() const;

    /**
      Sets, if the resource is active.
    */
    void setActive( bool active );

    /**
      Return true, if the resource is active.
    */
    bool isActive() const;

    /**
      Print resource information as debug output.
    */
    virtual void dump() const;

  protected:
    /**
     * Open this resource. When called, the resource must be in
     * a closed state.
     *
     * Returns true if the resource was opened successfully;
     * returns false if the resource was not opened successfully.
     *
     * The result of this call can be accessed later by isOpen()
     */
    virtual bool doOpen() { return true; }

    /**
     * Close this resource. Pre-condition: resource is open.
     * Post-condition: resource is closed.
     */
    virtual void doClose() {}

    void setIdentifier( const QString &identifier );
    void setType( const QString &type );

  private:
    class ResourcePrivate;
    ResourcePrivate *d;
};

class KRESOURCES_EXPORT PluginFactoryBase : public KLibFactory
{
  public:
    virtual Resource *resource( const KConfig *config ) = 0;

    virtual ConfigWidget *configWidget( QWidget *parent ) = 0;

  protected:
    virtual QObject* createObject( QObject *, const char *, const char *,
                                   const QStringList & )
    {
      return 0;
    }
};

template<class TR,class TC>
class PluginFactory : public PluginFactoryBase
{
  public:
    Resource *resource( const KConfig *config )
    {
      return new TR( config );
    }

    ConfigWidget *configWidget( QWidget *parent )
    {
      return new TC( parent );
    }
};

}

#endif
