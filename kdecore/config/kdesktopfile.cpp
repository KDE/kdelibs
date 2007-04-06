/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Pietro Iglio <iglio@kde.org>
  Copyright (c) 1999 Preston Brown <pbrown@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include <config.h>

#include <stdlib.h>
#include <unistd.h>

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QTextIStream>

#include "kdesktopfile.h"
#include "kdebug.h"
#include "kurl.h"
#include "kconfigbackend.h"
#include "kauthorized.h"
#include "kstandarddirs.h"
#include "kmountpoint.h"


class KDesktopFile::Private
{
    public:
        Private()
            : group( 0 )
        {
        }

        ~Private()
        {
            delete group;
        }

        KConfigGroup* group;
};

KDesktopFile::KDesktopFile( const char * resType, const QString &fileName)
  : KConfig(QString(), KConfig::NoGlobals),
    d( new Private )
{
    // KConfigBackEnd will try to locate the filename that is provided
    // based on the resource type specified, _only_ if the filename
    // is not an absolute path.
    backEnd()->changeFileName( fileName, resType, false );
    reparseConfiguration();
    d->group = new KConfigGroup( this, "Desktop Entry" );

    // we still need to set the group for backwards compat when people call
    // readEntry directly =/
    setGroup( "Desktop Entry" );
}

KDesktopFile::KDesktopFile( const QString &fileName)
  : KConfig(QString(), KConfig::NoGlobals),
    d( new Private )
{
    // KConfigBackEnd will try to locate the filename that is provided
    // based on the resource type specified, _only_ if the filename
    // is not an absolute path.
    backEnd()->changeFileName( fileName, "apps", false );
    reparseConfiguration();
    d->group = new KConfigGroup( this, "Desktop Entry" );

    // we still need to set the group for backwards compat when people call
    // readEntry directly =/
    setGroup( "Desktop Entry" );
}

KDesktopFile::~KDesktopFile()
{
    delete d;
}

QString KDesktopFile::locateLocal(const QString &path)
{
  QString local;
  if (path.endsWith(".directory"))
  {
    local = path;
    if (!QDir::isRelativePath(local))
    {
      // Relative wrt apps?
      local = KGlobal::dirs()->relativeLocation("apps", path);
    }

    if (QDir::isRelativePath(local))
    {
      local = KStandardDirs::locateLocal("apps", local); // Relative to apps
    }
    else
    {
      // XDG Desktop menu items come with absolute paths, we need to
      // extract their relative path and then build a local path.
      local = KGlobal::dirs()->relativeLocation("xdgdata-dirs", local);
      if (!QDir::isRelativePath(local))
      {
        // Hm, that didn't work...
        // What now? Use filename only and hope for the best.
        local = path.mid(path.lastIndexOf('/')+1);
      }
      local = KStandardDirs::locateLocal("xdgdata-dirs", local);
    }
  }
  else
  {
    if (QDir::isRelativePath(path))
    {
      local = KStandardDirs::locateLocal("apps", path); // Relative to apps
    }
    else
    {
      // XDG Desktop menu items come with absolute paths, we need to
      // extract their relative path and then build a local path.
      local = KGlobal::dirs()->relativeLocation("xdgdata-apps", path);
      if (!QDir::isRelativePath(local))
      {
        // What now? Use filename only and hope for the best.
        local = path.mid(path.lastIndexOf('/')+1);
      }
      local = KStandardDirs::locateLocal("xdgdata-apps", local);
    }
  }
  return local;
}

bool KDesktopFile::isDesktopFile(const QString& path)
{
  return (path.length() > 8
          && path.endsWith(QLatin1String(".desktop")));
}

bool KDesktopFile::isAuthorizedDesktopFile(const QString& path)
{
  if (KAuthorized::authorize("run_desktop_files"))
     return true;

  if (path.isEmpty())
     return false; // Empty paths are not ok.

  if (QDir::isRelativePath(path))
     return true; // Relative paths are ok.

  KStandardDirs *dirs = KGlobal::dirs();
  if (QDir::isRelativePath( dirs->relativeLocation("apps", path) ))
     return true;
  if (QDir::isRelativePath( dirs->relativeLocation("xdgdata-apps", path) ))
     return true;
  if (QDir::isRelativePath( dirs->relativeLocation("services", path) ))
     return true;
  if (dirs->relativeLocation("data", path).startsWith("kdesktop/Desktop"))
     return true;

  kWarning() << "Access to '" << path << "' denied because of 'run_desktop_files' restriction." << endl;
  return false;
}

QString KDesktopFile::readType() const
{
  return d->group->readEntry("Type");
}

QString KDesktopFile::readIcon() const
{
  return d->group->readEntry("Icon");
}

QString KDesktopFile::readName() const
{
  return d->group->readEntry("Name");
}

QString KDesktopFile::readComment() const
{
  return d->group->readEntry("Comment");
}

QString KDesktopFile::readGenericName() const
{
  return d->group->readEntry("GenericName");
}

QString KDesktopFile::readPath() const
{
  return d->group->readPathEntry("Path");
}

