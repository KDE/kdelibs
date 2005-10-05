/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "config.h"

#include <stdlib.h>

#include <qbytearray.h>
#include <qstring.h>

#include "kinstance.h"
#include "kconfig.h"
#include "kiconloader.h"
#include "kaboutdata.h"
#include "kstandarddirs.h"
#include "kglobal.h"
#include "kmimesourcefactory.h"

#ifndef NDEBUG
  #include <qhash.h>
  static QList<KInstance*> *allInstances = 0;
  typedef QHash<KInstance*, QByteArray*> InstanceDict;
  static InstanceDict *allOldInstances = 0;
  static inline void init_Instances() {
      if (!allInstances) {
          allInstances = new QList<KInstance*>;
	  allOldInstances = new InstanceDict;
      }
  }
  #define DEBUG_ADD do { \
      init_Instances(); \
      allInstances->append(this); \
      allOldInstances->insert(this, new QByteArray(d->name)); \
      } while (false);
  #define DEBUG_REMOVE do { allInstances->removeAll(this); } while (false);
  #define DEBUG_CHECK_ALIVE do { \
      KInstance *key = const_cast<KInstance*>(this);\
      if (!allInstances->contains(key)) {\
          QByteArray *old = allOldInstances->value(key);\
          qFatal("ACCESSING DELETED KINSTANCE! (%s)",\
	         (old ? old->data() : "<unknown>"));\
      }} while (false);
#else
  #define DEBUG_ADD
  #define DEBUG_REMOVE
  #define DEBUG_CHECK_ALIVE
#endif

class KInstance::Private
{
public:
    Private ()
    {
        dirs = 0;
	config = 0;
	iconLoader = 0;
	aboutData = 0;
        mimeSourceFactory = 0L;
    }

    ~Private ()
    {
        if (ownAboutdata)
	    delete aboutData;
	aboutData = 0;
	delete iconLoader;
	iconLoader = 0;
	// do not delete config, stored in d->sharedConfig
	config = 0;
	delete dirs;
	dirs = 0;
        delete mimeSourceFactory;
	mimeSourceFactory = 0;
    }

    KStandardDirs*      dirs;
#define _dirs d->dirs
    KConfig*            config;
#define _config d->config
    KIconLoader*        iconLoader;
#define _iconLoader d->iconLoader
    QByteArray          name;
#define _name d->name
    const KAboutData*   aboutData;
#define _aboutData d->aboutData
    KMimeSourceFactory* mimeSourceFactory;
    QString             configName;
    bool                ownAboutdata;
    KSharedConfig::Ptr  sharedConfig;
};

KInstance::KInstance( const QByteArray& name) : d(new Private)
{
    DEBUG_ADD
    Q_ASSERT(!name.isEmpty());
    if (!KGlobal::_instance)
    {
      KGlobal::_instance = this;
      KGlobal::setActiveInstance(this);
    }

    _name = name;
    _aboutData = new KAboutData(name, "", 0);
    d->ownAboutdata = true;
}

KInstance::KInstance( const KAboutData * aboutData ) : d(new Private)
{
    DEBUG_ADD

    _name = aboutData->appName();
    _aboutData = aboutData;
    d->ownAboutdata = false;

    Q_ASSERT(!_name.isEmpty());

    if (!KGlobal::_instance)
    {
      KGlobal::_instance = this;
      KGlobal::setActiveInstance(this);
    }
}

KInstance::KInstance( KInstance* src ) : d(src->d)
{
    DEBUG_ADD
    Q_ASSERT(!_name.isEmpty());

    if (!KGlobal::_instance || KGlobal::_instance == src )
    {
      KGlobal::_instance = this;
      KGlobal::setActiveInstance(this);
    }

    src->d = 0;
    delete src;
}

KInstance::~KInstance()
{
    DEBUG_CHECK_ALIVE

    delete d;
    d = 0;

    if (KGlobal::_instance == this)
        KGlobal::_instance = 0;
    if (KGlobal::activeInstance() == this)
        KGlobal::setActiveInstance(0);
    DEBUG_REMOVE
}


KStandardDirs *KInstance::dirs() const
{
    DEBUG_CHECK_ALIVE
    if( _dirs == 0 ) {
	_dirs = new KStandardDirs;
        if (_config)
            if (_dirs->addCustomized(_config))
                _config->reparseConfiguration();
    }

    return _dirs;
}

extern bool kde_kiosk_exception;
extern bool kde_kiosk_admin;

KConfig* KInstance::privateConfig() const
{
    DEBUG_CHECK_ALIVE
    return _config;
}

KConfig	*KInstance::config() const
{
    DEBUG_CHECK_ALIVE
    if( _config == 0 ) {
        if ( !d->configName.isEmpty() )
        {
            d->sharedConfig = KSharedConfig::openConfig( d->configName );

            // Check whether custom config files are allowed.
            d->sharedConfig->setGroup( "KDE Action Restrictions" );
            QString kioskException = d->sharedConfig->readEntry("kiosk_exception");
            if (d->sharedConfig->readBoolEntry( "custom_config", true))
            {
               d->sharedConfig->setGroup(QString::null);
            }
            else
            {
               d->sharedConfig = 0;
            }

        }

        if ( d->sharedConfig == 0 )
        {
	    if ( !_name.isEmpty() )
	        d->sharedConfig = KSharedConfig::openConfig( _name + "rc");
	    else
	        d->sharedConfig = KSharedConfig::openConfig( QString::null );
	}

	// Check if we are excempt from kiosk restrictions
	if (kde_kiosk_admin && !kde_kiosk_exception && !QByteArray(getenv("KDE_KIOSK_NO_RESTRICTIONS")).isEmpty())
	{
            kde_kiosk_exception = true;
            d->sharedConfig = 0;
            return config(); // Reread...
        }

	_config = d->sharedConfig;
        if (_dirs)
            if (_dirs->addCustomized(_config))
                _config->reparseConfiguration();
    }

    return _config;
}

KSharedConfig *KInstance::sharedConfig() const
{
    DEBUG_CHECK_ALIVE
    if (_config == 0)
       (void) config(); // Initialize config

    return d->sharedConfig;
}

void KInstance::setConfigName(const QString &configName)
{
    DEBUG_CHECK_ALIVE
    d->configName = configName;
}

KIconLoader *KInstance::iconLoader() const
{
    DEBUG_CHECK_ALIVE
    if( _iconLoader == 0 ) {
	_iconLoader = new KIconLoader( _name, dirs() );
    _iconLoader->enableDelayedIconSetLoading( true );
    }

    return _iconLoader;
}

void KInstance::newIconLoader() const
{
    DEBUG_CHECK_ALIVE
    KIconTheme::reconfigure();
    _iconLoader->reconfigure( _name, dirs() );
}

const KAboutData * KInstance::aboutData() const
{
    DEBUG_CHECK_ALIVE
    return _aboutData;
}

QByteArray KInstance::instanceName() const
{
    DEBUG_CHECK_ALIVE
    return _name;
}

KMimeSourceFactory* KInstance::mimeSourceFactory () const
{
  DEBUG_CHECK_ALIVE
  if (!d->mimeSourceFactory)
  {
    d->mimeSourceFactory = new KMimeSourceFactory(iconLoader());
  }

  return d->mimeSourceFactory;
}

void KInstance::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }
