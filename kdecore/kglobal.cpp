/*
* kglobal.cpp -- Implementation of class KGlobal.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sat May  1 02:08:43 EST 1999
*/

#include<assert.h>
#include"kglobal.h"

#include<qpopupmenu.h>

#include<kapp.h>

#include<kconfig.h>
#include<klocale.h>
#include<kcharsets.h>
#include<kiconloader.h>
#include<kstddirs.h>

KApplication *KGlobal::kApp()
{
	assert( kapp != 0 );

	return kapp;
}

KStandardDirs *KGlobal::dirs()
{
	if( _dirs == 0 ) {
		_dirs = new KStandardDirs( kApp()->getAppName() );
	}

	return _dirs;
}

KGlobal::ConfigState KGlobal::configState()
{
	return _configState;
}

KConfig	*KGlobal::config()
{
	if( _config == 0 ) {
		// TODO: Create global config
	}

	return _config;
}

KConfig	*KGlobal::instanceConfig()
{
	if( _instanceConfig == 0 ) {
		// TODO: Create instance config
	}

	return _instanceConfig;
}

KIconLoader *KGlobal::iconLoader()
{
	assert( kapp != 0 );
	
	if( _iconLoader == 0 ) {
		_iconLoader = new KIconLoader();
	}

	return _iconLoader;
}

bool KGlobal::localeConstructed()
{
	return (_locale != 0 );
}

KLocale	*KGlobal::locale()
{	
	if( _locale == 0 ) {
		_locale = new KLocale;
	}

	return _locale;
}

KCharsets *KGlobal::charsets()
{
	if( _charsets == 0 ) {
		_charsets = new KCharsets;
	}

	return _charsets;
}

QPopupMenu *KGlobal::helpMenu()

	if( _helpMenu == 0 ) {
		// TODO: Create help menu
	}

	return _helpMenu;
}

void KGlobal::cleanup()
{	
	delete _helpMenu;	_helpMenu = 0;
	delete _iconLoader;	_iconLoader = 0;

	delete _locale;		_locale = 0;
	delete _charsets;	_charsets = 0;

	delete _config;		_config = 0;
	delete _instanceConfig;	_instanceConfig = 0;
	_configState =  APPCONFIG_NONE;

	delete _dirs;		_dirs = 0;
}

// The Variables

KApplication    *KGlobal::_kapp		= 0;
KStandardDirs   *KGlobal::_dirs		= 0;

KGlobal::ConfigState KGlobal::_configState	= KGlobal::APPCONFIG_NONE;
KConfig         *KGlobal::_config	= 0;
KConfig         *KGlobal::_instanceConfig	= 0;
KIconLoader     *KGlobal::_iconLoader	= 0;

KLocale         *KGlobal::_locale	= 0;
KCharsets       *KGlobal::_charsets	= 0;

QPopupMenu      *KGlobal::_helpMenu	= 0;

