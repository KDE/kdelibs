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

#include <qregexp.h>
#include <qdict.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qstring.h>
#include <qstringlist.h>

#include "kstandarddirs.h"
#include "kconfig.h"
#include "kdebug.h"
#include "kinstance.h"
#include <sys/param.h>
#include <unistd.h>

template class QDict<QStringList>;

static const char* const types[] = {"html", "icon", "apps", "sound",
			      "data", "locale", "services", "mime",
			      "servicetypes", "config", "exe",
			      "wallpaper", "lib", "pixmap", "templates", "module", "qtplugins", 0 };

static int tokenize( QStringList& token, const QString& str,
		const QString& delim );

KStandardDirs::KStandardDirs( ) : addedCustoms(false)
{
    dircache.setAutoDelete(true);
    relatives.setAutoDelete(true);
    absolutes.setAutoDelete(true);
    savelocations.setAutoDelete(true);
    addKDEDefaults();
}

KStandardDirs::~KStandardDirs()
{
}

QStringList KStandardDirs::allTypes() const
{
    QStringList list;
    for (int i = 0; types[i] != 0; ++i)
        list.append(QString::fromLatin1(types[i]));
    return list;
}

void KStandardDirs::addPrefix( const QString& _dir )
{
    if (_dir.isNull())
	return;

    QString dir = _dir;
    if (dir.at(dir.length() - 1) != '/')
	dir += '/';

    if (!prefixes.contains(dir)) {
	prefixes.append(dir);
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

bool KStandardDirs::addResourceDir( const char *type,
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
    if (dir.isNull())
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
    QStringList list;

    checkConfig();

    QStringList candidates = resourceDirs(type);
    QDir testdir;

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
			    bool recursive, bool uniq)
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

      if (!recursive && (regexp.search(fn) == -1))
	continue; // No match

      QString pathfn = path + fn;
      if ( stat( QFile::encodeName(pathfn), &buff ) != 0 ) {
	kdDebug() << "Error stat'ing " << pathfn << perror << endl;
	continue; // Couldn't stat (Why not?)
      }
      if ( recursive ) {
	if ( S_ISDIR( buff.st_mode )) {
	  lookupDirectory(pathfn + '/', relPart + fn + '/', regexp, list, relList, recursive, uniq);
	}
        if (regexp.search(fn) == -1)

	  continue; // No match
      }
      if ( S_ISREG( buff.st_mode))
      {
        if (!uniq || !relList.contains(relPart + fn))
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
       if (!uniq || !relList.contains(relPart + fn))
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
			 bool recursive, bool uniq)
{
    if (relpath.isNull()) {
       lookupDirectory(prefix, relPart, regexp, list,
		       relList, recursive, uniq);
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

		if (pathExp.search(fn) == -1)
		    continue; // No match
		QString rfn = relPart+fn;
		fn = prefix + fn;
		if ( stat( QFile::encodeName(fn), &buff ) != 0 ) {
		    kdDebug() << "Error statting " << fn << perror << endl;
		    continue; // Couldn't stat (Why not?)
		}
		if ( S_ISDIR( buff.st_mode ))
		    lookupPrefix(fn + '/', rest, rfn + '/', regexp, list, relList, recursive, uniq);
	    }

	closedir( dp );
    } else {
        // Don't stat, if the dir doesn't exist we will find out
        // when we try to open it.
        lookupPrefix(prefix + path + '/', rest,
                     relPart + path + '/', regexp, list,
                     relList, recursive, uniq);
    }
}

QStringList
KStandardDirs::findAllResources( const char *type,
			         const QString& filter,
				 bool recursive,
			         bool uniq,
                                 QStringList &relList) const
{
    QStringList list;
    if (filter.at(0) == '/') // absolute paths we return
    {
        list.append( filter);
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

    checkConfig();

    QStringList candidates = resourceDirs(type);
    if (filterFile.isEmpty())
	filterFile = "*";

    if (filterFile.find('*') || filterPath.find('*')) {
	QRegExp regExp(filterFile, true, true);

	for (QStringList::ConstIterator it = candidates.begin();
	     it != candidates.end(); it++)
	    lookupPrefix(*it, filterPath, "", regExp, list,
			 relList, recursive, uniq);
    } else  {
	for (QStringList::ConstIterator it = candidates.begin();
	     it != candidates.end(); it++) {
	    QString fullpath = *it + filter;
	    if (exists(fullpath))
		list.append(fullpath);
	}
    }

    return list;
}

QStringList
KStandardDirs::findAllResources( const char *type,
			         const QString& filter,
				 bool recursive,
			         bool uniq) const
{
    QStringList relList;
    return findAllResources(type, filter, recursive, uniq, relList);
}

static QString realPath(const QString &dirname)
{
    char realpath_buffer[MAXPATHLEN + 1];
    /* If the path contains symlinks, get the real name */
    if (realpath( QFile::encodeName(dirname), realpath_buffer) != 0) {
        // succes, use result from realpath
        int len = strlen(realpath_buffer);
        realpath_buffer[len] = '/';
        realpath_buffer[len+1] = 0;
        return QFile::decodeName(realpath_buffer);
    }

    return dirname;
}

QStringList KStandardDirs::resourceDirs(const char *type) const
{
    QStringList *candidates = dircache.find(type);

    if (!candidates) { // filling cache
        if (strcmp(type, "socket") == 0)
        {
          char hostname[256];
          hostname[0] = 0;
          gethostname(hostname, 255);
          QString dir = QString("%1socket-%2").arg(localkdedir()).arg(hostname);
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
                system(QFile::encodeName(srv)+" socket");
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
          const_cast<KStandardDirs *>(this)->addResourceDir("socket", dir+'/');
        }
        if (strcmp(type, "tmp") == 0)
        {
          char hostname[256];
          hostname[0] = 0;
          gethostname(hostname, 255);
          QString dir = QString("%1tmp-%2").arg(localkdedir()).arg(hostname);
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
                system(QFile::encodeName(srv)+" tmp");
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
          const_cast<KStandardDirs *>(this)->addResourceDir("tmp", dir+'/');
        }
        QDir testdir;

        candidates = new QStringList();
        QStringList *dirs;

        dirs = relatives.find(type);
        if (dirs)
        {
            bool local = true;
            for (QStringList::ConstIterator pit = prefixes.begin();
                 pit != prefixes.end();
                 pit++)
            {
                for (QStringList::ConstIterator it = dirs->begin();
                     it != dirs->end(); ++it) {
                    QString path = realPath(*pit + *it);
                    testdir.setPath(path);
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

    QStringList tokens;
    p = pstr;

    if( p == QString::null ) {
	p = getenv( "PATH" );
    }

    tokenize( tokens, p, ":\b" );

    // split path using : or \b as delimiters
    for( unsigned i = 0; i < tokens.count(); i++ ) {
	p = tokens[ i ];

        if ( p[ 0 ] == '~' )
        {
            int len = p.find( '/' );
            if ( len == -1 )
                len = p.length();
            if ( len == 1 )
                p.replace( 0, 1, QDir::homeDirPath() );
            else
            {
                QString user = p.mid( 1, len - 1 );
                struct passwd *dir = getpwnam( user.local8Bit().data() );
                if ( dir && strlen( dir->pw_dir ) )
                    p.replace( 0, len, QString::fromLocal8Bit( dir->pw_dir ) );
            }
        }

	p += "/";
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
	return "lib/";
    if (!strcmp(type, "module"))
	return "lib/kde3/";
    if (!strcmp(type, "qtplugins"))
        return "lib/kde3/plugins";
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
       if (!dirs && ((strcmp(type, "socket") == 0) || (strcmp(type, "tmp") == 0)))
       {
          (void) resourceDirs(type); // Generate socket resource.
          dirs = relatives.find(type); // Search again.
       }
       if (dirs)
       {
          // Check for existance of typed directory + suffix
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
	    return localkdedir()+suffix;
	}
	if(!makeDir(fullPath, 0700)) {
            qWarning("failed to create %s", fullPath.latin1());
	    return localkdedir()+suffix;
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

static void fixHomeDir(QString &dir)
{
   if (dir[0] == '~')
   {
      dir = QDir::homeDirPath() + dir.mid(1);
   }
}

void KStandardDirs::addKDEDefaults()
{
    QStringList kdedirList;

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
           fixHomeDir(kdedir);
	   kdedirList.append(kdedir);
        }
    }
    kdedirList.append(KDEDIR);

#ifdef __KDE_EXECPREFIX
    QString execPrefix(__KDE_EXECPREFIX);
    if (execPrefix!="NONE")
       kdedirList.append(execPrefix);
#endif

    QString localKdeDir;
    if (getuid())
    {
       localKdeDir = readEnvPath("KDEHOME");
       if (!localKdeDir.isEmpty())
       {
          if (localKdeDir[localKdeDir.length()-1] != '/')
             localKdeDir += '/';
       }
       else
       {
          localKdeDir =  QDir::homeDirPath() + "/.kde/";
       }
    }
    else
    {
       // We treat root different to prevent root messing up the
       // file permissions in the users home directory.
       localKdeDir = readEnvPath("KDEROOTHOME");
       if (!localKdeDir.isEmpty())
       {
          if (localKdeDir[localKdeDir.length()-1] != '/')
             localKdeDir += '/';
       }
       else
       {
          struct passwd *pw = getpwuid(0);
          localKdeDir =  QFile::decodeName((pw && pw->pw_dir) ? pw->pw_dir : "/root")  + "/.kde/";
       }

    }

    fixHomeDir(localKdeDir);
    addPrefix(localKdeDir);

    for (QStringList::ConstIterator it = kdedirList.begin();
	 it != kdedirList.end(); it++)
    {
        QString dir = *it;
        fixHomeDir(dir);
	addPrefix(dir);
    }

    uint index = 0;
    while (types[index] != 0) {
	addResourceType(types[index], kde_default(types[index]));
	index++;
    }

    QString dir = QString("%1share/cache/").arg(localKdeDir);
    addResourceDir("cache", dir);

    addResourceDir("home", QDir::homeDirPath());
}

void KStandardDirs::checkConfig() const
{
    if (!addedCustoms && KGlobal::_instance && KGlobal::_instance->_config)
        const_cast<KStandardDirs*>(this)->addCustomized(KGlobal::_instance->_config);
}

bool KStandardDirs::addCustomized(KConfig *config)
{
    if (addedCustoms) // there are already customized entries
        return false; // we just quite and hope they are the right ones

    // save the numbers of config directories. If this changes,
    // we will return true to give KConfig a chance to reparse
    uint configdirs = resourceDirs("config").count();

    // reading the prefixes in
    QString oldGroup = config->group();
    config->setGroup("Directories");

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
	if (key.left(4) == "dir_") {
	    // generate directory list, there may be more than 1.
	    QStringList dirs = QStringList::split(',',
						  *it2);
	    QStringList::Iterator sIt(dirs.begin());
	    QString resType = key.mid(4, key.length());
	    for (; sIt != dirs.end(); ++sIt) {
		addResourceDir(resType.latin1(), *sIt);
	    }
	}
    }

    // save it for future calls - that will return
    addedCustoms = true;
    config->setGroup(oldGroup);

    // return true if the number of config dirs changed
    return (resourceDirs("config").count() != configdirs);
}

QString KStandardDirs::localkdedir() const
{
    // Return the prefix to use for saving
    return prefixes.first();
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
    // try to find slashes. If there are some, we have to
    // create the subdir first
    int slash = filename.findRev('/')+1;
    if (!slash) // only one filename
	return inst->dirs()->saveLocation(type) + filename;

    // split path from filename
    QString dir = filename.left(slash);
    QString file = filename.mid(slash);
    return inst->dirs()->saveLocation(type, dir) + file;
}
