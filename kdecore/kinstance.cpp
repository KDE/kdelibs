#include "kinstance.h"

#include "kconfig.h"
#include "klocale.h"
#include "kcharsets.h"
#include "kiconloader.h"
#include "kstddirs.h"
#include "kglobal.h"

#include <qfont.h>

KInstance::KInstance( const QCString& name )
    : _name( name )
{
    ASSERT(!name.isEmpty());
    if (!KGlobal::_instance)
      KGlobal::_instance = this;
    _iconLoader = 0;
    _config = 0;
    _dirs = 0;
}

KInstance::~KInstance()
{	
    delete _iconLoader;
    _iconLoader = 0;
    delete _config;
    _config = 0;
    delete _dirs;
    _dirs = 0;
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
	_iconLoader = new KIconLoader( this );
    }
    
    return _iconLoader;
}

QCString KInstance::instanceName() const
{
    return _name;
}
