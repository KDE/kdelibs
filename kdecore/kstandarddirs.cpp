/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kang <taj@kde.org>
   Copyright (C) 1999 Stephan Kulow <coolo@kde.org>
   Copyright (C) 1999 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/*
 * Author: Stephan Kulow <coolo@kde.org> and Sirtaj Singh Kang <taj@kde.org>
 * Version:	$Id$
 * Generated:	Thu Mar  5 16:05:28 EST 1998
 */

#include "config.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#include <qregexp.h>
#include <qasciidict.h>
#include <qdict.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qstringlist.h>

#include "kstandarddirs.h"
#include "kconfig.h"
#include "kdebug.h"
#include "kinstance.h"
#include "kshell.h"
#include "ksimpleconfig.h"
#include <sys/param.h>
#include <unistd.h>

template class QDict<QStringList>;

class KStandardDirs::KStandardDirsPrivate
{
public:
   KStandardDirsPrivate()
    : restrictionsActive(false),
      dataRestrictionActive(false)
   { }

   bool restrictionsActive;
   bool dataRestrictionActive;
   QAsciiDict<bool> restrictions;
   QStringList xdgdata_prefixes;
   QStringList xdgconf_prefixes;
};

static const char* const types[] = {"html", "icon", "apps", "sound",
			      "data", "locale", "services", "mime",
			      "servicetypes", "config", "exe",
			      "wallpaper", "lib", "pixmap", "templates",
			      "module", "qtplugins",
			      "xdgdata-apps", "xdgdata-dirs", "xdgconf-menu",
                              "kcfg", 0 };

static int tokenize( QStringList& token, const QString& str,
		const QString& delim );

KStandardDirs::KStandardDirs( ) : addedCustoms(false)
{
    d = new KStandardDirsPrivate;
    dircache.setAutoDelete(true);
    relatives.setAutoDelete(true);
    absolutes.setAutoDelete(true);
    savelocations.setAutoDelete(true);
    addKDEDefaults();
}

KStandardDirs::~KStandardDirs()
{
    delete d;
}

bool KStandardDirs::isRestrictedResource(const char *type, const QString& relPath) const
{
   if (!d || !d->restrictionsActive)
      return false;

   if (d->restrictions[type])
      return true;

   if (strcmp(type, "data")==0)
   {
      applyDataRestrictions(relPath);
      if (d->dataRestrictionActive)
      {
         d->dataRestrictionActive = false;
         return true;
      }
   }
   return false;
}

void KStandardDirs::applyDataRestrictions(const QString &relPath) const
{
   QString key;
   int i = relPath.find('/');
   if (i != -1)
      key = "data_"+relPath.left(i);
   else
      key = "data_"+relPath;

   if (d && d->restrictions[key.latin1()])
      d->dataRestrictionActive = true;
}


QStringList KStandardDirs::allTypes() const
{
    QStringList list;
    for (int i = 0; types[i] != 0; ++i)
        list.append(QString::fromLatin1(types[i]));
    return list;
}

static void priorityAdd(QStringList &prefixes, const QString& dir, bool priority)
{
    if (priority && !prefixes.isEmpty())
    {
        // Add in front but behind $KDEHOME
        QStringList::iterator it = prefixes.begin();
        it++;
        prefixes.insert(it, 1, dir);
    }
    else
    {
        prefixes.append(dir);
    }
}

void KStandardDirs::addPrefix( const QString& _dir )
{
    addPrefix(_dir, false);
}

void KStandardDirs::addPrefix( const QString& _dir, bool priority )
{
    if (_dir.isEmpty())
	return;

    QString dir = _dir;
    if (dir.at(dir.length() - 1) != '/')
	dir += '/';

    if (!prefixes.contains(dir)) {
        priorityAdd(prefixes, dir, priority);
	dircache.clear();
    }
}

void KStandardDirs::addXdgConfigPrefix( const QString& _dir )
{
    addXdgConfigPrefix(_dir, false);
}

void KStandardDirs::addXdgConfigPrefix( const QString& _dir, bool priority )
{
    if (_dir.isEmpty())
	return;

    QString dir = _dir;
    if (dir.at(dir.length() - 1) != '/')
	dir += '/';

    if (!d->xdgconf_prefixes.contains(dir)) {
        priorityAdd(d->xdgconf_prefixes, dir, priority);
	dircache.clear();
    }
}

void KStandardDirs::addXdgDataPrefix( const QString& _dir )
{
    addXdgDataPrefix(_dir, false);
}

