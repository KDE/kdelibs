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
#include <kconfig.h>

static const char* types[] = {"html", "icon", "mini", "apps", "sound",
			      "data", "locale", "services", "mime",
			      "servicetypes", "cgi", "config", "exe",
			      "toolbar", "wallpaper", "lib", 0};

static int tokenize( QStringList& token, const QString& str,
		const QString& delim );

KStandardDirs::KStandardDirs( const QString& ) : addedCustoms(false)
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
	prefixes.prepend(dir);
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

    enum { Empty, PathName, RegExp } filterType;
    if (filter.isEmpty())
	filterType = Empty;
    else {
	// TODO: RegExp
	filterType = PathName;
    }

    QStringList entries;
    for (QStringList::ConstIterator it = candidates.begin();
	 it != candidates.end(); it++) {
	testdir.setPath(*it);
	entries = testdir.entryList( QDir::Files | QDir::Readable, QDir::Unsorted);
	switch (filterType) {
	case Empty:
	    for (QStringList::ConstIterator it2 = entries.begin();
		 it2 != entries.end(); it2++)
		list.append(*it + *it2);
	    break;
	case PathName:
	    if (!access(*it + filter, F_OK|R_OK))
		list.append(*it + filter);
	    break;
	default:
	    break;
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
	int len = str.length();
	QString token = "";
	
	for( int index = 0; index < len; index++) {
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

QString KStandardDirs::kde_default(const QString& type) {
    if (type == "data")
	return "share/apps/";
    if (type == "html")
	return "share/doc/HTML/";
    if (type == "icon")
	return "share/icons/";
    if (type == "mini")
	return "share/icons/mini/";
    if (type == "config")
	return "share/config/";
    if (type == "toolbar")
	return "share/toolbar/";
    if (type == "apps")
	return "share/applnk/";
    if (type == "sound")
	return "share/sounds/";
    if (type == "locale")
	return "share/locale/";
    if (type == "services")
	return "share/services/";
    if (type == "servicetypes")
	return "share/servicetypes/";
    if (type == "mime")
	return "share/mimelnk/";
    if (type == "cgi")
	return "cgi-bin/";
    if (type == "wallpaper")
	return "share/wallpapers/";
    if (type == "exe")
	return "bin/";
    if (type == "lib")
	return "lib/";
    fatal("unknown resource type %s", type.ascii());
    return QString::null;
}

QString KStandardDirs::getSaveLocation(const QString& type,
				       const QString& suffix, 
				       bool create) const
{
    QStringList *dirs = relatives.find(type);
    if (!dirs)
	fatal("there are no relative suffixes for type %s registered", type.ascii());

    struct stat st;
    QString local = localkdedir();
   
    // Check for existance of typed directory + suffix
    QString fullPath = local + dirs->last() + suffix;
    if (stat(fullPath.ascii(), &st) != 0 || !(S_ISDIR(st.st_mode))) {
	if(!create) {
	    debug("save location %s doesn't exist", fullPath.ascii());
	    return local;
	}
	if(!makeDir(fullPath, 0700)) {
	    debug("failed to create %s", fullPath.ascii());
	    return local;
	}
    }
    return fullPath;

    // I can't think of a case where this happens  
    debug("couldn't find save location for type %s", type.ascii());
    return local;
}

bool KStandardDirs::makeDir(const QString& dir, int mode)
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
        struct stat st;
        int pos = target.find('/', i);
        base += target.mid(i, pos - i + 1);
        // bail out if we encountered a problem
        if (stat(base.ascii(), &st) != 0)
        {
           // Directory does not exist....
	  if ( mkdir(base.ascii(), (mode_t) mode) != 0) {
	    perror("trying to create local folder");
	    return false; // Couldn't create it :-(
	  }
        }
        i = pos + 1;
    }
    return true;
}

void KStandardDirs::addKDEDefaults() 
{
    QStringList kdedirList;
    
    const char *kdedirs = getenv("KDEDIRS");
    if (kdedirs)
	tokenize(kdedirList, kdedirs, ":");
    else {    
	QString kdedir = getenv("KDEDIR");
	if (kdedir.isEmpty())
	    kdedir = KDEDIR;
	kdedirList.append(kdedir);
    }

    addPrefix(localkdedir());
    for (QStringList::ConstIterator it = kdedirList.begin();
	 it != kdedirList.end(); it++)
	addPrefix(*it);


    uint index = 0;
    while (types[index] != 0) {
	addResourceType(types[index], kde_default(types[index]));
	index++;
    }
    
}

bool KStandardDirs::addCustomized(KConfig *config)
{
    if (!addedCustoms)
        return false;

    uint configdirs = getResourceDirs("config").count();
    KConfigGroupSaver(config, "Directories");
    QStringList list;
    QStringList::ConstIterator it;
    list = config->readListEntry("prefixes");
    for (it = list.begin(); it != list.end(); it++)
	addPrefix(*it);
    QMap<QString, QString> entries = config->entryMap("Directories");
    QMap<QString, QString>::ConstIterator it2;
    for (it2 = entries.begin(); it2 != entries.end(); it2++) {
	QString key = it2.key();
	if (key.left(4) == "dir_") {
	    addResourceDir(key.mid(5, key.length()), it2.data());
	    debug("adding custom dir %s", it2.data().ascii());
	}
    }
    addedCustoms = true;
    return (getResourceDirs("config").count() != configdirs);
}

QString KStandardDirs::localkdedir() const
{
    return QDir::homeDirPath() + "/.kde/";
}

QString locate( const QString& type,
		const QString& filename ) {
    return KGlobal::dirs()->findResource(type, filename);
}

QString locateLocal( const QString& type,
	             QString filename ) 
{
    QString dir, file;
    int slash = filename.findRev('/')+1;
    if (!slash)
	return KGlobal::dirs()->getSaveLocation(type)+filename;
    dir = filename.left(slash);
    file = filename.mid(slash);
    return KGlobal::dirs()->getSaveLocation(type, dir)+file;
}

