/*
    This file is part of libkresources
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KRESOURCES_RESOURCE_H
#define KRESOURCES_RESOURCE_H

#include <qmutex.h>
class KConfig;
namespace KRES {

/**
 * @internal
 * @libdoc The KDE Resource library
 *
 * NOTE: this library is NOT (YET?) PUBLIC. Do not publish this
 * interface, it is in constant flux.
 *
 * The KDE Resource framework can be used to manage resources of
 * different types, organized in families. The Resource framework
 * is currently used for addressbook resources in libkabc and for
 * calendar resources in libkcal.
 *
 * When you want to use the framework for a new family, you need to
 * <ul><li>Define a name for your resource family</li>
 * <li>subclass Resource and add the fields and method that are needed
 * in your application</li>
 * <li>If needed, override the doOpen() and doClose() methods.
 * <li> Provide a configuration possibility for resources in your
 * new family. You can use @ref ResourcesConfigPage to easily create a
 * KControl applet</li>
 * <li>In your application, you can use @ref ResourceManager to keep track
 * of the resources in your family, and you can use @ref ResourceSelectDialog
 * to let the user select a single resource.</li>
 * </ul>
 *
 * When you want to add a new resource type to an existing resource family,
 * you need to
 * <ul><li>Further subclass the family-specific Resource to implement
 * resource type-specific operation</li>
 * <li>Subclass ResourceConfigWidget to provide a configuration widget
 * for your new resource type</li>
 * <li>Provide a .desktop file so that the new resource type can be found
 * automatically by the ResourceManager</li>
 * </ul>
 *
 * Example:
 *
<B>resourceexample.h</B>:
<pre>
#include <kconfig.h>
#include <kresources/resource.h>

class ResourceExample : public KRES::ResourceExample
{
public:
  ResourceExample( const KConfig * );
  ~ResourceCalendarExchange();
  void writeConfig( KConfig *config );
private:
  QString mLocation;
  QString mPassword;
}
</pre>
<B>resourceexample.cpp</B>:
<pre>
#include <kconfig.h>

#include "resourceexample.h"

ResourceExample::ResourceExample( const KConfig *config )
    : Resource( config )
{
  if ( config ) {
    mLocation = config->readEntry( "Location" );
    mPassword = decryptStr( config->readEntry( "Password" ) );
  } else {
    mLocation = ""; // Or some sensible default
    mPassword = "";
  }
}

void ResourceExample::writeConfig( KConfig *config )
{
  KRES::Resource::writeConfig( config );
  config->writeEntry( "Location", mLocation );
  config->writeEntry( "Password", encryptStr( mPassword ) );
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
</pre>
* <B>resourceexampleconfig.h</B>:
<pre>
#include <klineedit.h>
#include <kresources/resourceconfigwidget.h>

#include "resourceexample.h"

class ResourceExampleConfig : public KRES::ResourceConfigWidget
{
  Q_OBJECT

public:
  ResourceExampleConfig( QWidget* parent = 0, const char* name = 0 );

public slots:
  virtual void loadSettings( KRES::Resource *resource);
  virtual void saveSettings( KRES::Resource *resource );

private:
  KLineEdit* mLocationEdit;
  KLineEdit* mPasswordEdit;
};
</pre>
* <B>resourceexampleconfig.cpp</B>:
<pre>
#include <qlayout.h>
#include <qlabel.h"
#include <kresources/resourceconfigwidget.h>
#include "resourceexample.h"
#include "resourceexampleconfig.h"

ResourceExampleConfig::ResourceExampleConfig( QWidget* parent,  const char* name )
    : KRES::ResourceConfigWidget( parent, name )
{
  resize( 245, 115 );
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
  ResourceExample* res = dynamic_cast<ResourceExample *>( resource );
  if (res) {
    mHostEdit->setText( res->host() );
    mPasswordEdit->setText( res->password() );
  } else
    kdDebug(5700) << "ERROR: ResourceExampleConfig::loadSettings(): no ResourceExample, cast failed" << endl;
}

void ResourceExampleConfig::saveSettings( KRES::Resource *resource )
{
  ResourceExample* res = dynamic_cast<ResourceExample *>( resource );
  if (res) {
    res->setHost(mHostEdit->text());
    res->setPassword(mPasswordEdit->text());
  } else
    kdDebug(5700) << "ERROR: ResourceExampleConfig::saveSettings(): no ResourceExample, cast failed" << endl;
}
</pre>
* <B>resourceexample.desktop</B>:
<pre>
[Desktop Entry]
Type=Service

[Misc]
Encoding=UTF-8
Name=Example Resource

[Plugin]
Type=exchange
X-KDE-Library=resourceexample
</pre>
* <B>Makefile.am</B>
<pre>
kde_module_LTLIBRARIES = resourceexample.la

resourceexample_la_SOURCES = resourceexample.cpp resourceexampleconfig.cpp
resourceexample_la_LDFLAGS= $(all_libraries) -module $(KDE_PLUGIN)
resourceexample_la_LIBADD= -lkderesources

linkdir= $(kde_datadir)/resources/family
link_DATA= resourceexample.desktop
</pre>
 *
 *
 */

/**
 * A @ref Resource  is a ...
 *
 * A subclass should reimplement at least the constructor and the
 * @ref writeConfig method.
 *
   */
class Resource
{
public:
  /**
   * Constructor. Construct resource from config.
   * @param config Configuration to read persistence information from.
   *               If config==0, create object using default settings.
   */
  Resource( const KConfig* config );

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
  virtual void writeConfig( KConfig* config );

  /**
   * Open this resource, if it not already open. Increase the open
   * count of this object, and open the resource by calling @ref doOpen().
   * This method may block while another thread is concurrently opening
   * or closing the resource.
   *
   * Returns true if the resource was already opened or if it was opened
   * successfully; returns false if the resource was not opened successfully.
   */
  bool open();

  /**
   * Decrease the open count of this object, and if the count reaches
   * zero, close this resource by calling @ref doClose().
   * This method may block while another thread is concurrently closing
   * or opening the resource.
   */
  void close();

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
  QString type() { return mType; }

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
   * Set the name of resource.You can override this method,
   * but also remember to call Resource::setResourceName().
   */
  virtual void setResourceName( const QString &name );

  /**
   * Returns the name of resource.
   */
  virtual QString resourceName() const;

  /**
   * This method can be used by all resources to encrypt
   * their passwords for storing in a config file.
   */
  static QString encryptStr( const QString & );
  /**
   * This method can be used by all resources to decrypt
   * their passwords read from a config file.
   */
  static QString decryptStr( const QString & );

  friend class ResourceFactory;
  friend class ResourceManagerImpl;

protected:
  /**
   * Open this resource. When called, the resource must be in
   * a closed state.
   *
   * Returns true if the resource was opened successfully;
   * returns false if the resource was not opened successfully.
   */
  virtual bool doOpen() { return true; }
  /**
   * Close this resource. Pre-condition: resource is open.
   * Post-condition: resource is closed.
   */
  virtual void doClose() {}

private:
  void setIdentifier( const QString& identifier ) { mIdentifier = identifier; }
  void setType( const QString& type ) { mType = type; }

  QMutex mMutex;
  int mOpenCount;
  QString mType;
  QString mIdentifier;
  bool mReadOnly;
  QString mName;
};

}
#endif