void KStandardDirs::addXdgDataPrefix( const QString& _dir, bool priority )
{
    if (_dir.isEmpty())
	return;

    QString dir = _dir;
    if (dir.at(dir.length() - 1) != '/')
	dir += '/';

    if (!d->xdgdata_prefixes.contains(dir)) {
	priorityAdd(d->xdgdata_prefixes, dir, priority);
	dircache.clear();
    }
}

QString KStandardDirs::kfsstnd_prefixes()
{
   return prefixes.join(":");
}

bool KStandardDirs::addResourceType( const char *type,
				     const QString& relativename )
{
    return addResourceType(type, relativename, true);
}
bool KStandardDirs::addResourceType( const char *type,
				     const QString& relativename,
				     bool priority )
{
    if (relativename.isEmpty())
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
        if (priority)
	    rels->prepend(copy);
	else
	    rels->append(copy);
	dircache.remove(type); // clean the cache
	return true;
    }
    return false;
}

bool KStandardDirs::addResourceDir( const char *type,
				    const QString& absdir)
{
    // KDE4: change priority to bring in line with addResourceType
    return addResourceDir(type, absdir, false);
}

bool KStandardDirs::addResourceDir( const char *type,
				    const QString& absdir,
				    bool priority)
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
        if (priority)
            paths->prepend(copy);
        else
	    paths->append(copy);
	dircache.remove(type); // clean the cache
	return true;
    }
    return false;
}

QString KStandardDirs::findResource( const char *type,
				     const QString& filename ) const
{
    if (filename.at(0) == '/')
	return filename; // absolute dirs are absolute dirs, right? :-/

#if 0
kdDebug() << "Find resource: " << type << endl;
for (QStringList::ConstIterator pit = prefixes.begin();
     pit != prefixes.end();
     pit++)
{
  kdDebug() << "Prefix: " << *pit << endl;
}
#endif

    QString dir = findResourceDir(type, filename);
    if (dir.isEmpty())
	return dir;
    else return dir + filename;
}

static Q_UINT32 updateHash(const QString &file, Q_UINT32 hash)
{
    QCString cFile = QFile::encodeName(file);
    struct stat buff;
    if ((access(cFile, R_OK) == 0) &&
        (stat( cFile, &buff ) == 0) &&
        (S_ISREG( buff.st_mode )))
    {
       hash = hash + (Q_UINT32) buff.st_ctime;
    }
    return hash;
}

Q_UINT32 KStandardDirs::calcResourceHash( const char *type,
			      const QString& filename, bool deep) const
{
    Q_UINT32 hash = 0;

    if (filename.at(0) == '/')
    {
        // absolute dirs are absolute dirs, right? :-/
	return updateHash(filename, hash);
    }
    if (d && d->restrictionsActive && (strcmp(type, "data")==0))
       applyDataRestrictions(filename);
    QStringList candidates = resourceDirs(type);
    QString fullPath;

    for (QStringList::ConstIterator it = candidates.begin();
	 it != candidates.end(); it++)
    {
        hash = updateHash(*it + filename, hash);
        if (!deep && hash)
           return hash;
    }
    return hash;
}


QStringList KStandardDirs::findDirs( const char *type,
                                     const QString& reldir ) const
{
    QDir testdir;
    QStringList list;
    if (reldir.startsWith("/"))
    {
        testdir.setPath(reldir);
        if (testdir.exists())
        {
            if (reldir.endsWith("/"))
               list.append(reldir);
            else
               list.append(reldir+'/');
        }
        return list;
    }

    checkConfig();

    if (d && d->restrictionsActive && (strcmp(type, "data")==0))
       applyDataRestrictions(reldir);
    QStringList candidates = resourceDirs(type);

    for (QStringList::ConstIterator it = candidates.begin();
         it != candidates.end(); it++) {
        testdir.setPath(*it + reldir);
        if (testdir.exists())
            list.append(testdir.absPath() + '/');
    }

    return list;
}

QString KStandardDirs::findResourceDir( const char *type,
					const QString& filename) const
{
#ifndef NDEBUG
    if (filename.isEmpty()) {
      kdWarning() << "filename for type " << type << " in KStandardDirs::findResourceDir is not supposed to be empty!!" << endl;
      return QString::null;
    }
#endif

    if (d && d->restrictionsActive && (strcmp(type, "data")==0))
       applyDataRestrictions(filename);
    QStringList candidates = resourceDirs(type);
    QString fullPath;

    for (QStringList::ConstIterator it = candidates.begin();
	 it != candidates.end(); it++)
      if (exists(*it + filename))
	return *it;

#ifndef NDEBUG
    if(false && type != "locale")
      kdDebug() << "KStdDirs::findResDir(): can't find \"" << filename << "\" in type \"" << type << "\"." << endl;
#endif

    return QString::null;
}

