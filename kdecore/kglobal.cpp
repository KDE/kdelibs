/*
* kglobal.cpp -- Implementation of class KGlobal.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sat May  1 02:08:43 EST 1999
*/

#include <assert.h>
#include "kglobal.h"

#include <kapp.h>

#include <kconfig.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kiconloader.h>
#include <kstddirs.h>

#include <qfont.h>

KApplication *KGlobal::kApp()
{
//	assert( kapp != 0 ); 	// This would be good, but not practical

	return kapp;
}

KStandardDirs *KGlobal::dirs()
{
	if( _dirs == 0 ) {
		_dirs = new KStandardDirs( );
		_dirs->addKDEDefaults();
	}

	return _dirs;
}

KConfig	*KGlobal::config()
{
        if( _config == 0 ) {
	    if (kapp)
		_config = new KConfig(QString(kapp->name()) + "rc");
	    else
		_config = new KConfig();
	}

	return _config;
}

KConfig	*KGlobal::instanceConfig()
{
	if( _instanceConfig == 0 ) {
		_instanceConfig = kApp()->getSessionConfig();
	}

	return _instanceConfig;
}

KIconLoader *KGlobal::iconLoader()
{
    if( _iconLoader == 0 ) {
	_iconLoader = new KIconLoader();
    }
    
    return _iconLoader;
}

KLocale	*KGlobal::locale()
{	
	if( _locale == 0 ) {
		// will set _locale if it works - otherwise 0 is returned
		KLocale::initInstance();
	}

	return _locale;
}

KCharsets *KGlobal::charsets()
{
	if( _charsets == 0 ) {
		_charsets =new KCharsets();
	}

	return _charsets;
}

QFont KGlobal::generalFont()
{
    if(_generalFont) return *_generalFont;

    KConfig *c = KGlobal::config();

    c->setGroup( "General" );
    _generalFont = new QFont(c->readFontEntry("font"));
    if( *_generalFont == QFont::defaultFont() )
    {
	*_generalFont = QFont("helvetica", 12, QFont::Normal);
	charsets()->setQFont(*_generalFont, charsets()->charsetForLocale());
    }
    return *_generalFont;
}
	
QFont KGlobal::fixedFont()
{
    if(_fixedFont) return *_fixedFont;

    KConfig *c = KGlobal::config();
    c->setGroup( "General" );
    _fixedFont = new QFont(c->readFontEntry("fixedFont"));
    if( *_fixedFont == QFont::defaultFont() )
    {
      *_fixedFont = QFont("fixed", 12, QFont::Normal);
      charsets()->setQFont(*_fixedFont, charsets()->charsetForLocale());
    }
    return *_fixedFont;
}
 


void KGlobal::freeAll()
{	
	delete _iconLoader;
	_iconLoader = 0;
	delete _locale;
	_locale = 0;
	delete _charsets;
	_charsets = 0;
	delete _generalFont;
	_generalFont = 0;
	delete _fixedFont;
	_fixedFont = 0;
	delete _config;
	_config = 0;
	delete _instanceConfig;
	_instanceConfig = 0;
	delete _dirs;
	_dirs = 0;
}
	
// The Variables

KApplication    *KGlobal::_kapp		= 0;
KStandardDirs   *KGlobal::_dirs		= 0;

KConfig         *KGlobal::_config	= 0;
KConfig         *KGlobal::_instanceConfig	= 0;
KIconLoader     *KGlobal::_iconLoader	= 0;

KLocale         *KGlobal::_locale	= 0;
KCharsets       *KGlobal::_charsets	= 0;

QFont           *KGlobal::_generalFont  = 0;
QFont           *KGlobal::_fixedFont    = 0;
