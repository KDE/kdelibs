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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#include "kinstance.h"

#include "kconfig.h"
#include "klocale.h"
#include "kcharsets.h"
#include "kiconloader.h"
#include "kaboutdata.h"
#include "kstddirs.h"
#include "kdebug.h"
#include "kglobal.h"
#include "kmimesourcefactory.h"

#include <qfont.h>

class KInstancePrivate
{
public:
  KInstancePrivate (KInstance* p)
    : mimeSourceFactory (new KMimeSourceFactory(p->iconLoader()))
  {
	QMimeSourceFactory::setDefaultFactory (mimeSourceFactory);
  }

  ~KInstancePrivate ()
  {
    delete mimeSourceFactory;
  }

  KMimeSourceFactory* mimeSourceFactory;
};

KInstance::KInstance( const QCString& name)
  : _dirs (0L),
    _config (0L),
    _iconLoader (0L),
    _name( name ), _aboutData( new KAboutData( name, "", 0 ) )
{
    ASSERT(!name.isEmpty());
    if (!KGlobal::_instance)
    {
      KGlobal::_instance = this;
      KGlobal::_activeInstance = this;
    }

    d = new KInstancePrivate (this);

    kdDebug() << "Instance " << _name.data() << " has no about data" << endl;
}

KInstance::KInstance( const KAboutData * aboutData )
  : _dirs (0L),
    _config (0L),
    _iconLoader (0L),
    _name( aboutData->appName() ), _aboutData( aboutData )
{
    ASSERT(!_name.isEmpty());

    if (!KGlobal::_instance)
    {
      KGlobal::_instance = this;
      KGlobal::_activeInstance = this;
    }

    d = new KInstancePrivate (this);
}

KInstance::~KInstance()
{
  delete d;
  d = 0;

    delete _iconLoader;
    _iconLoader = 0;
    delete _config;
    _config = 0;
    delete _dirs;
    _dirs = 0;

    if (KGlobal::_instance == this)
        KGlobal::_instance = 0;
    if (KGlobal::_activeInstance == this)
        KGlobal::_activeInstance = 0;
}


KStandardDirs *KInstance::dirs() const
{
    if( _dirs == 0 ) {
	_dirs = new KStandardDirs( );
	_dirs->addKDEDefaults();
    }

    return _dirs;
}

KConfig	*KInstance::config() const
{
    if( _config == 0 ) {
	if ( !_name.isEmpty() )
	    _config = new KConfig( _name + "rc");
	else
	    _config = new KConfig();
    }

    return _config;
}

KIconLoader *KInstance::iconLoader() const
{
    if( _iconLoader == 0 ) {
	_iconLoader = new KIconLoader( _name, dirs() );
    }

    return _iconLoader;
}

void KInstance::newIconLoader() const
{
    delete _iconLoader;
    _iconLoader = 0;
}

const KAboutData * KInstance::aboutData() const
{
    return _aboutData;
}

QCString KInstance::instanceName() const
{
    return _name;
}

KMimeSourceFactory* KInstance::mimeSourceFactory () const
{
  return d->mimeSourceFactory;
}
