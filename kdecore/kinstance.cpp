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
#include "kinstance.h"

#include <stdlib.h>
#include <unistd.h>

#include "kconfig.h"
#include "klocale.h"
#include "kcharsets.h"
#include "kiconloader.h"
#include "kaboutdata.h"
#include "kstandarddirs.h"
#include "kdebug.h"
#include "kglobal.h"
#include "kmimesourcefactory.h"

#include <qfont.h>

#include "config.h"
#ifndef NDEBUG
  #include <assert.h>
  #include <qptrdict.h>
  static QPtrList<KInstance> *allInstances = 0;
  static QPtrDict<QCString> *allOldInstances = 0;
  #define DEBUG_ADD do { if (!allInstances) { allInstances = new QPtrList<KInstance>(); allOldInstances = new QPtrDict<QCString>(); } allInstances->append(this); allOldInstances->insert( this, new QCString( _name)); } while (false);
  #define DEBUG_REMOVE do { allInstances->removeRef(this); } while (false);
  #define DEBUG_CHECK_ALIVE do { if (!allInstances->contains((KInstance*)this)) { QCString *old = allOldInstances->find((KInstance*)this); qWarning("ACCESSING DELETED KINSTANCE! (%s)", old ? old->data() : "<unknown>"); assert(false); } } while (false);
#else
  #define DEBUG_ADD
  #define DEBUG_REMOVE
  #define DEBUG_CHECK_ALIVE
#endif

class KInstancePrivate
{
public:
    KInstancePrivate ()
    {
        mimeSourceFactory = 0L;
    }

    ~KInstancePrivate ()
    {
        delete mimeSourceFactory;
    }

    KMimeSourceFactory* mimeSourceFactory;
    QString configName;
    bool ownAboutdata;
    KSharedConfig::Ptr sharedConfig;
};

KInstance::KInstance( const QCString& name)
  : _dirs (0L),
    _config (0L),
    _iconLoader (0L),
    _name( name ), _aboutData( new KAboutData( name, "", 0 ) )
{
    DEBUG_ADD
    Q_ASSERT(!name.isEmpty());
    if (!KGlobal::_instance)
    {
      KGlobal::_instance = this;
      KGlobal::setActiveInstance(this);
    }

    d = new KInstancePrivate ();
    d->ownAboutdata = true;
}

KInstance::KInstance( const KAboutData * aboutData )
  : _dirs (0L),
    _config (0L),
    _iconLoader (0L),
    _name( aboutData->appName() ), _aboutData( aboutData )
{
    DEBUG_ADD
    Q_ASSERT(!_name.isEmpty());

    if (!KGlobal::_instance)
    {
      KGlobal::_instance = this;
      KGlobal::setActiveInstance(this);
    }

    d = new KInstancePrivate ();
    d->ownAboutdata = false;
}

KInstance::KInstance( KInstance* src )
  : _dirs ( src->_dirs ),
    _config ( src->_config ),
    _iconLoader ( src->_iconLoader ),
    _name( src->_name ), _aboutData( src->_aboutData )
{
    DEBUG_ADD
    Q_ASSERT(!_name.isEmpty());

    if (!KGlobal::_instance || KGlobal::_instance == src )
    {
      KGlobal::_instance = this;
      KGlobal::setActiveInstance(this);
    }

    d = new KInstancePrivate ();
    d->ownAboutdata = src->d->ownAboutdata;
    d->sharedConfig = src->d->sharedConfig;

    src->_dirs = 0L;
    src->_config = 0L;
    src->_iconLoader = 0L;
    src->_aboutData = 0L;
    delete src;
}

KInstance::~KInstance()
{
    DEBUG_CHECK_ALIVE

    if (d->ownAboutdata)
        delete _aboutData;
    _aboutData = 0;

    delete d;
    d = 0;

    delete _iconLoader;
    _iconLoader = 0;

    // delete _config; // Do not delete, stored in d->sharedConfig
    _config = 0;
    delete _dirs;
    _dirs = 0;

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
	_dirs = new KStandardDirs( );
        if (_config)
            if (_dirs->addCustomized(_config))
                _config->reparseConfiguration();
    }

    return _dirs;
}

extern bool kde_kiosk_exception;
extern bool kde_kiosk_admin;

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
	if (kde_kiosk_admin && !kde_kiosk_exception && !QCString(getenv("KDE_KIOSK_NO_RESTRICTIONS")).isEmpty())
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

QCString KInstance::instanceName() const
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

