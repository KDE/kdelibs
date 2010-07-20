/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kang <taj@kde.org>
   Copyright (C) 1999,2007 Stephan Kulow <coolo@kde.org>
   Copyright (C) 1999 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2009 David Faure <faure@kde.org>

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
 * Generated: Thu Mar  5 16:05:28 EST 1998
 */

#include "kstandarddirs.h"
#include "kconfig.h"
#include "kconfiggroup.h"
#include "kdebug.h"
#include "kcomponentdata.h"
#include "kshell.h"
#include "kuser.h"
#include "kde_file.h"
#include "kkernel_win.h"
#include "kkernel_mac.h"
#include "klocale.h"

#include <config.h>
#include <config-prefix.h>
#include <config-kstandarddirs.h>

#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#ifdef Q_WS_WIN
#include <windows.h>
#ifdef _WIN32_WCE
#include <basetyps.h>
#endif
#ifdef Q_WS_WIN64
// FIXME: did not find a reliable way to fix with kdewin mingw header 
#define interface struct
#endif
#include <shlobj.h>
#include <QtCore/QVarLengthArray>
#endif

#include <QtCore/QMutex>
#include <QtCore/QRegExp>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>

class KStandardDirs::KStandardDirsPrivate
{
public:
    KStandardDirsPrivate(KStandardDirs* qq)
        : m_restrictionsActive(false),
          m_checkRestrictions(true),
          m_cacheMutex(QMutex::Recursive), // resourceDirs is recursive
          q(qq)
    { }

    bool hasDataRestrictions(const QString &relPath) const;
    QStringList resourceDirs(const char* type, const QString& subdirForRestrictions);
    void createSpecialResource(const char*);

    bool m_restrictionsActive : 1;
    bool m_checkRestrictions : 1;
    QMap<QByteArray, bool> m_restrictions;

    QStringList xdgdata_prefixes;
    QStringList xdgconf_prefixes;
    QStringList m_prefixes;

    // Directory dictionaries
    QMap<QByteArray, QStringList> m_absolutes;
    QMap<QByteArray, QStringList> m_relatives;

    // Caches (protected by mutex in const methods, cf ctor docu)
    QMap<QByteArray, QStringList> m_dircache;
    QMap<QByteArray, QString> m_savelocations;
    QMutex m_cacheMutex;

    KStandardDirs* q;
};

/* If you add a new resource type here, make sure to
 * 1) regenerate using "kdesdk/scripts/generate_string_table.pl types < tmpfile" with the data below in tmpfile.
 * 2) update the KStandardDirs class documentation
 * 3) update the list in kde-config.cpp

data
share/apps
html
share/doc/HTML
icon
share/icons
config
share/config
pixmap
share/pixmaps
apps
share/applnk
sound
share/sounds
locale
share/locale
services
share/kde4/services
servicetypes
share/kde4/servicetypes
mime
share/mimelnk
cgi
cgi-bin
wallpaper
share/wallpapers
templates
share/templates
exe
bin
module
%lib/kde4
qtplugins
%lib/kde4/plugins
kcfg
share/config.kcfg
emoticons
share/emoticons
xdgdata-apps
applications
xdgdata-icon
icons
xdgdata-pixmap
pixmaps
xdgdata-dirs
desktop-directories
xdgdata-mime
mime
xdgconf-menu
menus
xdgconf-autostart
autostart
*/

static const char types_string[] =
    "data\0"
    "share/apps\0"
    "html\0"
    "share/doc/HTML\0"
    "icon\0"
    "share/icons\0"
    "config\0"
    "share/config\0"
    "pixmap\0"
    "share/pixmaps\0"
    "apps\0"
    "share/applnk\0"
    "sound\0"
    "share/sounds\0"
    "locale\0"
    "share/locale\0"
    "services\0"
    "share/kde4/services\0"
    "servicetypes\0"
    "share/kde4/servicetypes\0"
    "mime\0"
    "share/mimelnk\0"
    "cgi\0"
    "cgi-bin\0"
    "wallpaper\0"
    "share/wallpapers\0"
    "templates\0"
    "share/templates\0"
    "exe\0"
    "bin\0"
    "module\0"
    "%lib/kde4\0"
    "qtplugins\0"
    "%lib/kde4/plugins\0"
    "kcfg\0"
    "share/config.kcfg\0"
    "emoticons\0"
    "share/emoticons\0"
    "xdgdata-apps\0"
    "applications\0"
    "xdgdata-icon\0"
    "icons\0"
    "xdgdata-pixmap\0"
    "pixmaps\0"
    "xdgdata-dirs\0"
    "desktop-directories\0"
    "xdgdata-mime\0"
    "xdgconf-menu\0"
    "menus\0"
    "xdgconf-autostart\0"
    "autostart\0"
    "\0";

static const int types_indices[] = {
    0,    5,   16,   21,   36,   41,   53,   60,
    73,   80,   94,   99,  112,  118,  131,  138,
    151,  160,  180,  193,  217,  222,  236,  240,
    248,  258,  275,  285,  301,  305,  309,  316,
    326,  336,  354,  359,  377,  387,  403,  416,
    429,  442,  448,  463,  471,  484,  504,  217,
    517,  530,  536,  554,  -1
};

static int tokenize( QStringList& token, const QString& str,
                     const QString& delim );

KStandardDirs::KStandardDirs()
    : d(new KStandardDirsPrivate(this))
{
    addKDEDefaults();
}

KStandardDirs::~KStandardDirs()
{
    delete d;
}

bool KStandardDirs::isRestrictedResource(const char *type, const QString& relPath) const
{
    if (!d->m_restrictionsActive)
        return false;

    if (d->m_restrictions.value(type, false))
        return true;

    if (strcmp(type, "data")==0 && d->hasDataRestrictions(relPath))
        return true;

    return false;
}

bool KStandardDirs::KStandardDirsPrivate::hasDataRestrictions(const QString &relPath) const
{
    QString key;
    const int i = relPath.indexOf('/');
    if (i != -1)
        key = "data_" + relPath.left(i);
    else
        key = "data_" + relPath;

    return m_restrictions.value(key.toLatin1(), false);
}


QStringList KStandardDirs::allTypes() const
{
    QStringList list;
    for (int i = 0; types_indices[i] != -1; i += 2)
        list.append(QLatin1String(types_string + types_indices[i]));
    // Those are added manually by addKDEDefaults
    list.append("lib");
    //list.append("home"); // undocumented on purpose, said Waldo in r113855.

    // Those are handled by resourceDirs() itself
    list.append("socket");
    list.append("tmp");
    list.append("cache");
    // Those are handled by installPath()
    list.append("include");

    // If you add anything here, make sure kde-config.cpp has a description for it.

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

    if (!d->m_prefixes.contains(dir)) {
        priorityAdd(d->m_prefixes, dir, priority);
        d->m_dircache.clear();
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
        d->m_dircache.clear();
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
        d->m_dircache.clear();
    }
}

