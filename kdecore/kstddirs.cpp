/*
* kstddirs.cpp -- Implementation of class KStandardDirs.
* Author:	Sirtaj Singh Kang <taj@kde.org>
* Version:	$Id$
* Generated:	Thu Mar  5 16:05:28 EST 1998
*/


#include "config.h"

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <iostream.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <qdict.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qstringlist.h>

#include "kstddirs.h"
#include "kglobal.h"

static int tokenize( QStringList& token, const QString& str,
		const QString& delim );

KStandardDirs::KStandardDirs( const QString& )
{
    dircache.setAutoDelete(true);
    relatives.setAutoDelete(true);
    absolutes.setAutoDelete(true);
}

KStandardDirs::~KStandardDirs()
{
}

void KStandardDirs::addPrefix( QString dir, bool)
{
    if (dir.isNull())
      return;

    if (dir.at(dir.length() - 1) != '/')
	dir += '/';

    if (!prefixes.contains(dir)) {
	prefixes.append(dir);
	dircache.clear();
    }
}

bool KStandardDirs::addResourceType( const QString& type,
		      const QString& relativename )
{
    if (relativename.isNull())
       return false;

    QStringList *rels = relatives.find(type);
    if (!rels) {
	rels = new QStringList();
	relatives.insert(type, rels);
    }
    QString copy = relativename;
    if (copy.at(copy.length() - 1) != '/')
      copy += '/';
    if (!rels->contains(copy)) {
	rels->append(copy);
	dircache.remove(type); // clean the cache
	return true;
    }
    return false;
}

bool KStandardDirs::addResourceDir( const QString& type,
		     const QString& absdir, bool)
{
    QStringList *paths = absolutes.find(type);
    if (!paths) {
	paths = new QStringList();
	absolutes.insert(type, paths);
    }
    if (!paths->contains(absdir)) {
	paths->append(absdir);
	dircache.remove(type); // clean the cache
	return true;
    }
    return false;
}

QString KStandardDirs::findResource( const QString& type,
		      const QString& filename ) const
{
    if (filename.at(0) == '/')
	return filename; // absolute dirs are absolute dirs, right? :-/

    QString dir = findResourceDir(type, filename);
    if (dir.isNull())
	return dir;
    else return dir + filename;
}

QStringList KStandardDirs::findDirs( const QString& type, 
                                     const QString& reldir ) const
{
    QStringList list;

    QStringList candidates = getResourceDirs(type);
    QDir testdir;

    for (QStringList::ConstIterator it = candidates.begin();
         it != candidates.end(); it++) {
        testdir.setPath(*it + reldir);
        if (testdir.exists())
            list.append(testdir.absPath() + '/');
    }
    return list;
}

QString KStandardDirs::findResourceDir( const QString& type,
					const QString& filename) const
{
    if (filename.isEmpty()) {
        warning("filename for type %s in KStandardDirs::findResourceDir is not supposed to be empty!!", type.ascii());
	return QString::null;
    }

    QStringList candidates = getResourceDirs(type);
    QFileInfo testfile;
    for (QStringList::ConstIterator it = candidates.begin();
	 it != candidates.end(); it++) {
	testfile.setFile(*it + filename);
	if (testfile.isFile())
	    return *it;
    }
    
    debug("KStdDirs::findResDir(): can't find \"%s\" in type \"%s\".",
          filename.ascii(), type.ascii());
          
    return QString::null;
}

QStringList KStandardDirs::findAllResources( const QString& type, 
					     const QString& filter, 
					     bool recursive) const
{
    assert(!recursive);
    
    if (filter.at(0) == '/') // absolute paths we return
      return filter;

    QStringList list;

    QStringList candidates = getResourceDirs(type);
    QDir testdir;

    QStringList entries;
    for (QStringList::ConstIterator it = candidates.begin();
	 it != candidates.end(); it++) {
	testdir.setPath(*it);
	entries = testdir.entryList( QDir::Files | QDir::Readable, QDir::Unsorted);
	for (QStringList::ConstIterator it2 = entries.begin();
	     it2 != entries.end(); it2++) {
	  if (filter.isEmpty() || (*it2) == filter)
	    list.append(*it + *it2);
	}
    }
    return list;
}

