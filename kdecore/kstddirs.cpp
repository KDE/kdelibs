/*
* kstddirs.cpp -- Implementation of class KStandardDirs.
* Author:	Sirtaj Singh Kang <taj@kde.org>
* Version:	$Id$
* Generated:	Thu Mar  5 16:05:28 EST 1998
*/

#include"kstddirs.h"
#include"config.h"

#include<stdlib.h>

#include<qdict.h>
#include<qdir.h>
#include<qfileinf.h>
#include<qstring.h>
#include<qstrlist.h>

#warning Hard codes /opt/kde
#define KDEDIR "/opt/kde"
#define KDEDIR_LEN 6

static const char *tokenize( QString& token, const char *str, 
		const char *delim );
static const char *selectStr( const char *env, 
		const char *builtin );

KStandardDirs::KStandardDirs( const char *appName ) :
	UserDir (	QDir::homeDirPath() ),
	KDEDir	(	selectStr( "KDEDIR", KDEDIR ) ),
	_appPath(	0	),
	_appName(	new QString( appName ) ),
	_app	(	new QDict<QString> ),
	_sysapp	(	new QDict<QString> ),
	_sys	(	new QDict<QString> ),
	_user	(	new QDict<QString> )

{
	if( _app ) _app->setAutoDelete( true );
	if( _sysapp ) _sysapp->setAutoDelete( true );
	if( _sys ) _sys->setAutoDelete( true );
	if( _user) _user->setAutoDelete( true );
}

KStandardDirs::~KStandardDirs()
{
	delete _app;
	delete _sysapp;
	delete _sys;
	delete _user;

	delete _appName;
}

QString KStandardDirs::findExe( const char *appname, 
		const char *pathstr, bool ignore)
{
	if( pathstr == 0 ) {
		pathstr = getenv( "PATH" );
	}

	QString onepath;
	QFileInfo info;

	const char *p = pathstr;

	// split path using : or \b as delimiters
        while( (p = tokenize( onepath, p, ":\b") ) != 0 ) {
		onepath += "/";
		onepath += appname;

		// Check for executable in this tokenized path
		info.setFile( onepath );

		if( info.exists() && ( ignore || info.isExecutable() )
			       	&& info.isFile() ) {
			return onepath;
		}
        }

	// If we reach here, the executable wasn't found.
	// So return empty string.

	onepath = (const char *)0;

	return onepath;
}

int KStandardDirs::findAllExe( QStrList& list, const char *appname,
			const char *pathstr=0, bool ignore )
{
	if( pathstr == 0 ) {
		pathstr = getenv( "PATH" );
	}

	QString onepath;
	QFileInfo info;

	const char *p = pathstr;

	list.clear();

	// split path using : or \b as delimiters
        while( (p = tokenize( onepath, p, ":\b") ) != 0 ) {
		onepath += "/";
		onepath += appname;

		// Check for executable in this tokenized path
		info.setFile( onepath );

		if( info.exists() && (ignore || info.isExecutable())
			       	&& info.isFile() ) {
			list.append( onepath );
		}
        }

	return list.count();
}


