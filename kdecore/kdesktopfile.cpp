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
  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
  Boston, MA 02111-1307, USA.
*/

// $Id$

#include <stdlib.h>
#include <unistd.h>

#include <qfile.h>
#include <qtextstream.h>

#include "kurl.h"
#include "kconfigbackend.h"
#include "kapplication.h"
#include "kstandarddirs.h"
#include "kmountpoint.h"

#include "kdesktopfile.h"
#include "kdesktopfile.moc"

KDesktopFile::KDesktopFile(const QString &fileName, bool bReadOnly,
			   const char * resType)
  : KConfig(QString::fromLatin1(""), bReadOnly, false)
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
  // no need to do anything
}

QString KDesktopFile::locateLocal(const QString &path)
{
  QString local;
  if (path.endsWith(".directory"))
  {
    local = path;
    if (local.startsWith("/"))
    {
      // Relative wrt apps?
      local = KGlobal::dirs()->relativeLocation("apps", path);
    }

    if (!local.startsWith("/"))
    {
      local = ::locateLocal("apps", local); // Relative to apps
    }
    else
    {
      // XDG Desktop menu items come with absolute paths, we need to 
      // extract their relative path and then build a local path.
      local = KGlobal::dirs()->relativeLocation("xdgdata-dirs", local);
      if (local.startsWith("/"))
      {
        // Hm, that didn't work...
        // What now? Use filename only and hope for the best.
        local = path.mid(path.findRev('/')+1);
      }
      local = ::locateLocal("xdgdata-dirs", local);
    }
  }
  else
  {
    if (!path.startsWith("/"))
    {
      local = ::locateLocal("apps", path); // Relative to apps
    }
    else
    {
      // XDG Desktop menu items come with absolute paths, we need to 
      // extract their relative path and then build a local path.
      local = KGlobal::dirs()->relativeLocation("xdgdata-apps", path);
      if (local.startsWith("/"))
      {
        // What now? Use filename only and hope for the best.
        local = path.mid(path.findRev('/')+1);
      }
      local = ::locateLocal("xdgdata-apps", local);
    }
  }
  return local;
}

bool KDesktopFile::isDesktopFile(const QString& path)
{
  int len = path.length();

  if(len > 8 && path.right(8) == QString::fromLatin1(".desktop"))
    return true;
  else if(len > 7 && path.right(7) == QString::fromLatin1(".kdelnk"))
    return true;
  else
    return false;
}

bool KDesktopFile::isAuthorizedDesktopFile(const QString& path)
{
  if (!kapp || kapp->authorize("run_desktop_files"))
     return true;

  if (path.isEmpty())
     return false; // Empty paths are not ok.
  
  if (path[0] != '/')
     return true; // Relative paths are ok.
     
  KStandardDirs *dirs = KGlobal::dirs();
  if (dirs->relativeLocation("apps", path)[0] != '/')
     return true;
  if (dirs->relativeLocation("xdgdata-apps", path)[0] != '/')
     return true;
  if (dirs->relativeLocation("services", path)[0] != '/')
     return true;
  if (dirs->relativeLocation("data", path).startsWith("kdesktop/Desktop"))
     return true;
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
        return QString::null;
    } else {
	QString url = readPathEntry("URL");
        if ( !url.isEmpty() && url[0] == '/' )
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
    setGroup(QString::fromLatin1("Desktop Action ") + group);
}

bool KDesktopFile::hasActionGroup(const QString &group) const
{
  return hasGroup(QString::fromLatin1("Desktop Action ") + group);
}

bool KDesktopFile::hasLinkType() const
{
  return readEntry("Type") == QString::fromLatin1("Link");
}

bool KDesktopFile::hasApplicationType() const
{
  return readEntry("Type") == QString::fromLatin1("Application");
}

bool KDesktopFile::hasMimeTypeType() const
{
  return readEntry("Type") == QString::fromLatin1("MimeType");
}

bool KDesktopFile::hasDeviceType() const
{
  return readEntry("Type") == QString::fromLatin1("FSDev") ||
         readEntry("Type") == QString::fromLatin1("FSDevice");
}

bool KDesktopFile::tryExec() const
{
  // Test for TryExec and "X-KDE-AuthorizeAction" 
  QString te = readPathEntry("TryExec");

  if (!te.isEmpty()) {
    if (te[0] == '/') {
      if (::access(QFile::encodeName(te), R_OK | X_OK))
	return false;
    } else {
      // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
      // Environment PATH may contain filenames in 8bit locale cpecified
      // encoding (Like a filenames).
      QStringList dirs = QStringList::split(':', QFile::decodeName(::getenv("PATH")));
      QStringList::Iterator it(dirs.begin());
      bool match = false;
      for (; it != dirs.end(); ++it) {
	QString fName = *it + "/" + te;
	if (::access(QFile::encodeName(fName), R_OK | X_OK) == 0)
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
        if (!kapp->authorize((*it).stripWhiteSpace()))
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
      if (!kapp->authorize("user/"+user))
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
	return readPathEntry( "DocPath" );
}

KDesktopFile* KDesktopFile::copyTo(const QString &file) const
{
  KDesktopFile *config = new KDesktopFile(QString::null, false);
  KConfig::copyTo(file, config);
  config->setDesktopGroup();
  return config;
}