QStringList KStandardDirs::getResourceDirs(const QString& type) const
{
    QStringList *candidates = dircache.find(type);
    if (!candidates) { // filling cache
        QDir testdir;
	candidates = new QStringList();
	QStringList *dirs = absolutes.find(type);
	if (dirs)
	    for (QStringList::ConstIterator it = dirs->begin();
		 it != dirs->end(); it++) {
	      testdir.setPath(*it);
	      if (testdir.exists() && !candidates->contains(*it)) {
	 // debug("adding abs %s for type %s", (*it).ascii(), type.ascii());
		candidates->append(*it);
	      }
	    }
	dirs = relatives.find(type);
	if (dirs)
	    for (QStringList::ConstIterator pit = prefixes.fromLast();
		 pit != prefixes.end(); pit--)
	      for (QStringList::ConstIterator it = dirs->begin();
		   it != dirs->end(); it++)
		  {
		    QString path = *pit + *it;
		    testdir.setPath(path);
		    if (testdir.exists() && !candidates->contains(path)) {
	// debug("adding mix %s for type %s", path.ascii(), type.ascii());
		      candidates->append(path);
		    }
		}
	dircache.insert(type, candidates);
    }
    return *candidates;
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

QString KStandardDirs::kde_data_relative()
{
    return "share/apps/";
}

QString KStandardDirs::getSaveLocation(const QString& type,
				       const QString& suffix, 
				       bool create) const
{
    QString local = QDir::homeDirPath() + "/.kde/";
    int length = local.length();

    QStringList candidates = getResourceDirs(type);
    for (QStringList::ConstIterator it = candidates.begin();
         it != candidates.end(); it++) 
    {
	if ((*it).left(length) == local) {
	    // Check for existance of typed directory + suffix
	    QString fullPath = *it + suffix;
	    if (access(fullPath.data(), F_OK) != 0) {
                if(!create) {
                    debug("save location %s doesn't exist", fullPath.ascii());
                    return local;
                }
                if(!makeDir(fullPath)) {
                    debug("failed to create %s", fullPath.ascii());
                    return local;
                }
            }
            return fullPath;
	}
    }
    debug("couldn't find save location for type %s", type.ascii());
    return local;
}

bool KStandardDirs::makeDir(const QString& dir)
{
    // we want an absolute path
    if (dir.at(0) != '/')
        return false;

    QString target = dir;
    uint len = target.length();

    // append trailing slash if missing
    if (dir.at(len - 1) != '/')
        target += '/';
    
    QString base("/");
    uint i = 1;

    while( i < len )
    {
        int pos = target.find('/', i);
        base += target.mid(i, pos - i + 1);
        // bail out if we encountered a problem
        if (access(base.ascii(), F_OK) != 0 && mkdir(base.ascii(), 0777) != 0)
            return false;
        i = pos + 1;
    }
    return true;
}

void KStandardDirs::addKDEDefaults(const QString &appName) {

    QString kdedir = getenv("KDEDIR");
    if (kdedir.isEmpty())
          kdedir = KDEDIR;

    addPrefix(kdedir);
    addPrefix(QDir::homeDirPath() + "/.kde/");

    addResourceType("html", "share/doc/HTML/");
    addResourceType("icon", "share/icons/");
    addResourceType("mini", "share/icons/mini/");
    addResourceType("apps", "share/applnk/");
    addResourceType("sound", "share/sounds/");
    addResourceType("data", kde_data_relative());
    if(!appName.isNull())
        addResourceType("appdata", kde_data_relative() + appName + "/");
    addResourceType("locale", "share/locale/");
    addResourceType("services", "share/services/");
    addResourceType("servicetypes", "share/servicetypes/");
    addResourceType("mime", "share/mimelnk/");
    addResourceType("cgi", "cgi-bin/");
    addResourceType("config", "share/config/");
    addResourceType("toolbar", "share/toolbar/");
    addResourceType("wallpaper", "share/wallpapers/");
    addResourceType("exe", "bin/");
    addResourceType("lib", "lib/");
}

QString locate( const QString& type,
		const QString& filename ) {
  return KGlobal::dirs()->findResource(type, filename);
}
