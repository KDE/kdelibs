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
#include<qfileinfo.h>
#include<qstring.h>
#include<qstringlist.h>

static int tokenize( QStringList& token, const QString& str, 
		const QString& delim );

KStandardDirs::KStandardDirs( const QString& )
{
    dircache.setAutoDelete(true);
}

KStandardDirs::~KStandardDirs()
{
}

void KStandardDirs::addPrefix( QString dir, bool)
{
    if (!prefixes.contains(dir)) {
	prefixes.append(dir);
	dircache.clear();
    }
}

bool KStandardDirs::addResourceType( const QString& type,
		      const QString& relativename )
{
    QStringList *rels = relatives.find(type);
    if (!rels) {
	rels = new QStringList();
	relatives.insert(type, rels);
    }
    if (!rels->contains(relativename)) {
	rels->append(relativename);
	dircache.remove(type); // clean the cache
	return true;
    }
    return false;
}

bool KStandardDirs::addResourceDir( const QString& type, 
		     const QString& absdir, bool)
{
    QStringList *abs = absolutes.find(type);
    if (!abs) {
	abs = new QStringList();
	absolutes.insert(type, abs);
    }
    if (!abs->contains(absdir)) {
	abs->append(absdir);
	dircache.remove(type); // clean the cache
	return true;
    }
    return false;
}

QString KStandardDirs::findResource( const QString& type, 
		      const QString& filename )
{
    QString dir = findResourceDir(type, filename);
    if (dir.isNull())
	return dir;
    else return dir + filename;
}

QString KStandardDirs::findResourceDir( const QString& type,
					const QString& filename)
{
    QStringList *candidates = dircache.find(type);
    if (!candidates) { // filling cache
	candidates = new QStringList();
	QStringList *dirs = absolutes.find(type);
	if (dirs) 
	    for (QStringList::ConstIterator it = dirs->begin(); 
		 it != dirs->end(); it++) {
		debug("adding abs %s for type %s", it->ascii(), type.ascii());
		candidates->append(*it);
	    }
	dirs = relatives.find(type);
	if (dirs) 
	    for (QStringList::ConstIterator pit = prefixes.begin(); 
		 pit != prefixes.end(); pit++)  
		for (QStringList::ConstIterator it = dirs->begin(); 
		     it != dirs->end(); it++) {
		    debug("adding mix %s for type %s", (*pit + *it).ascii(), type.ascii());
		    candidates->append(*pit + *it);
		}
	dircache.insert(type, candidates);
    }
    QDir testdir;
    for (QStringList::ConstIterator it = candidates->begin(); 
	 it != candidates->end(); it++) {
	testdir.setPath(*it);
	if (testdir.exists(filename, false))
	    return *it;
    }
    return QString::null;
}

QString KStandardDirs::findExe( const QString& appname, 
		const QString& pstr, bool ignore)
{
	QFileInfo info;
	QStringList tokens;
	QString p = pstr;
	
	if( p == QString::null ) {
		p = getenv( "PATH" );
	}

	tokenize( tokens, p, ":\b" );

	// split path using : or \b as delimiters
	for( unsigned i = 0; i < tokens.count(); i++ ) {
		p = tokens[ i ];
		p += "/";
		p += appname;

		// Check for executable in this tokenized path
		info.setFile( p );

		if( info.exists() && ( ignore || info.isExecutable() )
			       	&& info.isFile() ) {
			return p;
		}
	}

       	// If we reach here, the executable wasn't found.
	// So return empty string.

	return QString::null;
}

int KStandardDirs::findAllExe( QStringList& list, const QString& appname,
			const QString& pstr, bool ignore )
{
	QString p = pstr;
	QFileInfo info;
	QStringList tokens;

	if( p == QString::null ) {
		p = getenv( "PATH" );
	}

	list.clear();
	tokenize( tokens, p, ":\b" );

	for ( unsigned i = 0; i < tokens.count(); i++ ) {
		p = tokens[ i ];
		p += "/";
		p += appname;

		info.setFile( p );

		if( info.exists() && (ignore || info.isExecutable())
			       	&& info.isFile() ) {
			list.append( p );
		}

	}

	return list.count();
}

static int tokenize( QStringList& tokens, const QString& str, 
		     const QString& delim )
{
	int index = 0;
	int len = str.length();
	QString token = "";
	
	while( index < len ) {
		if ( delim.find( str[ index ] ) >= 0 ) {
			tokens.append( token );
			token = "";
		}
		else {
			token += str[ index ];
		}
	}
	if ( token.length() > 0 ) {
		tokens.append( token );
	}

	return tokens.count();
}