const char *KStandardDirs::closest( DirScope method, 
		const char *suffix ) const
{
	CHECK_PTR( suffix );
	CHECK_PTR( _app );
	CHECK_PTR( _sys );
	CHECK_PTR( _user );

	const char *sys, *sysapp, *user, *app;
	QString *found = _user->find( suffix );
       	

	// check dict for previous full-string insertion
	if( found == 0 ) {
		// not already inserted, so insert them now.
		const char *realsuffix = suffix;
		if( !strncmp( suffix, "KDEDIR", 
				KDEDIR_LEN ) ) {
			// remove prefix KDEDIR from path
			realsuffix += KDEDIR_LEN;
		}

		// system dir
		QString *name = new QString( KDEDir );
		*name += realsuffix;
		sys = name->data();

		const_cast<KStandardDirs*>(this)->_sys->insert( 
				suffix, name );

		// system app dir
		name = new QString( KDEDir );
		name->detach();

		*name += "/share/apps/";
		*name += realsuffix;

		sysapp = name->data();

		const_cast<KStandardDirs*>(this)->_sysapp->insert( 
				suffix, name );

		// user dir
		name = new QString( UserDir );
		name->detach();

		*name += "/.kde";
		*name += realsuffix;


		user = name->data();

		const_cast<KStandardDirs*>(this)->_user->insert( 
				suffix, name );


		// app dir
		name = new QString( UserDir );
		name->detach();

		*name += "/.kde/share/apps/";
		*name += *_appName;
		*name += realsuffix;

		app = name->data();

		const_cast<KStandardDirs*>(this)->_app->insert( 
				suffix, name );

	}
	else {
		// string's already in there
		user = found->data(); 
		sys = _sys->find( suffix )->data();
		sysapp = _sysapp->find( suffix )->data();
		app = _app->find( suffix )->data();
	}
			
	// return specific dirs

	switch( method ) {
		case User:	return user;
		case System:	return sys;
		case SysApp:	return sysapp;
		case App:	return app;
		default:	break;
	}

	// find closest

	QFileInfo info( app );	// app

	if( info.isDir() ) {
		return app;
	}

	info.setFile( user );	// user

	if( info.isDir() ) {
		return user;
	}

	info.setFile( sysapp );	// sysapp

	if( info.isDir() ) {
		return sysapp;
	}

	return sys;		// sys
}

const char *KStandardDirs::app( KStandardDirs::DirScope s ) const
{
	if( _appPath == 0 ) {
		QString app( "/share/apps/" );
		app += _appName->data();

		const_cast<KStandardDirs*>(this)->_appPath
			= closest( s, app.data() );
	}

	return _appPath;
}

const char *KStandardDirs::bin( DirScope s ) const
{
	return closest( s, KDE_BINDIR );
}

const char *KStandardDirs::cgi( DirScope s ) const
{

	return closest( s, KDE_CGIDIR );
}

const char *KStandardDirs::config( DirScope s ) const
{
	return closest( s, KDE_CONFIGDIR );
}

const char *KStandardDirs::apps( DirScope s ) const
{
	return closest( s, KDE_DATADIR );
}

const char *KStandardDirs::html( DirScope s ) const
{
	return closest( s, KDE_HTMLDIR );
}

const char *KStandardDirs::icon( DirScope s ) const
{
	return closest( s, KDE_ICONDIR );
}

const char *KStandardDirs::locale( DirScope s ) const
{
	return closest( s, KDE_LOCALE );
}

const char *KStandardDirs::mime( DirScope s ) const
{
	return closest( s, KDE_MIMEDIR );
}

const char *KStandardDirs::parts( DirScope s ) const
{
	return closest( s, KDE_PARTSDIR );
}

const char *KStandardDirs::toolbar( DirScope s ) const
{
	return closest( s, KDE_TOOLBARDIR );
}

const char *KStandardDirs::wallpaper( DirScope s ) const
{
	return closest( s, KDE_WALLPAPERDIR );
}

const char *KStandardDirs::sound( DirScope s ) const
{
	return closest( s, KDE_SOUNDDIR );
}

static const char *tokenize( QString& token, const char *str, 
		const char *delim )
{
        token = "";

        if( !str || !*str ) {
                return 0;
        }

        // find first non-delimiter character
        while( *str && strchr( delim, *str ) ) {
                str++;
        }

        if( !*str ) {
                return 0;
        }

        // find first delimiter or end, storing each non-qualifier into token
        while( *str && !strchr( delim, *str ) ) {
                token += *str;
                str++;
        }

        return str;
}

static const char *selectStr( const char *env, const char *builtin )
{
	const char *result = getenv( env );

	if( result == 0 ) {
		result = builtin;
	}

	return result;
}