bool KStandardDirs::exists(const QString &fullPath)
{
    struct stat buff;
    if (access(QFile::encodeName(fullPath), R_OK) == 0 && stat( QFile::encodeName(fullPath), &buff ) == 0)
	if (fullPath.at(fullPath.length() - 1) != '/') {
	    if (S_ISREG( buff.st_mode ))
		return true;
	} else
	    if (S_ISDIR( buff.st_mode ))
		return true;
    return false;
}

static void lookupDirectory(const QString& path, const QString &relPart,
			    const QRegExp &regexp,
			    QStringList& list,
			    QStringList& relList,
			    bool recursive, bool unique)
{
  QString pattern = regexp.pattern();
  if (recursive || pattern.contains('?') || pattern.contains('*'))
  {
    // We look for a set of files.
    DIR *dp = opendir( QFile::encodeName(path));
    if (!dp)
      return;

    assert(path.at(path.length() - 1) == '/');

    struct dirent *ep;
    struct stat buff;

    QString _dot(".");
    QString _dotdot("..");

    while( ( ep = readdir( dp ) ) != 0L )
    {
      QString fn( QFile::decodeName(ep->d_name));
      if (fn == _dot || fn == _dotdot || fn.at(fn.length() - 1).latin1() == '~')
	continue;

      if (!recursive && !regexp.exactMatch(fn))
	continue; // No match

      QString pathfn = path + fn;
      if ( stat( QFile::encodeName(pathfn), &buff ) != 0 ) {
	kdDebug() << "Error stat'ing " << pathfn << " : " << perror << endl;
	continue; // Couldn't stat (e.g. no read permissions)
      }
      if ( recursive ) {
	if ( S_ISDIR( buff.st_mode )) {
	  lookupDirectory(pathfn + '/', relPart + fn + '/', regexp, list, relList, recursive, unique);
	}
        if (!regexp.exactMatch(fn))
	  continue; // No match
      }
      if ( S_ISREG( buff.st_mode))
      {
        if (!unique || !relList.contains(relPart + fn))
        {
	    list.append( pathfn );
	    relList.append( relPart + fn );
        }
      }
    }
    closedir( dp );
  }
  else
  {
     // We look for a single file.
     QString fn = pattern;
     QString pathfn = path + fn;
     struct stat buff;
     if ( stat( QFile::encodeName(pathfn), &buff ) != 0 )
        return; // File not found
     if ( S_ISREG( buff.st_mode))
     {
       if (!unique || !relList.contains(relPart + fn))
       {
         list.append( pathfn );
         relList.append( relPart + fn );
       }
     }
  }
}

static void lookupPrefix(const QString& prefix, const QString& relpath,
                         const QString& relPart,
			 const QRegExp &regexp,
			 QStringList& list,
			 QStringList& relList,
			 bool recursive, bool unique)
{
    if (relpath.isEmpty()) {
       lookupDirectory(prefix, relPart, regexp, list,
		       relList, recursive, unique);
       return;
    }
    QString path;
    QString rest;

    if (relpath.length())
    {
       int slash = relpath.find('/');
       if (slash < 0)
	   rest = relpath.left(relpath.length() - 1);
       else {
	   path = relpath.left(slash);
	   rest = relpath.mid(slash + 1);
       }
    }

    assert(prefix.at(prefix.length() - 1) == '/');

    struct stat buff;

    if (path.contains('*') || path.contains('?')) {

	QRegExp pathExp(path, true, true);
	DIR *dp = opendir( QFile::encodeName(prefix) );
	if (!dp) {
	    return;
	}

	struct dirent *ep;

        QString _dot(".");
        QString _dotdot("..");

	while( ( ep = readdir( dp ) ) != 0L )
	    {
		QString fn( QFile::decodeName(ep->d_name));
		if (fn == _dot || fn == _dotdot || fn.at(fn.length() - 1) == '~')
		    continue;

		if ( !pathExp.exactMatch(fn) )
		    continue; // No match
		QString rfn = relPart+fn;
		fn = prefix + fn;
		if ( stat( QFile::encodeName(fn), &buff ) != 0 ) {
		    kdDebug() << "Error statting " << fn << " : " << perror << endl;
		    continue; // Couldn't stat (e.g. no permissions)
		}
		if ( S_ISDIR( buff.st_mode ))
		    lookupPrefix(fn + '/', rest, rfn + '/', regexp, list, relList, recursive, unique);
	    }

	closedir( dp );
    } else {
        // Don't stat, if the dir doesn't exist we will find out
        // when we try to open it.
        lookupPrefix(prefix + path + '/', rest,
                     relPart + path + '/', regexp, list,
                     relList, recursive, unique);
    }
}