QString KStandardDirs::kfsstnd_prefixes()
{
    return d->m_prefixes.join(QString(QChar(KPATH_SEPARATOR)));
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
                                     const QString& relativename,
                                     bool priority )
{
    return addResourceType( type, 0, relativename, priority);
}

bool KStandardDirs::addResourceType( const char *type,
                                     const char *basetype,
                                     const QString& relativename,
                                     bool priority )
{
    if (relativename.isEmpty())
        return false;

    QString copy = relativename;
    if (basetype)
        copy = QString('%') + basetype + '/' + relativename;

    if (!copy.endsWith('/'))
        copy += '/';

    QStringList& rels = d->m_relatives[type]; // find or insert

    if (!rels.contains(copy)) {
        if (priority)
            rels.prepend(copy);
        else
            rels.append(copy);
        d->m_dircache.remove(type); // clean the cache
        return true;
    }
    return false;
}

bool KStandardDirs::addResourceDir( const char *type,
                                    const QString& absdir,
                                    bool priority)
{
    if (absdir.isEmpty() || !type)
      return false;
    // find or insert entry in the map
    QString copy = absdir;
    if (copy.at(copy.length() - 1) != '/')
        copy += '/';

    QStringList &paths = d->m_absolutes[type];
    if (!paths.contains(copy)) {
        if (priority)
            paths.prepend(copy);
        else
            paths.append(copy);
        d->m_dircache.remove(type); // clean the cache
        return true;
    }
    return false;
}

QString KStandardDirs::findResource( const char *type,
                                     const QString& _filename ) const
{
    if (!QDir::isRelativePath(_filename))
      return !KGlobal::hasLocale() ? _filename // absolute dirs are absolute dirs, right? :-/
                                   : KGlobal::locale()->localizedFilePath(_filename); // -- almost.

#if 0
    kDebug(180) << "Find resource: " << type;
    for (QStringList::ConstIterator pit = m_prefixes.begin();
         pit != m_prefixes.end();
         ++pit)
    {
        kDebug(180) << "Prefix: " << *pit;
    }
#endif

    QString filename(_filename);
#ifdef Q_OS_WIN
    if(strcmp(type, "exe") == 0) {
      if(!filename.endsWith(QLatin1String(".exe")))
        filename += QLatin1String(".exe");
    }
#endif
    const QString dir = findResourceDir(type, filename);
    if (dir.isEmpty())
      return dir;
    else
      return !KGlobal::hasLocale() ? dir + filename
                                   : KGlobal::locale()->localizedFilePath(dir + filename);
}

static quint32 updateHash(const QString &file, quint32 hash)
{
    KDE_struct_stat buff;
    if ((KDE::access(file, R_OK) == 0) && (KDE::stat(file, &buff) == 0) && (S_ISREG(buff.st_mode))) {
        hash = hash + static_cast<quint32>(buff.st_ctime);
    }
    return hash;
}

quint32 KStandardDirs::calcResourceHash( const char *type,
                                         const QString& filename,
                                         SearchOptions options ) const
{
    quint32 hash = 0;

    if (!QDir::isRelativePath(filename))
    {
        // absolute dirs are absolute dirs, right? :-/
        return updateHash(filename, hash);
    }
    QStringList candidates = d->resourceDirs(type, filename);

    foreach ( const QString& candidate, candidates )
    {
        hash = updateHash(candidate + filename, hash);
        if (  !( options & Recursive ) && hash ) {
            return hash;
        }
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
            if (reldir.endsWith('/'))
                list.append(reldir);
            else
                list.append(reldir+'/');
        }
        return list;
    }

    const QStringList candidates = d->resourceDirs(type, reldir);

    for (QStringList::ConstIterator it = candidates.begin();
         it != candidates.end(); ++it) {
        testdir.setPath(*it + reldir);
        if (testdir.exists())
            list.append(testdir.absolutePath() + '/');
    }

    return list;
}

QString KStandardDirs::findResourceDir( const char *type,
                                        const QString& _filename) const
{
#ifndef NDEBUG
    if (_filename.isEmpty()) {
        kWarning() << "filename for type " << type << " in KStandardDirs::findResourceDir is not supposed to be empty!!";
        return QString();
    }
#endif

    QString filename(_filename);
#ifdef Q_OS_WIN
    if(strcmp(type, "exe") == 0) {
      if(!filename.endsWith(QLatin1String(".exe")))
        filename += QLatin1String(".exe");
    }
#endif
    const QStringList candidates = d->resourceDirs(type, filename);

    for (QStringList::ConstIterator it = candidates.begin();
         it != candidates.end(); ++it) {
        if (exists(*it + filename)) {
            return *it;
        }
    }

#ifndef NDEBUG
    if(false && strcmp(type, "locale"))
        kDebug(180) << "KStdDirs::findResDir(): can't find \"" << filename << "\" in type \"" << type << "\".";
#endif

    return QString();
}

bool KStandardDirs::exists(const QString &fullPath)
{
#ifdef Q_OS_WIN
    // access() and stat() give a stupid error message to the user
    // if the path is not accessible at all (e.g. no disk in A:/ and
    // we do stat("A:/.directory")
    if (fullPath.endsWith('/'))
        return QDir(fullPath).exists();
    return QFileInfo(fullPath).exists();
#else
    KDE_struct_stat buff;
    QByteArray cFullPath = QFile::encodeName(fullPath);
    if (access(cFullPath, R_OK) == 0 && KDE_stat( cFullPath, &buff ) == 0) {
        if (!fullPath.endsWith('/')) {
            if (S_ISREG( buff.st_mode ))
                return true;
        } else
            if (S_ISDIR( buff.st_mode ))
                return true;
    }
    return false;
#endif
}

