/*
 * Author: Stephan Kulow <coolo@kde.org> and Sirtaj Singh Kang <taj@kde.org>
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
#include <sys/types.h>
#include <dirent.h>

#include <qregexp.h>
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

KStandardDirs::KStandardDirs( ) : addedCustoms(false)
{
    dircache.setAutoDelete(true);
    relatives.setAutoDelete(true);
    absolutes.setAutoDelete(true);
}

KStandardDirs::~KStandardDirs()
{
}

void KStandardDirs::addPrefix( QString dir )
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
	rels->prepend(copy);
	dircache.remove(type); // clean the cache
	return true;
    }
    return false;
}

bool KStandardDirs::addResourceDir( const QString& type,
				    const QString& absdir)
{
    QStringList *paths = absolutes.find(type);
    if (!paths) {
	paths = new QStringList();
	absolutes.insert(type, paths);
    }
    QString copy = absdir;
    if (copy.at(copy.length() - 1) != '/')
      copy += '/';
    if (!paths->contains(copy)) {
	paths->append(copy);
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
#ifndef NDEBUG
    if (filename.isEmpty()) {
      warning("filename for type %s in KStandardDirs::findResourceDir is not supposed to be empty!!", type.ascii());
      return QString::null;
    }
#endif

    QStringList candidates = getResourceDirs(type);
    QString fullPath;
    struct stat buff;
    for (QStringList::ConstIterator it = candidates.begin();
	 it != candidates.end(); it++) 
    {
      fullPath = *it + filename;
      if (access(fullPath.ascii(), R_OK) == 0 && stat( fullPath.ascii(), &buff ) == 0)
	if ( S_ISREG( buff.st_mode ))
	  return *it;
    }

#ifndef NDEBUG
    if(type != "locale")
      debug("KStdDirs::findResDir(): can't find \"%s\" in type \"%s\".",
            filename.ascii(), type.ascii());
#endif    
          
    return QString::null;
}

static void lookupDirectory(const QString& path, const QString &relPart,
			    const QRegExp &regexp, 
			    QStringList& list, 
			    QStringList& relList, 
			    bool recursive, bool uniq)
{
  DIR *dp = opendir( QFile::encodeName(path));
  if (!dp)
    return;

  assert(path.at(path.length() - 1) == '/');

  struct dirent *ep;
  struct stat buff;

  while( ( ep = readdir( dp ) ) != 0L )
    {
      QString fn( QFile::decodeName(ep->d_name));
      if (fn == "." || fn == ".." || fn.at(fn.length() - 1) == '~')
	continue;

      if (!recursive && regexp.match(fn))
	continue; // No match

      QString rfn = relPart + fn;
      fn = path + fn;
      if ( stat( fn.ascii(), &buff ) != 0 ) {
	QString tmp = QString("Error statting %1:").arg( fn );
	perror(tmp.ascii());
	continue; // Couldn't stat (Why not?)
      }
      if ( recursive ) {
	if ( S_ISDIR( buff.st_mode ))
	  lookupDirectory(fn + '/', rfn + '/', regexp, list, relList, recursive, uniq);
	if (regexp.match(fn))
	  continue; // No match
      }
      if ( S_ISREG( buff.st_mode))
      {
        if (!uniq || !relList.contains(rfn))
        {
	    list.append( fn );
	    relList.append( rfn );
        }
      }	
    }
  closedir( dp );
}

static void lookupPrefix(const QString& prefix, const QString& relpath,
                         const QString& relPart,
			 const QRegExp &regexp, 
			 QStringList& list, 
			 QStringList& relList, 
			 bool recursive, bool uniq)
{
    if (relpath.isNull())
	return lookupDirectory(prefix, relPart, regexp, list, relList, recursive, uniq);
    
    QString path;
    QString rest;
    
    if (relpath.length())
    {
       int slash = relpath.find('/');
       if (slash < 0)
	   rest = relpath.left(relpath.length() - 1);
       else {
	   path = relpath.left(slash + 1);
	   rest = relpath.mid(slash + 1);
       }
    }

    QRegExp pathExp(path, true, true);
    DIR *dp = opendir( QFile::encodeName(prefix) );
    if (!dp) {
	warning("%s doesn't exist!", prefix.ascii());
	return;
    }

    assert(prefix.at(prefix.length() - 1) == '/');

    struct dirent *ep;
    struct stat buff;

    while( ( ep = readdir( dp ) ) != 0L )
    {
	QString fn( QFile::decodeName(ep->d_name));
	if (fn == "." || fn == ".." || fn.at(fn.length() - 1) == '~')
	    continue;

	if (!pathExp.match(fn))
	    continue; // No match
	QString rfn = relPart+fn;
	fn = prefix + fn;
	if ( stat( fn.ascii(), &buff ) != 0 ) {
	    QString tmp = QString("Error statting %1:").arg( fn );
	    perror(tmp.ascii());
	    continue; // Couldn't stat (Why not?)
	}
	if ( S_ISDIR( buff.st_mode ))
	    lookupPrefix(fn + '/', rest, rfn + '/', regexp, list, relList, recursive, uniq);
    }
    
    closedir( dp );
}

QStringList
KStandardDirs::findAllResources( const QString& type, 
			         const QString& filter, 
				 bool recursive,
			         bool uniq,
                                 QStringList &relList) const
{    
debug("findAllResources( %s, %s )", type.ascii(), filter.ascii());

    QStringList list;
    if (filter.at(0) == '/') // absolute paths we return
    {
        list.append( filter);
        relList.append( "" );
	return list;
    }

    QString filterPath;
    QString filterFile;
    
    if (filter.length())
    {
       int slash = filter.findRev('/');
       if (slash < 0)
	   filterFile = filter;
       else {
	   filterPath = filter.left(slash + 1);
	   filterFile = filter.mid(slash + 1);
       }
    }
    
    QStringList candidates = getResourceDirs(type);
    if (filterFile.isEmpty())
	filterFile = "*";
    QRegExp regExp(filterFile, true, true);
    
    for (QStringList::ConstIterator it = candidates.begin();
	 it != candidates.end(); it++) 
      lookupPrefix(*it, filterPath, "", regExp, list, relList, recursive, uniq);

debug("List:");
    for (QStringList::ConstIterator it = list.begin();
	 it != list.end(); it++)
      debug("%s", (*it).ascii());
debug("relList:");
    for (QStringList::ConstIterator it = relList.begin();
	 it != relList.end(); it++)
      debug("%s", (*it).ascii());
debug("Done");

    return list;
}

QStringList
KStandardDirs::findAllResources( const QString& type, 
			         const QString& filter, 
				 bool recursive,
			         bool uniq) const
{
   QStringList relList;
   return findAllResources(type, filter, recursive, uniq, relList);
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
		if (testdir.exists() && !candidates->contains(*it))
		    candidates->append(*it);
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
    
    for( int index = 0; index < len; index++) 
    {
	if ( delim.find( str[ index ] ) >= 0 ) 
	{
	    tokens.append( token );
	    token = "";
	}
	else 
	{
	    token += str[ index ];
	}
    }
    if ( token.length() > 0 ) 
    {
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
    if (addedCustoms) // there are already customized entries
        return false; // we just quite and hope they are the right ones

    // save the numbers of config directories. If this changes,
    // we will return true to give KConfig a chance to reparse
    uint configdirs = getResourceDirs("config").count();

    // reading the prefixes in
    KConfigGroupSaver(config, "Directories");
    QStringList list;
    QStringList::ConstIterator it;
    list = config->readListEntry("prefixes");
    for (it = list.begin(); it != list.end(); it++)
	addPrefix(*it);

    // iterating over all entries in the group Directories 
    // to find entries that start with dir_$type
    QMap<QString, QString> entries = config->entryMap("Directories");
    QMap<QString, QString>::ConstIterator it2;
    for (it2 = entries.begin(); it2 != entries.end(); it2++) 
    {
	QString key = it2.key();
	if (key.left(4) == "dir_") 
	{
	    addResourceDir(key.mid(5, key.length()), it2.data());
	    debug("adding custom dir %s", it2.data().ascii());
	}
    }

    // save it for future calls - that will return
    addedCustoms = true;

    // return true if the number of config dirs changed
    return (getResourceDirs("config").count() != configdirs);
}

QString KStandardDirs::localkdedir() const
{
    // we don't want to hardcode paths, but we shouldn't
    // make it too flexible, so that applications find the
    // files they saved
    return QDir::homeDirPath() + "/.kde/";
}

// just to make code more readable without macros
QString locate( const QString& type,
		const QString& filename ) 
{
    return KGlobal::dirs()->findResource(type, filename);
}

QString locateLocal( const QString& type,
	             const QString& filename ) 
{
    // try to find slashes. If there are some, we have to
    // create the subdir first
    int slash = filename.findRev('/')+1;
    if (!slash) // only one filename
	return KGlobal::dirs()->getSaveLocation(type) + filename;

    // split path from filename
    QString dir = filename.left(slash);
    QString file = filename.mid(slash);
    return KGlobal::dirs()->getSaveLocation(type, dir) + file;
}