QStringList
KStandardDirs::findAllResources( const char *type,
			         const QString& filter,
				 bool recursive,
			         bool unique,
                                 QStringList &relList) const
{
    QStringList list;
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

    checkConfig();

    QStringList candidates;
    if (filterPath.startsWith("/")) // absolute path
    {
        filterPath = filterPath.mid(1);
        candidates << "/";
    }
    else
    {
        if (d && d->restrictionsActive && (strcmp(type, "data")==0))
            applyDataRestrictions(filter);
        candidates = resourceDirs(type);
    }
    if (filterFile.isEmpty())
	filterFile = "*";

    QRegExp regExp(filterFile, true, true);

    for (QStringList::ConstIterator it = candidates.begin();
         it != candidates.end(); it++)
    {
        lookupPrefix(*it, filterPath, "", regExp, list,
                     relList, recursive, unique);
    }

    return list;
}

QStringList
KStandardDirs::findAllResources( const char *type,
			         const QString& filter,
				 bool recursive,
			         bool unique) const
{
    QStringList relList;
    return findAllResources(type, filter, recursive, unique, relList);
}

QString
KStandardDirs::realPath(const QString &dirname)
{
    char realpath_buffer[MAXPATHLEN + 1];
    memset(realpath_buffer, 0, MAXPATHLEN + 1);

    /* If the path contains symlinks, get the real name */
    if (realpath( QFile::encodeName(dirname).data(), realpath_buffer) != 0) {
        // succes, use result from realpath
        int len = strlen(realpath_buffer);
        realpath_buffer[len] = '/';
        realpath_buffer[len+1] = 0;
        return QFile::decodeName(realpath_buffer);
    }

    return dirname;
}

void KStandardDirs::createSpecialResource(const char *type)
{
   char hostname[256];
   hostname[0] = 0;
   gethostname(hostname, 255);
   QString dir = QString("%1%2-%3").arg(localkdedir()).arg(type).arg(hostname);
   char link[1024];
   link[1023] = 0;
   int result = readlink(QFile::encodeName(dir).data(), link, 1023);
   if ((result == -1) && (errno == ENOENT))
   {
      QString srv = findExe(QString::fromLatin1("lnusertemp"), KDEDIR+QString::fromLatin1("/bin"));
      if (srv.isEmpty())
         srv = findExe(QString::fromLatin1("lnusertemp"));
      if (!srv.isEmpty())
      {
         system(QFile::encodeName(srv)+" "+type);
         result = readlink(QFile::encodeName(dir).data(), link, 1023);
      }
   }
   if (result > 0)
   {
      link[result] = 0;
      if (link[0] == '/')
         dir = QFile::decodeName(link);
      else
         dir = QDir::cleanDirPath(dir+QFile::decodeName(link));
   }
   addResourceDir(type, dir+'/');
}

QStringList KStandardDirs::resourceDirs(const char *type) const
{
    QStringList *candidates = dircache.find(type);

    if (!candidates) { // filling cache
        if (strcmp(type, "socket") == 0)
           const_cast<KStandardDirs *>(this)->createSpecialResource(type);
        else if (strcmp(type, "tmp") == 0)
           const_cast<KStandardDirs *>(this)->createSpecialResource(type);
        else if (strcmp(type, "cache") == 0)
           const_cast<KStandardDirs *>(this)->createSpecialResource(type);

        QDir testdir;

        candidates = new QStringList();
        QStringList *dirs;

        bool restrictionActive = false;
        if (d && d->restrictionsActive)
        {
           if (d->dataRestrictionActive)
              restrictionActive = true;
           else if (d->restrictions["all"])
              restrictionActive = true;
           else if (d->restrictions[type])
              restrictionActive = true;
           d->dataRestrictionActive = false; // Reset
        }

        dirs = relatives.find(type);
        if (dirs)
        {
            bool local = true;
            const QStringList *prefixList = 0;
            if (strncmp(type, "xdgdata-", 8) == 0)
                prefixList = &(d->xdgdata_prefixes);
            else if (strncmp(type, "xdgconf-", 8) == 0)
                prefixList = &(d->xdgconf_prefixes);
            else
                prefixList = &prefixes;

            for (QStringList::ConstIterator pit = prefixList->begin();
                 pit != prefixList->end();
                 pit++)
            {
                for (QStringList::ConstIterator it = dirs->begin();
                     it != dirs->end(); ++it) {
                    QString path = realPath(*pit + *it);
                    testdir.setPath(path);
                    if (local && restrictionActive)
                       continue;
                    if ((local || testdir.exists()) && !candidates->contains(path))
                        candidates->append(path);
                }
                local = false;
            }
        }
        dirs = absolutes.find(type);
        if (dirs)
            for (QStringList::ConstIterator it = dirs->begin();
                 it != dirs->end(); ++it)
            {
                testdir.setPath(*it);
                if (testdir.exists())
                {
                    QString filename = realPath(*it);
                    if (!candidates->contains(filename))
                        candidates->append(filename);
                }
            }
        dircache.insert(type, candidates);
    }

#if 0
    kdDebug() << "found dirs for resource " << type << ":" << endl;
    for (QStringList::ConstIterator pit = candidates->begin();
	 pit != candidates->end();
	 pit++)
    {
	fprintf(stderr, "%s\n", (*pit).latin1());
    }
#endif


  return *candidates;
}