QString KDesktopFile::readDevice() const
{
  return d->group->readEntry("Dev");
}

QString KDesktopFile::readUrl() const
{
    if (hasDeviceType()) {
        QString device = readDevice();
        KMountPoint::List mountPoints = KMountPoint::possibleMountPoints();

        for(KMountPoint::List::ConstIterator it = mountPoints.begin();
            it != mountPoints.end(); ++it)
        {
            const KSharedPtr<KMountPoint> mp = *it;
            if (mp->mountedFrom() == device)
            {
                KUrl u;
                u.setPath( mp->mountPoint() );
                return u.url();
            }
        }
        return QString();
    } else {
        QString url = d->group->readPathEntry("URL");
        if ( !url.isEmpty() && !QDir::isRelativePath(url) )
        {
            // Handle absolute paths as such (i.e. we need to escape them)
            KUrl u;
            u.setPath( url );
            return u.url();
        }
        return url;
    }
}

QStringList KDesktopFile::readActions() const
{
    return d->group->readEntry("Actions", QStringList(), ';');
}

void KDesktopFile::setActionGroup(const QString &group)
{
    delete d->group;
    d->group = new KConfigGroup( this, QLatin1String("Desktop Action ") + group );
}

KConfigGroup KDesktopFile::actionGroup(const QString &groupName) const
{
    return KConfigGroup(const_cast<KDesktopFile*>(this),
                        QLatin1String("Desktop Action ") + groupName);
}

bool KDesktopFile::hasActionGroup(const QString &group) const
{
  return hasGroup(QLatin1String("Desktop Action ") + group);
}

bool KDesktopFile::hasLinkType() const
{
  return d->group->readEntry("Type") == QLatin1String("Link");
}

bool KDesktopFile::hasApplicationType() const
{
  return d->group->readEntry("Type") == QLatin1String("Application");
}

bool KDesktopFile::hasMimeTypeType() const
{
  return d->group->readEntry("Type") == QLatin1String("MimeType");
}

bool KDesktopFile::hasDeviceType() const
{
  return d->group->readEntry("Type") == QLatin1String("FSDevice");
}

bool KDesktopFile::tryExec() const
{
  // Test for TryExec and "X-KDE-AuthorizeAction"
  QString te = d->group->readPathEntry("TryExec");

  if (!te.isEmpty()) {
    if (!QDir::isRelativePath(te)) {
      if (::access(QFile::encodeName(te), X_OK))
	return false;
    } else {
      // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
      // Environment PATH may contain filenames in 8bit locale cpecified
      // encoding (Like a filenames).
      QStringList dirs = QFile::decodeName(::getenv("PATH"))
	      .split(KPATH_SEPARATOR,QString::SkipEmptyParts);
      QStringList::Iterator it(dirs.begin());
      bool match = false;
      for (; it != dirs.end(); ++it) {
	QString fName = *it + '/' + te;
	if (::access(QFile::encodeName(fName), X_OK) == 0)
	{
	  match = true;
	  break;
	}
      }
      // didn't match at all
      if (!match)
        return false;
    }
  }
  QStringList list = d->group->readEntry("X-KDE-AuthorizeAction", QStringList());
  if (!list.isEmpty())
  {
     for(QStringList::ConstIterator it = list.begin();
         it != list.end();
         ++it)
     {
        if (!KAuthorized::authorize((*it).trimmed()))
           return false;
     }
  }

  // See also KService::username()
  bool su = d->group->readEntry("X-KDE-SubstituteUID", false);
  if (su)
  {
      QString user = d->group->readEntry("X-KDE-Username");
      if (user.isEmpty())
        user = ::getenv("ADMIN_ACCOUNT");
      if (user.isEmpty())
        user = "root";
      if (!KAuthorized::authorize("user/"+user))
        return false;
  }

  return true;
}

/**
 * @return the filename as passed to the constructor.
 */
QString
KDesktopFile::fileName() const {
    return backEnd()->fileName();
}

/**
 * @return the resource type as passed to the constructor.
 */
QString KDesktopFile::resource() const
{
    return backEnd()->resource();
}

QStringList
KDesktopFile::sortOrder() const
{
  return d->group->readEntry("SortOrder", QStringList());
}

void KDesktopFile::virtual_hook( int id, void* data )
{ KConfig::virtual_hook( id, data ); }

QString KDesktopFile::readDocPath() const
{
    if ( hasKey( "DocPath" ) ) {
        return d->group->readPathEntry( "DocPath" );
    }

    return d->group->readPathEntry( "X-DocPath" );
}

KDesktopFile* KDesktopFile::copyTo(const QString &file) const
{
  KDesktopFile *config = new KDesktopFile(QString());
  KConfig::copyTo(file, config);
  return config;
}

KConfigGroup KDesktopFile::desktopGroup()
{
   return KConfigGroup( this, "Desktop Entry" );
}

const KConfigGroup KDesktopFile::desktopGroup() const
{
    return KConfigGroup(const_cast<KDesktopFile*>(this), "Desktop Entry");
}

