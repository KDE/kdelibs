/*
* kglobal.cpp -- Implementation of class KGlobal.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sat May  1 02:08:43 EST 1999
*/

#include<assert.h>
#include"kglobal.h"

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
		_dirs = new KStandardDirs( kApp()->appName() );
		_dirs.addKDEDefaults();
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
		_config = kApp()->getConfig();
		_configState = (KGlobal::ConfigState) kApp()->getConfigState();
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
		assert( kApp() );
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
		_charsets = new KCharsets;
	}

	return _charsets;
}

void KGlobal::freeAll()
{	
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