QStringList KStandardDirs::systemPaths( const QString& pstr )
{
    QStringList tokens;
    QString p = pstr;

    if( p.isNull() )
    {
	p = getenv( "PATH" );
    }

    tokenize( tokens, p, ":\b" );

    QStringList exePaths;

    // split path using : or \b as delimiters
    for( unsigned i = 0; i < tokens.count(); i++ )
    {
	p = tokens[ i ];

        if ( p[ 0 ] == '~' )
        {
            int len = p.find( '/' );
            if ( len == -1 )
                len = p.length();
            if ( len == 1 )
            {
                p.replace( 0, 1, QDir::homeDirPath() );
            }
            else
            {
                QString user = p.mid( 1, len - 1 );
                struct passwd *dir = getpwnam( user.local8Bit().data() );
                if ( dir && strlen( dir->pw_dir ) )
                    p.replace( 0, len, QString::fromLocal8Bit( dir->pw_dir ) );
            }
        }

	exePaths << p;
    }

    return exePaths;
}


QString KStandardDirs::findExe( const QString& appname,
				const QString& pstr, bool ignore)
{
    QFileInfo info;

    // absolute path ?
    if (appname.startsWith(QString::fromLatin1("/")))
    {
        info.setFile( appname );
        if( info.exists() && ( ignore || info.isExecutable() )
            && info.isFile() ) {
            return appname;
        }
        return QString::null;
    }

    QString p = QString("%1/%2").arg(__KDE_BINDIR).arg(appname);
    info.setFile( p );
    if( info.exists() && ( ignore || info.isExecutable() )
         && ( info.isFile() || info.isSymLink() )  ) {
         return p;
    }

    QStringList exePaths = systemPaths( pstr );
    for (QStringList::ConstIterator it = exePaths.begin(); it != exePaths.end(); it++)
    {
	p = (*it) + "/";
	p += appname;

	// Check for executable in this tokenized path
	info.setFile( p );

	if( info.exists() && ( ignore || info.isExecutable() )
           && ( info.isFile() || info.isSymLink() )  ) {
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
    QFileInfo info;
    QString p;
    list.clear();

    QStringList exePaths = systemPaths( pstr );
    for (QStringList::ConstIterator it = exePaths.begin(); it != exePaths.end(); it++)
    {
	p = (*it) + "/";
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

QString KStandardDirs::kde_default(const char *type) {
    if (!strcmp(type, "data"))
	return "share/apps/";
    if (!strcmp(type, "html"))
	return "share/doc/HTML/";
    if (!strcmp(type, "icon"))
	return "share/icons/";
    if (!strcmp(type, "config"))
	return "share/config/";
    if (!strcmp(type, "pixmap"))
	return "share/pixmaps/";
    if (!strcmp(type, "apps"))
	return "share/applnk/";
    if (!strcmp(type, "sound"))
	return "share/sounds/";
    if (!strcmp(type, "locale"))
	return "share/locale/";
    if (!strcmp(type, "services"))
	return "share/services/";
    if (!strcmp(type, "servicetypes"))
	return "share/servicetypes/";
    if (!strcmp(type, "mime"))
	return "share/mimelnk/";
    if (!strcmp(type, "cgi"))
	return "cgi-bin/";
    if (!strcmp(type, "wallpaper"))
	return "share/wallpapers/";
    if (!strcmp(type, "templates"))
	return "share/templates/";
    if (!strcmp(type, "exe"))
	return "bin/";
    if (!strcmp(type, "lib"))
	return "lib" KDELIBSUFF "/";
    if (!strcmp(type, "module"))
	return "lib" KDELIBSUFF "/kde3/";
    if (!strcmp(type, "qtplugins"))
        return "lib" KDELIBSUFF "/kde3/plugins";
    if (!strcmp(type, "xdgdata-apps"))
        return "applications/";
    if (!strcmp(type, "xdgdata-dirs"))
        return "desktop-directories/";
    if (!strcmp(type, "xdgconf-menu"))
        return "menus/";
    if (!strcmp(type, "kcfg"))
	return "share/config.kcfg";
    qFatal("unknown resource type %s", type);
    return QString::null;
}

QString KStandardDirs::saveLocation(const char *type,
				    const QString& suffix,
				    bool create) const
{
    checkConfig();

    QString *pPath = savelocations.find(type);
    if (!pPath)
    {
       QStringList *dirs = relatives.find(type);
       if (!dirs && (
                     (strcmp(type, "socket") == 0) ||
                     (strcmp(type, "tmp") == 0) ||
                     (strcmp(type, "cache") == 0) ))
       {
          (void) resourceDirs(type); // Generate socket|tmp|cache resource.
          dirs = relatives.find(type); // Search again.
       }
       if (dirs)
       {
          // Check for existence of typed directory + suffix
          if (strncmp(type, "xdgdata-", 8) == 0)
             pPath = new QString(realPath(localxdgdatadir() + dirs->last()));
          else if (strncmp(type, "xdgconf-", 8) == 0)
             pPath = new QString(realPath(localxdgconfdir() + dirs->last()));
          else
             pPath = new QString(realPath(localkdedir() + dirs->last()));
       }
       else {
          dirs = absolutes.find(type);
          if (!dirs)
             qFatal("KStandardDirs: The resource type %s is not registered", type);
          pPath = new QString(realPath(dirs->last()));
       }

       savelocations.insert(type, pPath);
    }
    QString fullPath = *pPath + suffix;

    struct stat st;
    if (stat(QFile::encodeName(fullPath), &st) != 0 || !(S_ISDIR(st.st_mode))) {
	if(!create) {
#ifndef NDEBUG
	    qDebug("save location %s doesn't exist", fullPath.latin1());
#endif
	    return fullPath;
	}
	if(!makeDir(fullPath, 0700)) {
            qWarning("failed to create %s", fullPath.latin1());
	    return fullPath;
	}
        dircache.remove(type);
    }
    return fullPath;
}

QString KStandardDirs::relativeLocation(const char *type, const QString &absPath)
{
    QString fullPath = absPath;
    int i = absPath.findRev('/');
    if (i != -1)
    {
       fullPath = realPath(absPath.left(i+1))+absPath.mid(i+1); // Normalize
    }

    QStringList candidates = resourceDirs(type);

    for (QStringList::ConstIterator it = candidates.begin();
	 it != candidates.end(); it++)
      if (fullPath.startsWith(*it))
      {
	return fullPath.mid((*it).length());
      }

    return absPath;
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

    QString base("");
    uint i = 1;

    while( i < len )
    {
        struct stat st;
        int pos = target.find('/', i);
        base += target.mid(i - 1, pos - i + 1);
        QCString baseEncoded = QFile::encodeName(base);
        // bail out if we encountered a problem
        if (stat(baseEncoded, &st) != 0)
        {
          // Directory does not exist....
          // Or maybe a dangling symlink ?
          if (lstat(baseEncoded, &st) == 0)
              (void)unlink(baseEncoded); // try removing

	  if ( mkdir(baseEncoded, (mode_t) mode) != 0) {
	    perror("trying to create local folder");
	    return false; // Couldn't create it :-(
	  }
        }
        i = pos + 1;
    }
    return true;
}

static QString readEnvPath(const char *env)
{
   QCString c_path = getenv(env);
   if (c_path.isEmpty())
      return QString::null;
   return QFile::decodeName(c_path);
}

#ifdef __linux__
static QString executablePrefix()
{
   char path_buffer[MAXPATHLEN + 1];
   path_buffer[MAXPATHLEN] = 0;
   int length = readlink ("/proc/self/exe", path_buffer, MAXPATHLEN);
   if (length == -1)
      return QString::null;

   path_buffer[length] = '\0';

   QString path = QFile::decodeName(path_buffer);

   if(path.isEmpty())
      return QString::null;

   int pos = path.findRev('/'); // Skip filename
   if(pos <= 0)
      return QString::null;
   pos = path.findRev('/', pos - 1); // Skip last directory
   if(pos <= 0)
      return QString::null;

   return path.left(pos);
}
#endif

void KStandardDirs::addKDEDefaults()
{
    QStringList kdedirList;

    // begin KDEDIRS
    QString kdedirs = readEnvPath("KDEDIRS");
    if (!kdedirs.isEmpty())
    {
	tokenize(kdedirList, kdedirs, ":");
    }
    else
    {
	QString kdedir = readEnvPath("KDEDIR");
	if (!kdedir.isEmpty())
        {
           kdedir = KShell::tildeExpand(kdedir);
	   kdedirList.append(kdedir);
        }
    }
    kdedirList.append(KDEDIR);

#ifdef __KDE_EXECPREFIX
    QString execPrefix(__KDE_EXECPREFIX);
    if (execPrefix!="NONE")
       kdedirList.append(execPrefix);
#endif
#ifdef __linux__
    kdedirList.append(executablePrefix());
#endif

    // We treat root differently to prevent a "su" shell messing up the
    // file permissions in the user's home directory.
    QString localKdeDir = readEnvPath(getuid() ? "KDEHOME" : "KDEROOTHOME");
    if (!localKdeDir.isEmpty())
    {
       if (localKdeDir[localKdeDir.length()-1] != '/')
          localKdeDir += '/';
    }
    else
    {
       localKdeDir =  QDir::homeDirPath() + "/.kde/";
    }

    if (localKdeDir != "-/")
    {
        localKdeDir = KShell::tildeExpand(localKdeDir);
        addPrefix(localKdeDir);
    }

    for (QStringList::ConstIterator it = kdedirList.begin();
	 it != kdedirList.end(); it++)
    {
        QString dir = KShell::tildeExpand(*it);
	addPrefix(dir);
    }
    // end KDEDIRS

    // begin XDG_CONFIG_XXX
    QStringList xdgdirList;
    QString xdgdirs = readEnvPath("XDG_CONFIG_DIRS");
    if (!xdgdirs.isEmpty())
    {
	tokenize(xdgdirList, xdgdirs, ":");
    }
    else
    {
	xdgdirList.clear();
        xdgdirList.append("/etc/xdg");
        xdgdirList.append(KDESYSCONFDIR "/xdg");
    }

    QString localXdgDir = readEnvPath("XDG_CONFIG_HOME");
    if (!localXdgDir.isEmpty())
    {
       if (localXdgDir[localXdgDir.length()-1] != '/')
          localXdgDir += '/';
    }
    else
    {
       localXdgDir =  QDir::homeDirPath() + "/.config/";
    }

    localXdgDir = KShell::tildeExpand(localXdgDir);
    addXdgConfigPrefix(localXdgDir);

    for (QStringList::ConstIterator it = xdgdirList.begin();
	 it != xdgdirList.end(); it++)
    {
        QString dir = KShell::tildeExpand(*it);
	addXdgConfigPrefix(dir);
    }
    // end XDG_CONFIG_XXX

    // begin XDG_DATA_XXX
    xdgdirs = readEnvPath("XDG_DATA_DIRS");
    if (!xdgdirs.isEmpty())
    {
	tokenize(xdgdirList, xdgdirs, ":");
    }
    else
    {
	xdgdirList.clear();
        for (QStringList::ConstIterator it = kdedirList.begin();
           it != kdedirList.end(); it++)
        {
           QString dir = *it;
           if (dir[dir.length()-1] != '/')
             dir += '/';
           xdgdirList.append(dir+"share/");
        }

        xdgdirList.append("/usr/local/share/");
        xdgdirList.append("/usr/share/");
    }

    localXdgDir = readEnvPath("XDG_DATA_HOME");
    if (!localXdgDir.isEmpty())
    {
       if (localXdgDir[localXdgDir.length()-1] != '/')
          localXdgDir += '/';
    }
    else
    {
       localXdgDir = QDir::homeDirPath() + "/.local/share/";
    }

    localXdgDir = KShell::tildeExpand(localXdgDir);
    addXdgDataPrefix(localXdgDir);

    for (QStringList::ConstIterator it = xdgdirList.begin();
	 it != xdgdirList.end(); it++)
    {
        QString dir = KShell::tildeExpand(*it);
	addXdgDataPrefix(dir);
    }
    // end XDG_DATA_XXX


    uint index = 0;
    while (types[index] != 0) {
	addResourceType(types[index], kde_default(types[index]));
	index++;
    }

    addResourceDir("home", QDir::homeDirPath());
}

void KStandardDirs::checkConfig() const
{
    if (!addedCustoms && KGlobal::_instance && KGlobal::_instance->_config)
        const_cast<KStandardDirs*>(this)->addCustomized(KGlobal::_instance->_config);
}

static QStringList lookupProfiles(const QString &mapFile)
{
    QStringList profiles;

    if (mapFile.isEmpty() || !QFile::exists(mapFile))
    {
       profiles << "default";
       return profiles;
    }

    struct passwd *pw = getpwuid(geteuid());
    if (!pw)
    {
        profiles << "default";
        return profiles; // Not good
    }

    QCString user = pw->pw_name;

    KSimpleConfig mapCfg(mapFile, true);
    mapCfg.setGroup("Users");
    if (mapCfg.hasKey(user.data()))
    {
        profiles = mapCfg.readListEntry(user.data());
        return profiles; 
    }
        
    mapCfg.setGroup("General");
    QStringList groups = mapCfg.readListEntry("groups");

    mapCfg.setGroup("Groups");

    for( QStringList::ConstIterator it = groups.begin();
         it != groups.end(); ++it )
    {
        QCString grp = (*it).utf8();
        // Check if user is in this group
        struct group *grp_ent = getgrnam(grp);
        if (!grp_ent) continue;

        char ** members = grp_ent->gr_mem;
        for(char * member; (member = *members); ++members)
        {
            if (user == member)
            {
                // User is in this group --> add profiles
                profiles += mapCfg.readListEntry(*it);
                break;
            }
        }
    }

    if (profiles.isEmpty())
        profiles << "default";
    return profiles;
}

bool KStandardDirs::addCustomized(KConfig *config)
{
    if (addedCustoms) // there are already customized entries
        return false; // we just quit and hope they are the right ones

    // save it for future calls - that will return
    addedCustoms = true;

    // save the numbers of config directories. If this changes,
    // we will return true to give KConfig a chance to reparse
    uint configdirs = resourceDirs("config").count();

    // reading the prefixes in
    QString oldGroup = config->group();
    QString group = QString::fromLatin1("Directories");
    config->setGroup(group);
    QString userMapFile = config->readEntry("userProfileMapFile");

    QStringList profiles = lookupProfiles(userMapFile);
    
    bool priority = false;
    while(true)
    {
        config->setGroup(group);
        QStringList list = config->readListEntry("prefixes");
        for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++)
        {
            addPrefix(*it, priority);
	    addXdgConfigPrefix(*it+"/etc/xdg", priority);
	}

        // iterating over all entries in the group Directories
        // to find entries that start with dir_$type
        QMap<QString, QString> entries = config->entryMap(group);
        for (QMap<QString, QString>::ConstIterator it2 = entries.begin(); 
             it2 != entries.end(); it2++)
        {
            QString key = it2.key();
            if (key.startsWith("dir_")) {
                // generate directory list, there may be more than 1.
                QStringList dirs = QStringList::split(',',
						  *it2);
                QStringList::Iterator sIt(dirs.begin());
                QString resType = key.mid(4, key.length());
                for (; sIt != dirs.end(); ++sIt) {
                    addResourceDir(resType.latin1(), *sIt, priority);
                }
            }
        }
        if (profiles.isEmpty())
           break;
        group = QString::fromLatin1("Directories-%1").arg(profiles.back());
        profiles.pop_back();
        priority = true;
    }

    // Process KIOSK restrictions.
    config->setGroup("KDE Resource Restrictions");
    QMap<QString, QString> entries = config->entryMap("KDE Resource Restrictions");
    for (QMap<QString, QString>::ConstIterator it2 = entries.begin(); 
         it2 != entries.end(); it2++)
    {
	QString key = it2.key();
        if (!config->readBoolEntry(key, true))
        {
           d->restrictionsActive = true;
           d->restrictions.insert(key.latin1(), &d->restrictionsActive); // Anything will do
           dircache.remove(key.latin1());
        }
    }

    config->setGroup(oldGroup);

    // return true if the number of config dirs changed
    return (resourceDirs("config").count() != configdirs);
}

QString KStandardDirs::localkdedir() const
{
    // Return the prefix to use for saving
    return prefixes.first();
}

QString KStandardDirs::localxdgdatadir() const
{
    // Return the prefix to use for saving
    return d->xdgdata_prefixes.first();
}

QString KStandardDirs::localxdgconfdir() const
{
    // Return the prefix to use for saving
    return d->xdgconf_prefixes.first();
}


// just to make code more readable without macros
QString locate( const char *type,
		const QString& filename, const KInstance* inst )
{
    return inst->dirs()->findResource(type, filename);
}

QString locateLocal( const char *type,
	             const QString& filename, const KInstance* inst )
{
    return locateLocal(type, filename, true, inst);
}

QString locateLocal( const char *type,
	             const QString& filename, bool createDir, const KInstance* inst )
{
    // try to find slashes. If there are some, we have to
    // create the subdir first
    int slash = filename.findRev('/')+1;
    if (!slash) // only one filename
	return inst->dirs()->saveLocation(type, QString::null, createDir) + filename;

    // split path from filename
    QString dir = filename.left(slash);
    QString file = filename.mid(slash);
    return inst->dirs()->saveLocation(type, dir, createDir) + file;
}
