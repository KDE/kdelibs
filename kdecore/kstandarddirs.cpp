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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/*
 * Author: Stephan Kulow <coolo@kde.org> and Sirtaj Singh Kang <taj@kde.org>
 * Generated:	Thu Mar  5 16:05:28 EST 1998
 */

#include "config.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#include <qregexp.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qsettings.h>
#include <qstring.h>
#include <qstringlist.h>

#include "kstandarddirs.h"
#include "kconfig.h"
#include "kdebug.h"
#include "kinstance.h"
#include "kshell.h"
#include "ksimpleconfig.h"
#include "kuser.h"
#include "kstaticdeleter.h"
#include <kde_file.h>


class KStandardDirs::KStandardDirsPrivate
{
public:
   KStandardDirsPrivate()
    : restrictionsActive(false),
      dataRestrictionActive(false),
      checkRestrictions(true)
   { }

   bool restrictionsActive;
   bool dataRestrictionActive;
   bool checkRestrictions;
   QMap<QByteArray, bool> restrictions;
   QStringList xdgdata_prefixes;
   QStringList xdgconf_prefixes;
};

// Singleton, with data shared by all kstandarddirs instances.
// Used in static methods like findExe()
class KStandardDirsSingleton
{
public:
   QString defaultprefix;
   QString defaultbindir;
   static KStandardDirsSingleton* self();
private:
   static KStandardDirsSingleton* s_self;
};
static KStaticDeleter<KStandardDirsSingleton> kstds_sd;
KStandardDirsSingleton* KStandardDirsSingleton::s_self = 0;
KStandardDirsSingleton* KStandardDirsSingleton::self() {
    if ( !s_self )
        kstds_sd.setObject( s_self, new KStandardDirsSingleton );
    return s_self;
}

static const char* const types[] = {"html", "icon", "apps", "sound",
			      "data", "locale", "services", "mime",
			      "servicetypes", "config", "exe",
			      "wallpaper", "lib", "pixmap", "templates",
			      "module", "qtplugins",
			      "xdgdata-apps", "xdgdata-dirs", "xdgconf-menu",
			      "kcfg", "emoticons", 0 };

static int tokenize( QStringList& token, const QString& str,
		const QString& delim );

KStandardDirs::KStandardDirs() : addedCustoms(false)
{
    d = new KStandardDirsPrivate;
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

   if (d->restrictions.value(type, false))
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
   int i = relPath.indexOf('/');
   if (i != -1)
      key = "data_"+relPath.left(i);
   else
      key = "data_"+relPath;

   if (d && d->restrictions.value(key.toLatin1(), false))
      d->dataRestrictionActive = true;
}


QStringList KStandardDirs::allTypes() const
{
    QStringList list;
    for (int i = 0; types[i] != 0; ++i)
        list.append(QLatin1String(types[i]));
    return list;
}

