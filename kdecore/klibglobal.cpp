#include "klibglobal.h"

#include "kconfig.h"
#include "klocale.h"
#include "kcharsets.h"
#include "kiconloader.h"
#include "kstddirs.h"
#include "kglobal.h"

#include <qfont.h>

KLibGlobal::KLibGlobal( const QString& name )
    : _name( name )
{
    _iconLoader = 0;
    _config = 0;
    _dirs = 0;
}

KLibGlobal::~KLibGlobal()
{	
	delete _iconLoader;
	_iconLoader = 0;
	delete _config;
	_config = 0;
	delete _dirs;
	_dirs = 0;
}


KStandardDirs *KLibGlobal::dirs()
{
	if( _dirs == 0 ) {
		_dirs = new KStandardDirs( );
		_dirs->addKDEDefaults();
	}

	return _dirs;
}

KConfig	*KLibGlobal::config()
{
        if( _config == 0 ) {
	    if ( !_name.isEmpty() )
		_config = new KConfig( _name + "rc");
	    else
		_config = new KConfig();
	}

	return _config;
}

KIconLoader *KLibGlobal::iconLoader()
{
    if( _iconLoader == 0 ) {
	_iconLoader = new KIconLoader( this, name() );
    }

    return _iconLoader;
}

KLocale	*KLibGlobal::locale()
{	
    return KGlobal::locale();
}

KCharsets *KLibGlobal::charsets()
{
    return KGlobal::charsets();
}

QFont KLibGlobal::generalFont()
{
    return KGlobal::generalFont();
}
	
QFont KLibGlobal::fixedFont()
{
    return KGlobal::fixedFont();
}

int KLibGlobal::dndEventDelay()
{
    return KGlobal::dndEventDelay();
}

QString KLibGlobal::name() const
{
    return _name;
}