static void lookupDirectory(const QString& path, const QString &relPart,
                            const QRegExp &regexp,
                            QStringList& list,
                            QStringList& relList,
                            bool recursive, bool unique)
{
    const QString pattern = regexp.pattern();
    if (recursive || pattern.contains('?') || pattern.contains('*'))
    {
        if (path.isEmpty()) //for sanity
            return;
#ifdef Q_WS_WIN
        QString path_ = path + QLatin1String( "*.*" );
        WIN32_FIND_DATA findData;
        HANDLE hFile = FindFirstFile( (LPWSTR)path_.utf16(), &findData );
        if( hFile == INVALID_HANDLE_VALUE )
            return;
        do {
            const int len = wcslen( findData.cFileName );
            if (!( findData.cFileName[0] == '.' &&
                   findData.cFileName[1] == '\0' ) &&
                !( findData.cFileName[0] == '.' &&
                   findData.cFileName[1] == '.' &&
                   findData.cFileName[2] == '\0' ) &&
                 ( findData.cFileName[len-1] != '~' ) ) {
                QString fn = QString::fromUtf16( (const unsigned short*)findData.cFileName );
                if (!recursive && !regexp.exactMatch(fn))
                    continue; // No match
                QString pathfn = path + fn;
                bool bIsDir = ( ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY );
                if ( recursive ) {
                    if ( bIsDir ) {
                        lookupDirectory(pathfn + '/', relPart + fn + '/', regexp, list, relList, recursive, unique);
                    }
                    if (!regexp.exactMatch(fn))
                        continue; // No match
                }
                if ( !bIsDir )
                {
                    if ( !unique || !relList.contains(relPart + fn) )
                    {
                        list.append( pathfn );
                        relList.append( relPart + fn );
                    }
                }
            }
        } while( FindNextFile( hFile, &findData ) != 0 );
        FindClose( hFile );
#else
        // We look for a set of files.
        DIR *dp = opendir( QFile::encodeName(path));
        if (!dp)
            return;

        assert(path.at(path.length() - 1) == '/');

        struct dirent *ep;

        while( ( ep = readdir( dp ) ) != 0L )
        {
            QString fn( QFile::decodeName(ep->d_name));
            if (fn == "." || fn == ".." || fn.at(fn.length() - 1).toLatin1() == '~')
                continue;

            if (!recursive && !regexp.exactMatch(fn))
                continue; // No match

            bool isDir;
            bool isReg;

            QString pathfn = path + fn;
#ifdef HAVE_DIRENT_D_TYPE
            isDir = ep->d_type == DT_DIR;
            isReg = ep->d_type == DT_REG;

            if (ep->d_type == DT_UNKNOWN || ep->d_type == DT_LNK)
#endif
            {
                KDE_struct_stat buff;
                if ( KDE::stat( pathfn, &buff ) != 0 ) {
                    kDebug(180) << "Error stat'ing " << pathfn << " : " << perror;
                    continue; // Couldn't stat (e.g. no read permissions)
                }
                isReg = S_ISREG (buff.st_mode);
                isDir = S_ISDIR (buff.st_mode);
            }

            if ( recursive ) {
                if ( isDir ) {
                    lookupDirectory(pathfn + '/', relPart + fn + '/', regexp, list, relList, recursive, unique);
                }
                if (!regexp.exactMatch(fn))
                    continue; // No match
            }
            if ( isReg )
            {
                if (!unique || !relList.contains(relPart + fn))
                {
                    list.append( pathfn );
                    relList.append( relPart + fn );
                }
            }
        }
        closedir( dp );
#endif
    }
    else
    {
        // We look for a single file.
        QString fn = pattern;
        QString pathfn = path + fn;
        KDE_struct_stat buff;
        if ( KDE::stat( pathfn, &buff ) != 0 )
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
        if (recursive)
            Q_ASSERT(prefix != "/"); // we don't want to recursively list the whole disk!
        lookupDirectory(prefix, relPart, regexp, list,
                        relList, recursive, unique);
        return;
    }
    QString path;
    QString rest;

    int slash = relpath.indexOf('/');
    if (slash < 0)
        rest = relpath.left(relpath.length() - 1);
    else {
        path = relpath.left(slash);
        rest = relpath.mid(slash + 1);
    }

    if (prefix.isEmpty()) //for sanity
        return;
#ifndef Q_WS_WIN
	// what does this assert check ?
    assert(prefix.at(prefix.length() - 1) == '/');
#endif
    if (path.contains('*') || path.contains('?')) {

        QRegExp pathExp(path, Qt::CaseSensitive, QRegExp::Wildcard);

#ifdef Q_WS_WIN
        QString prefix_ = prefix + QLatin1String( "*.*" );
        WIN32_FIND_DATA findData;
        HANDLE hFile = FindFirstFile( (LPWSTR)prefix_.utf16(), &findData );
        if( hFile == INVALID_HANDLE_VALUE )
            return;
        do {
            const int len = wcslen( findData.cFileName );
            if (!( findData.cFileName[0] == '.' &&
                   findData.cFileName[1] == '\0' ) &&
                !( findData.cFileName[0] == '.' &&
                   findData.cFileName[1] == '.' &&
                   findData.cFileName[2] == '\0' ) &&
                 ( findData.cFileName[len-1] != '~' ) ) {
                const QString fn = QString::fromUtf16( (const unsigned short*)findData.cFileName );
                if ( !pathExp.exactMatch(fn) )
                    continue; // No match
                if ( ( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == FILE_ATTRIBUTE_DIRECTORY )
                    lookupPrefix(prefix + fn + '/', rest, relPart + fn + '/',
                                 regexp, list, relList, recursive, unique);
            }
        } while( FindNextFile( hFile, &findData ) != 0 );
        FindClose( hFile );
#else
        DIR *dp = opendir( QFile::encodeName(prefix) );
        if (!dp) {
            return;
        }

        struct dirent *ep;

        while( ( ep = readdir( dp ) ) != 0L )
        {
            QString fn( QFile::decodeName(ep->d_name));
            if (fn == "." || fn == ".." || fn.at(fn.length() - 1) == '~')
                continue;

            if ( !pathExp.exactMatch(fn) )
                continue; // No match
            QString rfn = relPart+fn;
            fn = prefix + fn;

            bool isDir;

#ifdef HAVE_DIRENT_D_TYPE
            isDir = ep->d_type == DT_DIR;

            if (ep->d_type == DT_UNKNOWN || ep->d_type == DT_LNK)
#endif
            {
                QString pathfn = path + fn;
                KDE_struct_stat buff;
                if ( KDE::stat( fn, &buff ) != 0 ) {
                    kDebug(180) << "Error stat'ing " << fn << " : " << perror;
                    continue; // Couldn't stat (e.g. no read permissions)
                }
                isDir = S_ISDIR (buff.st_mode);
            }
            if ( isDir )
                lookupPrefix(fn + '/', rest, rfn + '/', regexp, list, relList, recursive, unique);
        }

        closedir( dp );
#endif
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
                                 SearchOptions options,
                                 QStringList &relList) const
{
    QString filterPath;
    QString filterFile;

    if ( !filter.isEmpty() )
    {
        int slash = filter.lastIndexOf('/');
        if (slash < 0) {
            filterFile = filter;
        } else {
            filterPath = filter.left(slash + 1);
            filterFile = filter.mid(slash + 1);
        }
    }

    QStringList candidates;
    if ( !QDir::isRelativePath(filter) ) // absolute path
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
        candidates = d->resourceDirs(type, filter);
    }

    if (filterFile.isEmpty()) {
        filterFile = '*';
    }

    QRegExp regExp(filterFile, Qt::CaseSensitive, QRegExp::Wildcard);

    QStringList list;
    foreach ( const QString& candidate, candidates )
    {
        lookupPrefix(candidate, filterPath, "", regExp, list,
                     relList, options & Recursive, options & NoDuplicates);
    }

    return list;
}

QStringList
KStandardDirs::findAllResources( const char *type,
                                 const QString& filter,
                                 SearchOptions options ) const
{
    QStringList relList;
    return findAllResources(type, filter, options, relList);
}

// ####### KDE4: should this be removed, in favor of QDir::canonicalPath()?
// aseigo: QDir::canonicalPath returns QString() if the dir doesn't exist
//         and this method is often used with the expectation for it to work
//         even if the directory doesn't exist. so ... no, we can't drop this
//         yet
QString
KStandardDirs::realPath(const QString &dirname)
{
#ifdef Q_WS_WIN
    const QString strRet = realFilePath(dirname);
    if ( !strRet.endsWith('/') )
        return strRet + '/';
    return strRet;
#else
    char realpath_buffer[MAXPATHLEN + 1];
    memset(realpath_buffer, 0, MAXPATHLEN + 1);

    /* If the path contains symlinks, get the real name */
    if (realpath( QFile::encodeName(dirname).constData(), realpath_buffer) != 0) {
        // success, use result from realpath
        int len = strlen(realpath_buffer);
        realpath_buffer[len] = '/';
        realpath_buffer[len+1] = 0;
        return QFile::decodeName(realpath_buffer);
    }

    if ( !dirname.endsWith('/') )
        return dirname + '/';
    return dirname;
#endif
}

// ####### KDE4: should this be removed, in favor of QDir::canonicalPath()?
// aseigo: QDir::canonicalPath returns QString() if the dir doesn't exist
//         and this method is often used with the expectation for it to work
//         even if the directory doesn't exist. so ... no, we can't drop this
//         yet
QString
KStandardDirs::realFilePath(const QString &filename)
{
#ifdef Q_WS_WIN
    LPCWSTR lpIn = (LPCWSTR)filename.utf16();
    QVarLengthArray<WCHAR, MAX_PATH> buf(MAX_PATH);
    DWORD len = GetFullPathNameW(lpIn, buf.size(), buf.data(), NULL);
    if (len > (DWORD)buf.size()) {
        buf.resize(len);
        len = GetFullPathNameW(lpIn, buf.size(), buf.data(), NULL);
    }
    if (len == 0)
        return QString();
    return QString::fromUtf16((const unsigned short*)buf.data()).replace('\\','/').toLower();
#else
    char realpath_buffer[MAXPATHLEN + 1];
    memset(realpath_buffer, 0, MAXPATHLEN + 1);

    /* If the path contains symlinks, get the real name */
    if (realpath( QFile::encodeName(filename).constData(), realpath_buffer) != 0) {
        // success, use result from realpath
        return QFile::decodeName(realpath_buffer);
    }

    return filename;
#endif
}


void KStandardDirs::KStandardDirsPrivate::createSpecialResource(const char *type)
{
    char hostname[256];
    hostname[0] = 0;
    gethostname(hostname, 255);
    const QString localkdedir = m_prefixes.first();
    QString dir = QString("%1%2-%3").arg(localkdedir).arg(type).arg(hostname);
    char link[1024];
    link[1023] = 0;
    int result = readlink(QFile::encodeName(dir).constData(), link, 1023);
    bool relink = (result == -1) && (errno == ENOENT);
    if (result > 0)
    {
        link[result] = 0;
        if (!QDir::isRelativePath(link))
        {
            KDE_struct_stat stat_buf;
            int res = KDE::lstat(link, &stat_buf);
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
        QString srv = findExe(QLatin1String("lnusertemp"), installPath("libexec"));
        if (srv.isEmpty())
            srv = findExe(QLatin1String("lnusertemp"));
        if (!srv.isEmpty())
        {
            if (system(QFile::encodeName(srv) + ' ' + type) == -1) {
                fprintf(stderr, "Error: unable to launch lnusertemp command" );
            }
            result = readlink(QFile::encodeName(dir).constData(), link, 1023);
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
    q->addResourceDir(type, dir+'/', false);
}

QStringList KStandardDirs::resourceDirs(const char *type) const
{
    return d->resourceDirs(type, QString());
}

QStringList KStandardDirs::KStandardDirsPrivate::resourceDirs(const char* type, const QString& subdirForRestrictions)
{
    QMutexLocker lock(&m_cacheMutex);
    const bool dataRestrictionActive = m_restrictionsActive
                                       && (strcmp(type, "data") == 0)
                                       && hasDataRestrictions(subdirForRestrictions);

    QMap<QByteArray, QStringList>::const_iterator dirCacheIt = m_dircache.constFind(type);

    QStringList candidates;

    if (dirCacheIt != m_dircache.constEnd() && !dataRestrictionActive) {
        //qDebug() << this << "resourceDirs(" << type << "), in cache already";
        candidates = *dirCacheIt;
    }
    else // filling cache
    {
        //qDebug() << this << "resourceDirs(" << type << "), not in cache";
        if (strcmp(type, "socket") == 0)
            createSpecialResource(type);
        else if (strcmp(type, "tmp") == 0)
            createSpecialResource(type);
        else if (strcmp(type, "cache") == 0)
            createSpecialResource(type);

        QDir testdir;

        bool restrictionActive = false;
        if (m_restrictionsActive) {
            if (dataRestrictionActive)
                restrictionActive = true;
            if (m_restrictions.value("all", false))
                restrictionActive = true;
            else if (m_restrictions.value(type, false))
                restrictionActive = true;
        }

        QStringList dirs;
        dirs = m_relatives.value(type);
        const QString typeInstallPath = installPath(type); // could be empty
// better #ifdef incasesensitive_filesystem
#ifdef Q_WS_WIN
        const QString installdir = typeInstallPath.isEmpty() ? QString() : realPath(typeInstallPath).toLower();
        const QString installprefix = installPath("kdedir").toLower();
#else
        const QString installdir = typeInstallPath.isEmpty() ? QString() : realPath(typeInstallPath);
        const QString installprefix = installPath("kdedir");
#endif
        if (!dirs.isEmpty())
        {
            bool local = true;

            for (QStringList::ConstIterator it = dirs.constBegin();
                 it != dirs.constEnd(); ++it)
            {
                if ( (*it).startsWith('%'))
                {
                    // grab the "data" from "%data/apps"
                    const int pos = (*it).indexOf('/');
                    QString rel = (*it).mid(1, pos - 1);
                    QString rest = (*it).mid(pos + 1);
                    const QStringList basedirs = resourceDirs(rel.toUtf8().constData(), subdirForRestrictions);
                    for (QStringList::ConstIterator it2 = basedirs.begin();
                         it2 != basedirs.end(); ++it2)
                    {
#ifdef Q_WS_WIN
                        const QString path = realPath( *it2 + rest ).toLower();
#else
                        const QString path = realPath( *it2 + rest );
#endif
                        testdir.setPath(path);
                        if ((local || testdir.exists()) && !candidates.contains(path))
                            candidates.append(path);
                        local = false;
                    }
                }
            }

            const QStringList *prefixList = 0;
            if (strncmp(type, "xdgdata-", 8) == 0)
                prefixList = &(xdgdata_prefixes);
            else if (strncmp(type, "xdgconf-", 8) == 0)
                prefixList = &(xdgconf_prefixes);
            else
                prefixList = &m_prefixes;

            for (QStringList::ConstIterator pit = prefixList->begin();
                 pit != prefixList->end();
                 ++pit)
            {
	        if((*pit)!=installprefix||installdir.isEmpty())
	        {
                    for (QStringList::ConstIterator it = dirs.constBegin();
                         it != dirs.constEnd(); ++it)
                    {
                        if ( (*it).startsWith('%'))
                            continue;
#ifdef Q_WS_WIN
                        const QString path = realPath( *pit + *it ).toLower();
#else
                        const QString path = realPath( *pit + *it );
#endif
                        testdir.setPath(path);
                        if (local && restrictionActive)
                            continue;
                        if ((local || testdir.exists()) && !candidates.contains(path))
                            candidates.append(path);
                    }
                    local = false;
                }
	        else
	        {
                    // we have a custom install path, so use this instead of <installprefix>/<relative dir>
	            testdir.setPath(installdir);
                    if(testdir.exists() && ! candidates.contains(installdir))
                        candidates.append(installdir);
	        }
	    }
        }

        // make sure we find the path where it's installed
        if (!installdir.isEmpty()) {
            bool ok = true;
            foreach (const QString &s, candidates) {
                if (installdir.startsWith(s)) {
                    ok = false;
                    break;
                }
            }
            if (ok)
                candidates.append(installdir);
        }

        dirs = m_absolutes.value(type);
        if (!dirs.isEmpty())
            for (QStringList::ConstIterator it = dirs.constBegin();
                 it != dirs.constEnd(); ++it)
            {
                testdir.setPath(*it);
                if (testdir.exists()) {
#ifdef Q_WS_WIN
                    const QString filename = realPath( *it ).toLower();
#else
                    const QString filename = realPath( *it );
#endif
                    if (!candidates.contains(filename)) {
                        candidates.append(filename);
                    }
                }
            }

        // Insert result into the cache for next time.
        // Exception: data_subdir restrictions are per-subdir, so we can't store such results
        if (!dataRestrictionActive) {
            //kDebug() << this << "Inserting" << type << candidates << "into dircache";
            m_dircache.insert(type, candidates);
        }
    }

#if 0
    kDebug(180) << "found dirs for resource" << type << ":" << candidates;
#endif

    return candidates;
}

#ifdef Q_OS_WIN
static QStringList executableExtensions()
{
    QStringList ret = QString::fromLocal8Bit(qgetenv("PATHEXT")).split(';');
    if (!ret.contains(".exe", Qt::CaseInsensitive)) {
        // If %PATHEXT% does not contain .exe, it is either empty, malformed, or distorted in ways that we cannot support, anyway.
        ret.clear();
        ret << ".exe" << ".com" << ".bat" << ".cmd";
    }
    return ret;
}
#endif

QStringList KStandardDirs::systemPaths( const QString& pstr )
{
    QStringList tokens;
    QString p = pstr;

    if( p.isEmpty() )
    {
        p = QString::fromLocal8Bit( qgetenv( "PATH" ) );
    }

    QString delimiters(QChar(KPATH_SEPARATOR));
    delimiters += "\b";
    tokenize( tokens, p, delimiters );

    QStringList exePaths;

    // split path using : or \b as delimiters
    for( int i = 0; i < tokens.count(); i++ )
    {
        exePaths << KShell::tildeExpand( tokens[ i ] );
    }

    return exePaths;
}

#ifdef Q_WS_MAC
static QString getBundle( const QString& path, bool ignore )
{
    kDebug(180) << "getBundle(" << path << ", " << ignore << ") called";
    QFileInfo info;
    QString bundle = path;
    bundle += ".app/Contents/MacOS/" + bundle.section('/', -1);
    info.setFile( bundle );
    FILE *file;
    if (file = fopen(info.absoluteFilePath().toUtf8().constData(), "r")) {
        fclose(file);
        struct stat _stat;
        if ((stat(info.absoluteFilePath().toUtf8().constData(), &_stat)) < 0) {
            return QString();
        }
        if ( ignore || (_stat.st_mode & S_IXUSR) ) {
            if ( ((_stat.st_mode & S_IFMT) == S_IFREG) || ((_stat.st_mode & S_IFMT) == S_IFLNK) ) {
                kDebug(180) << "getBundle(): returning " << bundle;
                return bundle;
            }
        }
    }
    return QString();
}
#endif

static QString checkExecutable( const QString& path, bool ignoreExecBit )
{
#ifdef Q_WS_MAC
    QString bundle = getBundle( path, ignoreExecBit );
    if ( !bundle.isEmpty() ) {
        //kDebug(180) << "findExe(): returning " << bundle;
        return bundle;
    }
#endif
    QFileInfo info( path );
    QFileInfo orig = info;
#if defined(Q_OS_DARWIN) || defined(Q_OS_MAC)
    FILE *file;
    if (file = fopen(orig.absoluteFilePath().toUtf8().constData(), "r")) {
        fclose(file);
        struct stat _stat;
        if ((stat(orig.absoluteFilePath().toUtf8().constData(), &_stat)) < 0) {
            return QString();
        }
        if ( ignoreExecBit || (_stat.st_mode & S_IXUSR) ) {
            if ( ((_stat.st_mode & S_IFMT) == S_IFREG) || ((_stat.st_mode & S_IFMT) == S_IFLNK) ) {
                orig.makeAbsolute();
                return orig.filePath();
            }
        }
    }
    return QString();
#else
    if( info.exists() && info.isSymLink() )
        info = QFileInfo( info.canonicalFilePath() );
    if( info.exists() && ( ignoreExecBit || info.isExecutable() ) && info.isFile() ) {
        // return absolute path, but without symlinks resolved in order to prevent
        // problems with executables that work differently depending on name they are
        // run as (for example gunzip)
        orig.makeAbsolute();
        return orig.filePath();
    }
    //kDebug(180) << "checkExecutable(): failed, returning empty string";
    return QString();
#endif
}

QString KStandardDirs::findExe( const QString& appname,
                                const QString& pstr,
                                SearchOptions options )
{
    //kDebug(180) << "findExe(" << appname << ", pstr, " << ignoreExecBit << ") called";

#ifdef Q_OS_WIN
    QStringList executable_extensions = executableExtensions();
    if (!executable_extensions.contains(appname.section('.', -1, -1, QString::SectionIncludeLeadingSep), Qt::CaseInsensitive)) {
        QString found_exe;
        foreach (const QString& extension, executable_extensions) {
            found_exe = findExe(appname + extension, pstr, options);
            if (!found_exe.isEmpty()) {
                return found_exe;
            }
        }
        return QString();
    }
#endif
    QFileInfo info;

    // absolute or relative path?
    if (appname.contains(QDir::separator()))
    {
        //kDebug(180) << "findExe(): absolute path given";
        QString path = checkExecutable(appname, options & IgnoreExecBit);
        return path;
    }

    //kDebug(180) << "findExe(): relative path given";

    // Look in the default bin and libexec dirs. Maybe we should use the "exe" resource instead?

    QString p = installPath("libexec") + appname;
    QString result = checkExecutable(p, options & IgnoreExecBit);
    if (!result.isEmpty()) {
        //kDebug(180) << "findExe(): returning " << result;
        return result;
    }

    p = installPath("exe") + appname;
    result = checkExecutable(p, options & IgnoreExecBit);
    if (!result.isEmpty()) {
        //kDebug(180) << "findExe(): returning " << result;
        return result;
    }

    //kDebug(180) << "findExe(): checking system paths";
    const QStringList exePaths = systemPaths( pstr );
    for (QStringList::ConstIterator it = exePaths.begin(); it != exePaths.end(); ++it)
    {
        p = (*it) + '/';
        p += appname;

        // Check for executable in this tokenized path
        result = checkExecutable(p, options & IgnoreExecBit);
        if (!result.isEmpty()) {
            //kDebug(180) << "findExe(): returning " << result;
            return result;
        }
    }

    // If we reach here, the executable wasn't found.
    // So return empty string.

    //kDebug(180) << "findExe(): failed, nothing matched";
    return QString();
}

int KStandardDirs::findAllExe( QStringList& list, const QString& appname,
                               const QString& pstr, SearchOptions options )
{
#ifdef Q_OS_WIN
    QStringList executable_extensions = executableExtensions();
    if (!executable_extensions.contains(appname.section('.', -1, -1, QString::SectionIncludeLeadingSep), Qt::CaseInsensitive)) {
        int total = 0;
        foreach (const QString& extension, executable_extensions) {
            total += findAllExe (list, appname + extension, pstr, options);
        }
        return total;
    }
#endif
    QFileInfo info;
    QString p;
    list.clear();

    const QStringList exePaths = systemPaths( pstr );
    for (QStringList::ConstIterator it = exePaths.begin(); it != exePaths.end(); ++it)
    {
        p = (*it) + '/';
        p += appname;

#ifdef Q_WS_MAC
        QString bundle = getBundle( p, (options & IgnoreExecBit) );
        if ( !bundle.isEmpty() ) {
            //kDebug(180) << "findExe(): returning " << bundle;
            list.append( bundle );
        }
#endif

        info.setFile( p );

        if( info.exists() && ( ( options & IgnoreExecBit ) || info.isExecutable())
            && info.isFile() ) {
            list.append( p );
        }
    }

    return list.count();
}

static inline QString equalizePath(QString &str)
{
#ifdef Q_WS_WIN
    // filter pathes through QFileInfo to have always
    // the same case for drive letters
    QFileInfo f(str);
    if (f.isAbsolute())
        return f.absoluteFilePath();
    else
#endif
        return str;
}

static int tokenize( QStringList& tokens, const QString& str,
                     const QString& delim )
{
    int len = str.length();
    QString token;

    for( int index = 0; index < len; index++)
    {
        if ( delim.contains( str[ index ] ) )
        {
            tokens.append( equalizePath(token) );
            token.clear();
        }
        else
        {
            token += str[ index ];
        }
    }
    if ( !token.isEmpty() )
    {
        tokens.append( equalizePath(token) );
    }

    return tokens.count();
}

QString KStandardDirs::kde_default(const char *type)
{
    return QString('%') + type + '/';
}

QString KStandardDirs::saveLocation(const char *type,
                                    const QString& suffix,
                                    bool create) const
{
    QMutexLocker lock(&d->m_cacheMutex);
    QString path = d->m_savelocations.value(type);
    if (path.isEmpty())
    {
        QStringList dirs = d->m_relatives.value(type);
        if (dirs.isEmpty() && (
                (strcmp(type, "socket") == 0) ||
                (strcmp(type, "tmp") == 0) ||
                (strcmp(type, "cache") == 0) ))
        {
            (void) resourceDirs(type); // Generate socket|tmp|cache resource.
            dirs = d->m_relatives.value(type); // Search again.
        }
        if (!dirs.isEmpty())
        {
            path = dirs.last();

            if ( path.startsWith('%'))
            {
                // grab the "data" from "%data/apps"
                const int pos = path.indexOf('/');
                QString rel = path.mid(1, pos - 1);
                QString rest = path.mid(pos + 1);
                QString basepath = saveLocation(rel.toUtf8().constData());
                path = basepath + rest;
            } else

                // Check for existence of typed directory + suffix
                if (strncmp(type, "xdgdata-", 8) == 0) {
                    path = realPath( localxdgdatadir() + path ) ;
                } else if (strncmp(type, "xdgconf-", 8) == 0) {
                    path = realPath( localxdgconfdir() + path );
                } else {
                    path = realPath( localkdedir() + path );
                }
        }
        else {
            dirs = d->m_absolutes.value(type);
            if (dirs.isEmpty()) {
                qFatal("KStandardDirs: The resource type %s is not registered", type);
            }
            path = realPath(dirs.last());
        }

        d->m_savelocations.insert(type, path.endsWith('/') ? path : path + '/');
    }
    QString fullPath = path + suffix;

    KDE_struct_stat st;
    if (KDE::stat(fullPath, &st) != 0 || !(S_ISDIR(st.st_mode))) {
        if(!create) {
#ifndef NDEBUG
            // Too much noise from kbuildsycoca4 -- it's fine if this happens from KConfig
            // when parsing global files without a local equivalent.
            //kDebug(180) << QString("save location %1 doesn't exist").arg(fullPath);
#endif
            return fullPath;
        }
        if(!makeDir(fullPath, 0700)) {
            return fullPath;
        }
        d->m_dircache.remove(type);
    }
    if (!fullPath.endsWith('/'))
        fullPath += '/';
    return fullPath;
}

// KDE5: make the method const
QString KStandardDirs::relativeLocation(const char *type, const QString &absPath)
{
    QString fullPath = absPath;
    int i = absPath.lastIndexOf('/');
    if (i != -1) {
        fullPath = realFilePath(absPath); // Normalize
    }

    const QStringList candidates = resourceDirs(type);

    for (QStringList::ConstIterator it = candidates.begin();
         it != candidates.end(); ++it) {
        if (fullPath.startsWith(*it)) {
            return fullPath.mid((*it).length());
        }
    }
    return absPath;
}


bool KStandardDirs::makeDir(const QString& dir, int mode)
{
    // we want an absolute path
    if (QDir::isRelativePath(dir))
        return false;

#ifdef Q_WS_WIN
    return QDir().mkpath(dir);
#else
    QString target = dir;
    uint len = target.length();

    // append trailing slash if missing
    if (dir.at(len - 1) != '/')
        target += '/';

    QString base;
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

            if (KDE_mkdir(baseEncoded, static_cast<mode_t>(mode)) != 0) {
                baseEncoded.prepend( "trying to create local folder " );
                perror(baseEncoded.constData());
                return false; // Couldn't create it :-(
            }
        }
        i = pos + 1;
    }
    return true;
#endif
}

static QString readEnvPath(const char *env)
{
    QByteArray c_path;
#ifndef _WIN32_WCE
    c_path = qgetenv(env);
    if (c_path.isEmpty())
        return QString();
#else
    bool ok;
    QString retval;
    getWin32RegistryValue(HKEY_LOCAL_MACHINE, "Software\\kde", retval, &ok);
    if (!ok){
        return QString();
    } else {
        c_path = retval.toAscii();
    }
#endif
    return QDir::fromNativeSeparators(QFile::decodeName(c_path));
}

#ifdef __linux__
static QString executablePrefix()
{
    char path_buffer[MAXPATHLEN + 1];
    path_buffer[MAXPATHLEN] = 0;
    int length = readlink ("/proc/self/exe", path_buffer, MAXPATHLEN);
    if (length == -1)
        return QString();

    path_buffer[length] = '\0';

    QString path = QFile::decodeName(path_buffer);

    if(path.isEmpty())
        return QString();

    int pos = path.lastIndexOf('/'); // Skip filename
    if(pos <= 0)
        return QString();
    pos = path.lastIndexOf('/', pos - 1); // Skip last directory
    if(pos <= 0)
        return QString();

    return path.left(pos);
}
#endif

void KStandardDirs::addResourcesFrom_krcdirs()
{
    QString localFile = QDir::currentPath() + "/.krcdirs";
    if (!QFile::exists(localFile))
        return;

    QSettings iniFile(localFile, QSettings::IniFormat);
    iniFile.beginGroup("KStandardDirs");
    const QStringList resources = iniFile.allKeys();
    foreach(const QString &key, resources)
    {
        QDir path(iniFile.value(key).toString());
        if (!path.exists())
            continue;

        if(path.makeAbsolute())
            addResourceDir(key.toAscii(), path.path(), false);
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
    kdedirList.append(installPath("kdedir"));

    QString execPrefix(EXEC_INSTALL_PREFIX);
    if (!execPrefix.isEmpty() && !kdedirList.contains(execPrefix))
        kdedirList.append(execPrefix);
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
#if defined(Q_WS_MACX)
        localKdeDir =  QDir::homePath() + QLatin1String("/Library/Preferences/KDE/");
#elif defined(Q_WS_WIN)
#ifndef _WIN32_WCE
        WCHAR wPath[MAX_PATH+1];
        if ( SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, wPath) == S_OK) {
          localKdeDir = QDir::fromNativeSeparators(QString::fromUtf16((const ushort *) wPath)) + QLatin1Char('/') + KDE_DEFAULT_HOME + QLatin1Char('/');
        } else {
#endif
          localKdeDir =  QDir::homePath() + QLatin1Char('/') + KDE_DEFAULT_HOME + QLatin1Char('/');
#ifndef _WIN32_WCE  
        }
#endif
#else
        localKdeDir =  QDir::homePath() + QLatin1Char('/') + KDE_DEFAULT_HOME + QLatin1Char('/');
#endif
    }

    if (localKdeDir != "-/")
    {
        localKdeDir = KShell::tildeExpand(localKdeDir);
        addPrefix(localKdeDir);
    }

#ifdef Q_WS_MACX
    // Adds the "Contents" directory of the current application bundle to
    // the search path. This way bundled resources can be found.
    QDir bundleDir(mac_app_filename());
    if (bundleDir.dirName() == "MacOS") { // just to be sure we're in a bundle
        bundleDir.cdUp();
        // now dirName should be "Contents". In there we can find our normal
        // dir-structure, beginning with "share"
        addPrefix(bundleDir.absolutePath());
    }
#endif

    QStringList::ConstIterator end(kdedirList.end());
    for (QStringList::ConstIterator it = kdedirList.constBegin();
         it != kdedirList.constEnd(); ++it)
    {
        const QString dir = KShell::tildeExpand(*it);
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
        xdgdirList.append(installPath("kdedir") + "etc/xdg");
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
#ifdef Q_WS_MACX
        localXdgDir =  QDir::homePath() + "/Library/Preferences/XDG/";
#else
        localXdgDir =  QDir::homePath() + "/.config/";
#endif
    }

    localXdgDir = KShell::tildeExpand(localXdgDir);
    addXdgConfigPrefix(localXdgDir);

    for (QStringList::ConstIterator it = xdgdirList.constBegin();
         it != xdgdirList.constEnd(); ++it)
    {
        QString dir = KShell::tildeExpand(*it);
        addXdgConfigPrefix(dir);
    }
    // end XDG_CONFIG_XXX

    // begin XDG_DATA_XXX
    QStringList kdedirDataDirs;
    for (QStringList::ConstIterator it = kdedirList.constBegin();
         it != kdedirList.constEnd(); ++it) {
        QString dir = *it;
        if (!dir.endsWith('/'))
            dir += '/';
        kdedirDataDirs.append(dir+"share/");
    }

    xdgdirs = readEnvPath("XDG_DATA_DIRS");
    if (!xdgdirs.isEmpty()) {
        tokenize(xdgdirList, xdgdirs, QString(QChar(KPATH_SEPARATOR)));
        // Ensure the kdedirDataDirs are in there too,
        // otherwise resourceDirs() will add kdedir/share/applications/kde4
        // as returned by installPath(), and that's incorrect.
        Q_FOREACH(const QString& dir, kdedirDataDirs) {
            if (!xdgdirList.contains(dir))
                xdgdirList.append(dir);
        }
    } else {
        xdgdirList = kdedirDataDirs;
#ifndef Q_WS_WIN
        xdgdirList.append("/usr/local/share/");
        xdgdirList.append("/usr/share/");
#endif
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

    for (QStringList::ConstIterator it = xdgdirList.constBegin();
         it != xdgdirList.constEnd(); ++it)
    {
        QString dir = KShell::tildeExpand(*it);
        addXdgDataPrefix(dir);
    }
    // end XDG_DATA_XXX


    addResourceType("lib", 0, "lib" KDELIBSUFF "/");

    uint index = 0;
    while (types_indices[index] != -1) {
        addResourceType(types_string + types_indices[index], 0, types_string + types_indices[index+1], true);
        index+=2;
    }
    addResourceType("exe", "lib", "kde4/libexec", true );

    addResourceDir("home", QDir::homePath(), false);

    addResourceType("autostart", "xdgconf-autostart", "/"); // merge them, start with xdg autostart
    addResourceType("autostart", NULL, "share/autostart"); // KDE ones are higher priority
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

    KConfig mapCfgFile(mapFile);
    KConfigGroup mapCfg(&mapCfgFile, "Users");
    if (mapCfg.hasKey(user.constData()))
    {
        profiles = mapCfg.readEntry(user.constData(), QStringList());
        return profiles;
    }

    const KConfigGroup generalGrp(&mapCfgFile, "General");
    const QStringList groups = generalGrp.readEntry("groups", QStringList());

    const KConfigGroup groupsGrp(&mapCfgFile, "Groups");

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
            profiles += groupsGrp.readEntry(*it, QStringList());
        }
        else
        {
            for(int i = 0; i < sup_gids_nr; i++)
            {
                if (sup_gids[i] == gid)
                {
                    // User is in this group --> add profiles
                    profiles += groupsGrp.readEntry(*it, QStringList());
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
    if (!d->m_checkRestrictions) // there are already customized entries
        return false; // we just quit and hope they are the right ones

    // save the numbers of config directories. If this changes,
    // we will return true to give KConfig a chance to reparse
    int configdirs = resourceDirs("config").count();

    if (true)
    {
        // reading the prefixes in
        QString group = QLatin1String("Directories");
        KConfigGroup cg(config, group);

        QString kioskAdmin = cg.readEntry("kioskAdmin");
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

        if (kde_kiosk_admin && !qgetenv("KDE_KIOSK_NO_PROFILES").isEmpty())
            readProfiles = false;

        QString userMapFile = cg.readEntry("userProfileMapFile");
        QString profileDirsPrefix = cg.readEntry("profileDirsPrefix");
        if (!profileDirsPrefix.isEmpty() && !profileDirsPrefix.endsWith('/'))
            profileDirsPrefix.append("/");

        QStringList profiles;
        if (readProfiles)
            profiles = lookupProfiles(userMapFile);
        QString profile;

        bool priority = false;
        while(true)
        {
            KConfigGroup cg(config, group);
            const QStringList list = cg.readEntry("prefixes", QStringList());
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
            const QMap<QString, QString> entries = config->entryMap(group);
            for (QMap<QString, QString>::ConstIterator it2 = entries.begin();
                 it2 != entries.end(); ++it2)
            {
                const QString key = it2.key();
                if (key.startsWith(QLatin1String("dir_"))) {
                    // generate directory list, there may be more than 1.
                    QStringList dirs = (*it2).split(',');
                    QStringList::Iterator sIt(dirs.begin());
                    QString resType = key.mid(4);
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
    if (!kde_kiosk_admin || qgetenv("KDE_KIOSK_NO_RESTRICTIONS").isEmpty())
    {
        KConfigGroup cg(config, "KDE Resource Restrictions");
        const QMap<QString, QString> entries = cg.entryMap();
        for (QMap<QString, QString>::ConstIterator it2 = entries.begin();
             it2 != entries.end(); ++it2)
        {
            const QString key = it2.key();
            if (!cg.readEntry(key, true))
            {
                d->m_restrictionsActive = true;
                d->m_restrictions.insert(key.toLatin1(), true);
                d->m_dircache.remove(key.toLatin1());
            }
        }
    }

    // check if the number of config dirs changed
    bool configDirsChanged = (resourceDirs("config").count() != configdirs);
    // If the config dirs changed, we check kiosk restrictions again.
    d->m_checkRestrictions = configDirsChanged;
    // return true if the number of config dirs changed: reparse config file
    return configDirsChanged;
}

QString KStandardDirs::localkdedir() const
{
    // Return the prefix to use for saving
    return d->m_prefixes.first();
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
QString KStandardDirs::locate( const char *type,
                               const QString& filename, const KComponentData &cData)
{
    return cData.dirs()->findResource(type, filename);
}

QString KStandardDirs::locateLocal( const char *type,
                                    const QString& filename, const KComponentData &cData)
{
    return locateLocal(type, filename, true, cData);
}

QString KStandardDirs::locateLocal( const char *type,
                                    const QString& filename, bool createDir,
                                    const KComponentData &cData)
{
    // try to find slashes. If there are some, we have to
    // create the subdir first
    int slash = filename.lastIndexOf('/')+1;
    if (!slash) { // only one filename
        return cData.dirs()->saveLocation(type, QString(), createDir) + filename;
    }

    // split path from filename
    QString dir = filename.left(slash);
    QString file = filename.mid(slash);
    return cData.dirs()->saveLocation(type, dir, createDir) + file;
}

bool KStandardDirs::checkAccess(const QString& pathname, int mode)
{
    int accessOK = KDE::access( pathname, mode );
    if ( accessOK == 0 )
        return true;  // OK, I can really access the file

    // else
    // if we want to write the file would be created. Check, if the
    // user may write to the directory to create the file.
    if ( (mode & W_OK) == 0 )
        return false;   // Check for write access is not part of mode => bail out


    if (!KDE::access( pathname, F_OK)) // if it already exists
        return false;

    //strip the filename (everything until '/' from the end
    QString dirName(pathname);
    int pos = dirName.lastIndexOf('/');
    if ( pos == -1 )
        return false;   // No path in argument. This is evil, we won't allow this
    else if ( pos == 0 ) // don't turn e.g. /root into an empty string
        pos = 1;

    dirName.truncate(pos); // strip everything starting from the last '/'

    accessOK = KDE::access( dirName, W_OK );
    // -?- Can I write to the accessed diretory
    if ( accessOK == 0 )
        return true;  // Yes
    else
        return false; // No
}

