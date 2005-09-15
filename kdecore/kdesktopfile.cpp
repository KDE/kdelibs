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
  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include <stdlib.h>
#include <unistd.h>

#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>

#include <kdebug.h>
#include "kurl.h"
#include "kconfigbackend.h"
#include "kapplication.h"
#include "kauthorized.h"
#include "kstandarddirs.h"
#include "kmountpoint.h"

#include "kdesktopfile.h"
#include "kdesktopfile.moc"

class KDesktopFile::Private {
};

KDesktopFile::KDesktopFile(const QString &fileName, bool bReadOnly,
			   const char * resType)
  : KConfig(QString(), bReadOnly, false), d(0)
{
  // KConfigBackEnd will try to locate the filename that is provided
  // based on the resource type specified, _only_ if the filename
  // is not an absolute path.
  backEnd->changeFileName(fileName, resType, false);
  setReadOnly(bReadOnly);
  reparseConfiguration();
  setDesktopGroup();
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
      local = ::locateLocal("apps", local); // Relative to apps
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
      local = ::locateLocal("xdgdata-dirs", local);
    }
  }
  else
  {
    if (QDir::isRelativePath(path))
    {
      local = ::locateLocal("apps", path); // Relative to apps
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
      local = ::locateLocal("xdgdata-apps", local);
    }
  }
  return local;
}

bool KDesktopFile::isDesktopFile(const QString& path)
{
  return (path.length() > 8
	   && path.right(8) == QLatin1String(".desktop"));
}

bool KDesktopFile::isAuthorizedDesktopFile(const QString& path)
{
  if (!kapp || KAuthorized::authorize("run_desktop_files"))
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

  kdWarning() << "Access to '" << path << "' denied because of 'run_desktop_files' restriction." << endl;
  return false;
}

QString KDesktopFile::readType() const
{
  return readEntry("Type");
}

QString KDesktopFile::readIcon() const
{
  return readEntry("Icon");
}

QString KDesktopFile::readName() const
{
  return readEntry("Name");
}

QString KDesktopFile::readComment() const
{
  return readEntry("Comment");
}

QString KDesktopFile::readGenericName() const
{
  return readEntry("GenericName");
}

QString KDesktopFile::readPath() const
{
  return readPathEntry("Path");
}

QString KDesktopFile::readDevice() const
{
  return readEntry("Dev");
}

QString KDesktopFile::readURL() const
{
    if (hasDeviceType()) {
        QString device = readDevice();
        KMountPoint::List mountPoints = KMountPoint::possibleMountPoints();

        for(KMountPoint::List::ConstIterator it = mountPoints.begin();
            it != mountPoints.end(); ++it)
        {
            KMountPoint *mp = *it;
            if (mp->mountedFrom() == device)
            {
                KURL u;
                u.setPath( mp->mountPoint() );
                return u.url();
            }
        }
        return QString();
    } else {
	QString url = readPathEntry("URL");
        if ( !url.isEmpty() && !QDir::isRelativePath(url) )
        {
            // Handle absolute paths as such (i.e. we need to escape them)
            KURL u;
            u.setPath( url );
            return u.url();
        }
        return url;
    }
}

QStringList KDesktopFile::readActions() const
{
    return readListEntry("Actions", ';');
}

void KDesktopFile::setActionGroup(const QString &group)
{
    setGroup(QLatin1String("Desktop Action ") + group);
}

bool KDesktopFile::hasActionGroup(const QString &group) const
{
  return hasGroup(QLatin1String("Desktop Action ") + group);
}

bool KDesktopFile::hasLinkType() const
{
  return readEntry("Type") == QLatin1String("Link");
}

bool KDesktopFile::hasApplicationType() const
{
  return readEntry("Type") == QLatin1String("Application");
}

bool KDesktopFile::hasMimeTypeType() const
{
  return readEntry("Type") == QLatin1String("MimeType");
}

bool KDesktopFile::hasDeviceType() const
{
  return readEntry("Type") == QLatin1String("FSDevice");
}

bool KDesktopFile::tryExec() const
{
  // Test for TryExec and "X-KDE-AuthorizeAction"
  QString te = readPathEntry("TryExec");

  if (!te.isEmpty()) {
    if (!QDir::isRelativePath(te)) {
      if (::access(QFile::encodeName(te), X_OK))
	return false;
    } else {
      // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
      // Environment PATH may contain filenames in 8bit locale cpecified
      // encoding (Like a filenames).
      QStringList dirs = QFile::decodeName(::getenv("PATH"))
	      .split(':',QString::SkipEmptyParts);
      QStringList::Iterator it(dirs.begin());
      bool match = false;
      for (; it != dirs.end(); ++it) {
	QString fName = *it + "/" + te;
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
  QStringList list = readListEntry("X-KDE-AuthorizeAction");
  if (kapp && !list.isEmpty())
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
  bool su = readBoolEntry("X-KDE-SubstituteUID");
  if (su)
  {
      QString user = readEntry("X-KDE-Username");
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
KDesktopFile::fileName() const { return backEnd->fileName(); }

/**
 * @return the resource type as passed to the constructor.
 */
QString
KDesktopFile::resource() const { return backEnd->resource(); }

QStringList
KDesktopFile::sortOrder() const
{
  return readListEntry("SortOrder");
}

void KDesktopFile::virtual_hook( int id, void* data )
{ KConfig::virtual_hook( id, data ); }

QString KDesktopFile::readDocPath() const
{
  if(hasKey( "DocPath" ))
    return readPathEntry( "DocPath" );
  return readPathEntry( "X-DocPath" );
}

KDesktopFile* KDesktopFile::copyTo(const QString &file) const
{
  KDesktopFile *config = new KDesktopFile(QString(), false);
  KConfig::copyTo(file, config);
  config->setDesktopGroup();
  return config;
}