static void priorityAdd(QStringList &prefixes, const QString& dir, bool priority)
{
    if (priority && !prefixes.isEmpty())
    {
        // Add in front but behind $KDEHOME
        QStringList::iterator it = prefixes.begin();
        it++;
        prefixes.insert(it, dir);
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
   return prefixes.join(QString(QChar(KPATH_SEPARATOR)));
}

QString KStandardDirs::kfsstnd_xdg_conf_prefixes()
{
   return d->xdgconf_prefixes.join(QString(QChar(KPATH_SEPARATOR)));
}

QString KStandardDirs::kfsstnd_xdg_data_prefixes()
{
   return d->xdgdata_prefixes.join(QString(QChar(KPATH_SEPARATOR)));
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

    QStringList& rels = relatives[type]; // find or insert
    QString copy = relativename;
    if (copy.at(copy.length() - 1) != '/')
	copy += '/';
    if (!rels.contains(copy)) {
        if (priority)
	    rels.prepend(copy);
	else
	    rels.append(copy);
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
    // find or insert entry in the map
    QStringList &paths = absolutes[type];
    QString copy = absdir;
    if (copy.at(copy.length() - 1) != '/')
      copy += '/';

    if (!paths.contains(copy)) {
        if (priority)
            paths.prepend(copy);
        else
	    paths.append(copy);
	dircache.remove(type); // clean the cache
	return true;
    }
    return false;
}

QString KStandardDirs::findResource( const char *type,
				     const QString& filename ) const
{
	if (!QDir::isRelativePath(filename))
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

static quint32 updateHash(const QString &file, quint32 hash)
{
    QByteArray cFile = QFile::encodeName(file);
    KDE_struct_stat buff;
    if ((access(cFile, R_OK) == 0) &&
        (KDE_stat( cFile, &buff ) == 0) &&
        (S_ISREG( buff.st_mode )))
    {
       hash = hash + (quint32) buff.st_ctime;
    }
    return hash;
}

quint32 KStandardDirs::calcResourceHash( const char *type,
			      const QString& filename, bool deep) const
{
    quint32 hash = 0;

    if (!QDir::isRelativePath(filename))
    {
        // absolute dirs are absolute dirs, right? :-/
	return updateHash(filename, hash);
    }
    if (d && d->restrictionsActive && (strcmp(type, "data")==0))
       applyDataRestrictions(filename);
    QStringList candidates = resourceDirs(type);
    QString fullPath;

    for (QStringList::ConstIterator it = candidates.begin();
	 it != candidates.end(); ++it)
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
    if (!QDir::isRelativePath(reldir))
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
         it != candidates.end(); ++it) {
        testdir.setPath(*it + reldir);
        if (testdir.exists())
            list.append(testdir.absolutePath() + '/');
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
      it != candidates.end(); ++it) {
      if (exists(*it + filename)) {
#ifdef Q_WS_WIN //this ensures we're using installed .la files
          if ((*it).isEmpty() && filename.right(3)==".la") {
#ifndef NDEBUG
              kdDebug() << "KStandardDirs::findResourceDir() found .la in cwd: skipping. (fname=" << filename  << ")" << endl;
#endif
              continue;
          }
#endif //Q_WS_WIN
          return *it;
      }
    }

#ifndef NDEBUG
    if(false && type != "locale")
      kdDebug() << "KStdDirs::findResDir(): can't find \"" << filename << "\" in type \"" << type << "\"." << endl;
#endif

    return QString::null;
}

bool KStandardDirs::exists(const QString &fullPath)
{
    KDE_struct_stat buff;
    if (access(QFile::encodeName(fullPath), R_OK) == 0 && KDE_stat( QFile::encodeName(fullPath), &buff ) == 0)
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
    if (path.isEmpty()) //for sanity
      return;
    // We look for a set of files.
    DIR *dp = opendir( QFile::encodeName(path));
    if (!dp)
      return;

#ifdef Q_WS_WIN
    assert(path.at(path.length() - 1) == '/' || path.at(path.length() - 1) == '\\');
#else
    assert(path.at(path.length() - 1) == '/');
#endif

    struct dirent *ep;
    KDE_struct_stat buff;

    QString _dot(".");
    QString _dotdot("..");

    while( ( ep = readdir( dp ) ) != 0L )
    {
      QString fn( QFile::decodeName(ep->d_name));
      if (fn == _dot || fn == _dotdot || fn.at(fn.length() - 1).toLatin1() == '~')
	continue;

      if (!recursive && !regexp.exactMatch(fn))
	continue; // No match

      QString pathfn = path + fn;
      if ( KDE_stat( QFile::encodeName(pathfn), &buff ) != 0 ) {
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
     KDE_struct_stat buff;
     if ( KDE_stat( QFile::encodeName(pathfn), &buff ) != 0 )
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
       int slash = relpath.indexOf('/');
       if (slash < 0)
	   rest = relpath.left(relpath.length() - 1);
       else {
	   path = relpath.left(slash);
	   rest = relpath.mid(slash + 1);
       }
    }

    if (prefix.isEmpty()) //for sanity
      return;
#ifdef Q_WS_WIN
    assert(prefix.at(prefix.length() - 1) == '/' || prefix.at(prefix.length() - 1) == '\\');
#else
    assert(prefix.at(prefix.length() - 1) == '/');
#endif
    KDE_struct_stat buff;

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
		if ( KDE_stat( QFile::encodeName(fn), &buff ) != 0 ) {
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
       int slash = filter.lastIndexOf('/');
       if (slash < 0)
	   filterFile = filter;
       else {
	   filterPath = filter.left(slash + 1);
	   filterFile = filter.mid(slash + 1);
       }
    }

    checkConfig();

    QStringList candidates;
	if (!QDir::isRelativePath(filter)) // absolute path
    {
#ifdef Q_OS_WIN
        candidates << filterPath.left(3); //e.g. "C:\"
        filterPath = filterPath.mid(3);
#else
        candidates << "/";
        filterPath = filterPath.mid(1);
#endif
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
         it != candidates.end(); ++it)
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

QString
KStandardDirs::realFilePath(const QString &filename)
{
    char realpath_buffer[MAXPATHLEN + 1];
    memset(realpath_buffer, 0, MAXPATHLEN + 1);

    /* If the path contains symlinks, get the real name */
    if (realpath( QFile::encodeName(filename).data(), realpath_buffer) != 0) {
        // succes, use result from realpath
        return QFile::decodeName(realpath_buffer);
    }

    return filename;
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
   bool relink = (result == -1) && (errno == ENOENT);
   if (result > 0)
   {
      link[result] = 0;
      if (!QDir::isRelativePath(link))
      {
         KDE_struct_stat stat_buf;
         int res = KDE_lstat(link, &stat_buf);
         if ((res == -1) && (errno == ENOENT))
         {
            relink = true;
         }
         else if ((res == -1) || (!S_ISDIR(stat_buf.st_mode)))
         {
            fprintf(stderr, "Error: \"%s\" is not a directory.\n", link);
            relink = true;
         }
         else if (stat_buf.st_uid != getuid())
         {
            fprintf(stderr, "Error: \"%s\" is owned by uid %d instead of uid %d.\n", link, stat_buf.st_uid, getuid());
            relink = true;
         }
      }
   }
#ifdef Q_WS_WIN
   if (relink)
   {
      if (!makeDir(dir, 0700))
         fprintf(stderr, "failed to create \"%s\"", qPrintable(dir));
      else
         result = readlink(QFile::encodeName(dir).data(), link, 1023);
   }
#else //UNIX
   if (relink)
   {
      QString srv = findExe(QLatin1String("lnusertemp"), kfsstnd_defaultbindir());
      if (srv.isEmpty())
         srv = findExe(QLatin1String("lnusertemp"));
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
         dir = QDir::cleanPath(dir+QFile::decodeName(link));
   }
#endif
   addResourceDir(type, dir+'/');
}

QStringList KStandardDirs::resourceDirs(const char *type) const
{
    QMap<QByteArray, QStringList>::const_iterator dirCacheIt = dircache.find(type);

    QStringList candidates;

    if (dirCacheIt != dircache.end())
    {
        candidates = *dirCacheIt;
    }
    else // filling cache
    {
        if (strcmp(type, "socket") == 0)
           const_cast<KStandardDirs *>(this)->createSpecialResource(type);
        else if (strcmp(type, "tmp") == 0)
           const_cast<KStandardDirs *>(this)->createSpecialResource(type);
        else if (strcmp(type, "cache") == 0)
           const_cast<KStandardDirs *>(this)->createSpecialResource(type);

        QDir testdir;

        bool restrictionActive = false;
        if (d && d->restrictionsActive)
        {
           if (d->dataRestrictionActive)
              restrictionActive = true;
           else if (d->restrictions.value("all", false))
              restrictionActive = true;
           else if (d->restrictions.value(type, false))
              restrictionActive = true;
           d->dataRestrictionActive = false; // Reset
        }

        QStringList dirs;
        dirs = relatives.value(type);
        if (!dirs.isEmpty())
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
                 ++pit)
            {
                for (QStringList::ConstIterator it = dirs.begin();
                     it != dirs.end(); ++it) {
                    QString path = realPath(*pit + *it);
                    testdir.setPath(path);
                    if (local && restrictionActive)
                       continue;
                    if ((local || testdir.exists()) && !candidates.contains(path))
                        candidates.append(path);
                }
                local = false;
            }
        }
        dirs = absolutes.value(type);
        if (!dirs.isEmpty())
            for (QStringList::ConstIterator it = dirs.begin();
                 it != dirs.end(); ++it)
            {
                testdir.setPath(*it);
                if (testdir.exists())
                {
                    QString filename = realPath(*it);
                    if (!candidates.contains(filename))
                        candidates.append(filename);
                }
            }
        dircache.insert(type, candidates);
    }

#if 0
    kdDebug() << "found dirs for resource " << type << ":" << endl;
    for (QStringList::ConstIterator pit = candidates.begin();
	 pit != candidates.end();
	 pit++)
    {
	fprintf(stderr, "%s\n", qPrintable(*pit));
    }
#endif

  return candidates;
}

QStringList KStandardDirs::systemPaths( const QString& pstr )
{
    QStringList tokens;
    QString p = pstr;

    if( p.isNull() )
    {
	p = getenv( "PATH" );
    }

    QString delimiters(QChar(KPATH_SEPARATOR));
    delimiters += "\b";
    tokenize( tokens, p, delimiters );

    QStringList exePaths;

    // split path using : or \b as delimiters
    for( int i = 0; i < tokens.count(); i++ )
    {
	p = tokens[ i ];

        if ( p[ 0 ] == '~' )
        {
            int len = p.indexOf( '/' );
            if ( len == -1 )
                len = p.length();
            if ( len == 1 )
            {
                p.replace( 0, 1, QDir::homePath() );
            }
            else
            {
                QString user = p.mid( 1, len - 1 );
                struct passwd *dir = getpwnam( user.toLocal8Bit().data() );
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
#ifdef Q_WS_WIN
    QString real_appname = appname + ".exe";
#else
    QString real_appname = appname;
#endif
    QFileInfo info;

    // absolute path ?
    if (!QDir::isRelativePath(real_appname))
    {
        info.setFile( real_appname );
        if( info.exists() && ( ignore || info.isExecutable() )
            && info.isFile() ) {
            return real_appname;
        }
        return QString::null;
    }

    QString p = QString("%1/%2").arg(kfsstnd_defaultbindir()).arg(real_appname);
    info.setFile( p );
    if( info.exists() && ( ignore || info.isExecutable() )
         && ( info.isFile() || info.isSymLink() )  ) {
         return p;
    }

    QStringList exePaths = systemPaths( pstr );
    for (QStringList::ConstIterator it = exePaths.begin(); it != exePaths.end(); ++it)
    {
	p = (*it) + "/";
	p += real_appname;

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
#ifdef Q_WS_WIN
    QString real_appname = appname + ".exe";
#else
    QString real_appname = appname;
#endif
    QFileInfo info;
    QString p;
    list.clear();

    QStringList exePaths = systemPaths( pstr );
    for (QStringList::ConstIterator it = exePaths.begin(); it != exePaths.end(); ++it)
    {
	p = (*it) + "/";
	p += real_appname;

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
	if ( delim.indexOf( str[ index ] ) >= 0 )
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
	return "lib" KDELIBSUFF "/kde4/";
    if (!strcmp(type, "qtplugins"))
        return "lib" KDELIBSUFF "/kde4/plugins";
    if (!strcmp(type, "xdgdata-apps"))
        return "applications/";
    if (!strcmp(type, "xdgdata-dirs"))
        return "desktop-directories/";
    if (!strcmp(type, "xdgconf-menu"))
        return "menus/";
    if (!strcmp(type, "kcfg"))
	return "share/config.kcfg";
    if (!strcmp(type, "emoticons"))
			return "share/emoticons";


    qFatal("unknown resource type %s", type);
    return QString::null;
}

QString KStandardDirs::saveLocation(const char *type,
				    const QString& suffix,
				    bool create) const
{
    checkConfig();

    QString path = savelocations.value(type);
    if (path.isEmpty())
    {
       QStringList dirs = relatives.value(type);
       if (dirs.isEmpty() && (
                     (strcmp(type, "socket") == 0) ||
                     (strcmp(type, "tmp") == 0) ||
                     (strcmp(type, "cache") == 0) ))
       {
          (void) resourceDirs(type); // Generate socket|tmp|cache resource.
          dirs = relatives.value(type); // Search again.
       }
       if (!dirs.isEmpty())
       {
          // Check for existence of typed directory + suffix
          if (strncmp(type, "xdgdata-", 8) == 0)
             path = realPath(localxdgdatadir() + dirs.last());
          else if (strncmp(type, "xdgconf-", 8) == 0)
             path = realPath(localxdgconfdir() + dirs.last());
          else
             path = realPath(localkdedir() + dirs.last());
       }
       else {
          dirs = absolutes.value(type);
          if (dirs.isEmpty())
             qFatal("KStandardDirs: The resource type %s is not registered", type);
          path = realPath(dirs.last());
       }

       savelocations.insert(type, path);
    }
    QString fullPath = path + (path.endsWith("/") ? "" : "/") + suffix;

    KDE_struct_stat st;
    if (KDE_stat(QFile::encodeName(fullPath), &st) != 0 || !(S_ISDIR(st.st_mode))) {
	if(!create) {
#ifndef NDEBUG
	    kdDebug() << QString("save location %1 doesn't exist").arg(fullPath) << endl;
#endif
	    return fullPath;
	}
	if(!makeDir(fullPath, 0700)) {
	    return fullPath;
	}
        dircache.remove(type);
    }
    if (!fullPath.endsWith("/"))
	    fullPath += "/";
    return fullPath;
}

QString KStandardDirs::relativeLocation(const char *type, const QString &absPath)
{
    QString fullPath = absPath;
    int i = absPath.lastIndexOf('/');
    if (i != -1)
    {
       fullPath = realPath(absPath.left(i+1))+absPath.mid(i+1); // Normalize
    }

    QStringList candidates = resourceDirs(type);

    for (QStringList::ConstIterator it = candidates.begin();
	 it != candidates.end(); ++it)
      if (fullPath.startsWith(*it))
      {
	return fullPath.mid((*it).length());
      }

    return absPath;
}


bool KStandardDirs::makeDir(const QString& dir, int mode)
{
    // we want an absolute path
    if (QDir::isRelativePath(dir))
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
        KDE_struct_stat st;
        int pos = target.indexOf('/', i);
        base += target.mid(i - 1, pos - i + 1);
        QByteArray baseEncoded = QFile::encodeName(base);
        // bail out if we encountered a problem
        if (KDE_stat(baseEncoded, &st) != 0)
        {
          // Directory does not exist....
          // Or maybe a dangling symlink ?
          if (KDE_lstat(baseEncoded, &st) == 0)
              (void)unlink(baseEncoded); // try removing

	  if ( KDE_mkdir(baseEncoded, (mode_t) mode) != 0) {
            baseEncoded.prepend( "trying to create local folder " );
	    perror(baseEncoded.data());
	    return false; // Couldn't create it :-(
	  }
        }
        i = pos + 1;
    }
    return true;
}

static QString readEnvPath(const char *env)
{
   QByteArray c_path = getenv(env);
   if (c_path.isEmpty())
      return QString::null;
#ifdef Q_OS_WIN
   //win32 paths are case-insensitive: avoid duplicates on various dir lists
   return QFile::decodeName(c_path).toLower();
#else
   return QFile::decodeName(c_path);
#endif
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

   int pos = path.lastIndexOf('/'); // Skip filename
   if(pos <= 0)
      return QString::null;
   pos = path.lastIndexOf('/', pos - 1); // Skip last directory
   if(pos <= 0)
      return QString::null;

   return path.left(pos);
}
#endif

QString KStandardDirs::kfsstnd_defaultprefix()
{
   KStandardDirsSingleton* s = KStandardDirsSingleton::self();
   if (!s->defaultprefix.isEmpty())
      return s->defaultprefix;
#ifdef Q_WS_WIN
   s->defaultprefix = readEnvPath("KDEDIR");
   if (s->defaultprefix.isEmpty()) {
      s->defaultprefix = QFile::decodeName("c:\\kde");
      //TODO: find other location (the Registry?)
   }
#else //UNIX
   s->defaultprefix = KDEDIR;
#endif
   if (s->defaultprefix.isEmpty())
      kdWarning() << "KStandardDirs::kfsstnd_defaultprefix(): default KDE prefix not found!" << endl;
   return s->defaultprefix;
}

QString KStandardDirs::kfsstnd_defaultbindir()
{
   KStandardDirsSingleton* s = KStandardDirsSingleton::self();
   if (!s->defaultbindir.isEmpty())
      return s->defaultbindir;
#ifdef Q_WS_WIN
   s->defaultbindir = kfsstnd_defaultprefix() + QLatin1String("/bin");
#else //UNIX
   s->defaultbindir = __KDE_BINDIR;
   if (s->defaultbindir.isEmpty())
      s->defaultbindir = kfsstnd_defaultprefix() + QLatin1String("/bin");
#endif
   if (s->defaultbindir.isEmpty())
      kdWarning() << "KStandardDirs::kfsstnd_defaultbindir(): default binary KDE dir not found!" << endl;
  return s->defaultbindir;
}

void KStandardDirs::addResourcesFrom_krcdirs()
{
    QString localFile = QDir::currentPath() + QDir::separator() + ".krcdirs";
    if (!QFile::exists(localFile))
        return;

    QSettings iniFile(localFile, QSettings::IniFormat);
    iniFile.beginGroup("KStandardDirs");
    const QStringList resources = iniFile.allKeys();
    foreach(QString key, resources)
    {
        QDir path(iniFile.value(key).toString());
        if (!path.exists())
            continue;

        if(path.makeAbsolute())
            addResourceDir(key.ascii(), path.path());
    }
}

void KStandardDirs::addKDEDefaults()
{
    addResourcesFrom_krcdirs();

    QStringList kdedirList;

    // begin KDEDIRS
    QString kdedirs = readEnvPath("KDEDIRS");
    if (!kdedirs.isEmpty())
    {
        tokenize(kdedirList, kdedirs, QString(QChar(KPATH_SEPARATOR)));
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
#ifndef Q_OS_WIN //no default KDEDIR on win32 defined
    kdedirList.append(KDEDIR);
#endif

#ifdef __KDE_EXECPREFIX
    QString execPrefix(__KDE_EXECPREFIX);
    if (execPrefix!="NONE")
       kdedirList.append(execPrefix);
#endif
#ifdef __linux__
    const QString linuxExecPrefix = executablePrefix();
    if ( !linuxExecPrefix.isEmpty() )
       kdedirList.append( linuxExecPrefix );
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
       localKdeDir =  QDir::homePath() + "/.kde/";
    }

    if (localKdeDir != "-/")
    {
        localKdeDir = KShell::tildeExpand(localKdeDir);
        addPrefix(localKdeDir);
    }

	QStringList::ConstIterator end(kdedirList.end());
    for (QStringList::ConstIterator it = kdedirList.begin();
	 it != end; ++it)
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
	tokenize(xdgdirList, xdgdirs, QString(QChar(KPATH_SEPARATOR)));
    }
    else
    {
	xdgdirList.clear();
        xdgdirList.append("/etc/xdg");
#ifdef Q_WS_WIN
        xdgdirList.append(kfsstnd_defaultprefix() + "/etc/xdg");
#else
        xdgdirList.append(KDESYSCONFDIR "/xdg");
#endif
    }

    QString localXdgDir = readEnvPath("XDG_CONFIG_HOME");
    if (!localXdgDir.isEmpty())
    {
       if (localXdgDir[localXdgDir.length()-1] != '/')
          localXdgDir += '/';
    }
    else
    {
       localXdgDir =  QDir::homePath() + "/.config/";
    }

    localXdgDir = KShell::tildeExpand(localXdgDir);
    addXdgConfigPrefix(localXdgDir);

    for (QStringList::ConstIterator it = xdgdirList.begin();
	 it != xdgdirList.end(); ++it)
    {
        QString dir = KShell::tildeExpand(*it);
	addXdgConfigPrefix(dir);
    }
    // end XDG_CONFIG_XXX

    // begin XDG_DATA_XXX
    xdgdirs = readEnvPath("XDG_DATA_DIRS");
    if (!xdgdirs.isEmpty())
    {
	tokenize(xdgdirList, xdgdirs, QString(QChar(KPATH_SEPARATOR)));
    }
    else
    {
	xdgdirList.clear();
        for (QStringList::ConstIterator it = kdedirList.begin();
           it != kdedirList.end(); ++it)
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
       localXdgDir = QDir::homePath() + "/.local/share/";
    }

    localXdgDir = KShell::tildeExpand(localXdgDir);
    addXdgDataPrefix(localXdgDir);

    for (QStringList::ConstIterator it = xdgdirList.begin();
	 it != xdgdirList.end(); ++it)
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

    addResourceDir("home", QDir::homePath());
}

void KStandardDirs::checkConfig() const
{
    if (!addedCustoms && KGlobal::_instance && KGlobal::_instance->privateConfig())
        const_cast<KStandardDirs*>(this)->addCustomized(KGlobal::_instance->privateConfig());
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

    QByteArray user = pw->pw_name;

    gid_t sup_gids[512];
    int sup_gids_nr = getgroups(512, sup_gids);

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
        QByteArray grp = (*it).toUtf8();
        // Check if user is in this group
        struct group *grp_ent = getgrnam(grp);
        if (!grp_ent) continue;
        gid_t gid = grp_ent->gr_gid;
        if (pw->pw_gid == gid)
        {
            // User is in this group --> add profiles
            profiles += mapCfg.readListEntry(*it);
        }
        else
        {
            for(int i = 0; i < sup_gids_nr; i++)
            {
                if (sup_gids[i] == gid)
                {
                    // User is in this group --> add profiles
                    profiles += mapCfg.readListEntry(*it);
                    break;
                }
            }
        }
    }

    if (profiles.isEmpty())
        profiles << "default";
    return profiles;
}

extern bool kde_kiosk_admin;

bool KStandardDirs::addCustomized(KConfig *config)
{
    if (addedCustoms && !d->checkRestrictions) // there are already customized entries
        return false; // we just quit and hope they are the right ones

    // save the numbers of config directories. If this changes,
    // we will return true to give KConfig a chance to reparse
    int configdirs = resourceDirs("config").count();

    // Remember original group
    QString oldGroup = config->group();

    if (!addedCustoms)
    {
        // We only add custom entries once
        addedCustoms = true;

        // reading the prefixes in
        QString group = QLatin1String("Directories");
        config->setGroup(group);

        QString kioskAdmin = config->readEntry("kioskAdmin");
        if (!kioskAdmin.isEmpty() && !kde_kiosk_admin)
        {
            int i = kioskAdmin.indexOf(':');
            QString user = kioskAdmin.left(i);
            QString host = kioskAdmin.mid(i+1);

            KUser thisUser;
            char hostname[ 256 ];
            hostname[ 0 ] = '\0';
            if (!gethostname( hostname, 255 ))
                hostname[sizeof(hostname)-1] = '\0';

            if ((user == thisUser.loginName()) &&
                (host.isEmpty() || (host == hostname)))
            {
                kde_kiosk_admin = true;
            }
        }

        bool readProfiles = true;

        if (kde_kiosk_admin && !QByteArray(getenv("KDE_KIOSK_NO_PROFILES")).isEmpty())
            readProfiles = false;

        QString userMapFile = config->readEntry("userProfileMapFile");
        QString profileDirsPrefix = config->readEntry("profileDirsPrefix");
        if (!profileDirsPrefix.isEmpty() && !profileDirsPrefix.endsWith("/"))
            profileDirsPrefix.append("/");

        QStringList profiles;
        if (readProfiles)
            profiles = lookupProfiles(userMapFile);
        QString profile;

        bool priority = false;
        while(true)
        {
            config->setGroup(group);
            QStringList list = config->readListEntry("prefixes");
            for (QStringList::ConstIterator it = list.begin(); it != list.end(); ++it)
            {
                addPrefix(*it, priority);
                addXdgConfigPrefix(*it+"/etc/xdg", priority);
                addXdgDataPrefix(*it+"/share", priority);
            }
            // If there are no prefixes defined, check if there is a directory
            // for this profile under <profileDirsPrefix>
            if (list.isEmpty() && !profile.isEmpty() && !profileDirsPrefix.isEmpty())
            {
                QString dir = profileDirsPrefix + profile;
                addPrefix(dir, priority);
                addXdgConfigPrefix(dir+"/etc/xdg", priority);
                addXdgDataPrefix(dir+"/share", priority);
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
                    QStringList dirs = (*it2).split(',');
                    QStringList::Iterator sIt(dirs.begin());
                    QString resType = key.mid(4, key.length());
                    for (; sIt != dirs.end(); ++sIt)
                    {
                        addResourceDir(resType.toLatin1(), *sIt, priority);
                    }
                }
            }
            if (profiles.isEmpty())
                break;
            profile = profiles.back();
            group = QString::fromLatin1("Directories-%1").arg(profile);
            profiles.pop_back();
            priority = true;
        }
    }

    // Process KIOSK restrictions.
    if (!kde_kiosk_admin || QByteArray(getenv("KDE_KIOSK_NO_RESTRICTIONS")).isEmpty())
    {
        config->setGroup("KDE Resource Restrictions");
        QMap<QString, QString> entries = config->entryMap("KDE Resource Restrictions");
        for (QMap<QString, QString>::ConstIterator it2 = entries.begin();
            it2 != entries.end(); it2++)
        {
            QString key = it2.key();
            if (!config->readEntry(key, true).toBool())
            {
                d->restrictionsActive = true;
                d->restrictions.insert(key.toLatin1(), true);
                dircache.remove(key.toLatin1());
            }
        }
    }

    config->setGroup(oldGroup);

    // check if the number of config dirs changed
    bool configDirsChanged = (resourceDirs("config").count() != configdirs);
    // If the config dirs changed, we check kiosk restrictions again.
    d->checkRestrictions = configDirsChanged;
    // return true if the number of config dirs changed: reparse config file
    return configDirsChanged;
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
    int slash = filename.lastIndexOf('/')+1;
    if (!slash) // only one filename
	return inst->dirs()->saveLocation(type, QString::null, createDir) + filename;

    // split path from filename
    QString dir = filename.left(slash);
    QString file = filename.mid(slash);
    return inst->dirs()->saveLocation(type, dir, createDir) + file;
}
