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

#include "kdesktopfile.h"

#include <unistd.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

#include <config.h>
#include "kconfig_p.h"
#include "kdebug.h"
#include "kurl.h"
#include "kconfiggroup.h"
#include "kauthorized.h"
#include "kstandarddirs.h"
#include "kconfigini_p.h"
#include "kde_file.h"

class KDesktopFilePrivate : public KConfigPrivate
{
 public:
    KDesktopFilePrivate(const char * resourceType, const QString &fileName);
    KConfigGroup desktopGroup;
};

KDesktopFilePrivate::KDesktopFilePrivate(const char * resourceType, const QString &fileName)
    : KConfigPrivate(KGlobal::mainComponent(), KConfig::NoGlobals, resourceType)
{
    mBackend = new KConfigIniBackend();
    bDynamicBackend = false;
    changeFileName(fileName, resourceType);
}

KDesktopFile::KDesktopFile(const char * resourceType, const QString &fileName)
    : KConfig(*new KDesktopFilePrivate(resourceType, fileName))
{
    Q_D(KDesktopFile);
    reparseConfiguration();
    d->desktopGroup = KConfigGroup(this, "Desktop Entry");
}

KDesktopFile::KDesktopFile(const QString &fileName)
    : KConfig(*new KDesktopFilePrivate("apps", fileName)) // TODO KDE5: default to xdgdata-apps instead of apps
{
    Q_D(KDesktopFile);
    reparseConfiguration();
    d->desktopGroup = KConfigGroup(this, "Desktop Entry");
}

KDesktopFile::~KDesktopFile()
{
}

KConfigGroup KDesktopFile::desktopGroup() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup;
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
  // Explicitly forbid desktop files if Kiosk does not allow.  There
  // may be other reasons that desktop files are forbidden so keep
  // checking otherwise.
  if (!KAuthorized::authorize("run_desktop_files")) {
     kWarning() << "Access to '" << path << "' denied because of 'run_desktop_files' restriction." << endl;
     return false;
  }

  if (path.isEmpty())
     return false; // Empty paths are not ok.

  if (QDir::isRelativePath(path))
     return true; // Relative paths are ok.

  KStandardDirs *dirs = KGlobal::dirs();
  QStringList kdePrefixes;
  kdePrefixes += dirs->resourceDirs("apps");
  kdePrefixes += dirs->resourceDirs("services");
  kdePrefixes += dirs->resourceDirs("xdgdata-apps");
  kdePrefixes += dirs->resourceDirs("autostart");

  const QString realPath = KStandardDirs::realPath(path);

  // Check if the .desktop file is installed as part of KDE or XDG.
  foreach (const QString &prefix, kdePrefixes) {
    if (realPath.startsWith(prefix))
      return true;
  }

  // Not otherwise permitted, so only allow if the file is executable, or if
  // owned by root (uid == 0)
  QFileInfo entryInfo( path );
  if (entryInfo.isExecutable() || entryInfo.ownerId() == 0)
      return true;

  kWarning() << "Access to '" << path << "' denied because of 'non_executable_desktop_file' restriction." << endl;
  return false;
}

QString KDesktopFile::readType() const
{
  Q_D(const KDesktopFile);
  return d->desktopGroup.readEntry("Type", QString());
}

QString KDesktopFile::readIcon() const
{
  Q_D(const KDesktopFile);
  return d->desktopGroup.readEntry("Icon", QString());
}

QString KDesktopFile::readName() const
{
  Q_D(const KDesktopFile);
  return d->desktopGroup.readEntry("Name", QString());
}

QString KDesktopFile::readComment() const
{
  Q_D(const KDesktopFile);
  return d->desktopGroup.readEntry("Comment", QString());
}

QString KDesktopFile::readGenericName() const
{
  Q_D(const KDesktopFile);
  return d->desktopGroup.readEntry("GenericName", QString());
}

QString KDesktopFile::readPath() const
{
  Q_D(const KDesktopFile);
  // NOT readPathEntry, it is not XDG-compliant. Path entries written by
  // KDE4 will be still treated as such, though.
  return d->desktopGroup.readEntry("Path", QString());
}

QString KDesktopFile::readDevice() const
{
  Q_D(const KDesktopFile);
  return d->desktopGroup.readEntry("Dev", QString());
}

QString KDesktopFile::readUrl() const
{
    Q_D(const KDesktopFile);
    if (hasDeviceType()) {
        return d->desktopGroup.readEntry("MountPoint", QString());
    } else {
        // NOT readPathEntry (see readPath())
        QString url = d->desktopGroup.readEntry("URL", QString());
        if ( !url.isEmpty() && !QDir::isRelativePath(url) )
        {
            // Handle absolute paths as such (i.e. we need to escape them)
            return KUrl(url).url();
        }
        return url;
    }
}

QStringList KDesktopFile::readActions() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readXdgListEntry("Actions");
}

KConfigGroup KDesktopFile::actionGroup(const QString &group)
{
    return KConfigGroup(this, QLatin1String("Desktop Action ") + group);
}

const KConfigGroup KDesktopFile::actionGroup(const QString& group) const
{
    return const_cast<KDesktopFile*>(this)->actionGroup(group);
}

bool KDesktopFile::hasActionGroup(const QString &group) const
{
  return hasGroup(QString(QLatin1String("Desktop Action ") + group).toUtf8().constData());
}

bool KDesktopFile::hasLinkType() const
{
  return readType() == QLatin1String("Link");
}

bool KDesktopFile::hasApplicationType() const
{
  return readType() == QLatin1String("Application");
}

bool KDesktopFile::hasMimeTypeType() const
{
  return readType() == QLatin1String("MimeType");
}

bool KDesktopFile::hasDeviceType() const
{
  return readType() == QLatin1String("FSDevice");
}

bool KDesktopFile::tryExec() const
{
  Q_D(const KDesktopFile);
  // Test for TryExec and "X-KDE-AuthorizeAction"
  // NOT readPathEntry (see readPath())
  QString te = d->desktopGroup.readEntry("TryExec", QString());

  if (!te.isEmpty()) {
    if (!QDir::isRelativePath(te)) {
      if (KDE::access(te, X_OK))
        return false;
    } else {
      // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
      // Environment PATH may contain filenames in 8bit locale specified
      // encoding (Like a filenames).
      const QStringList dirs = QFile::decodeName(qgetenv("PATH"))
        .split(KPATH_SEPARATOR,QString::SkipEmptyParts);
      QStringList::ConstIterator it(dirs.begin());
      bool match = false;
      for (; it != dirs.end(); ++it) {
        QString fName = *it + KDIR_SEPARATOR + te;
        if (KDE::access(fName, X_OK) == 0)
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
  const QStringList list = d->desktopGroup.readEntry("X-KDE-AuthorizeAction", QStringList());
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
  bool su = d->desktopGroup.readEntry("X-KDE-SubstituteUID", false);
  if (su)
  {
      QString user = d->desktopGroup.readEntry("X-KDE-Username", QString());
      if (user.isEmpty())
        user = QString::fromLocal8Bit(qgetenv("ADMIN_ACCOUNT"));
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
//QString KDesktopFile::fileName() const { return backEnd->fileName(); }

/**
 * @return the resource type as passed to the constructor.
 */
//QString
//KDesktopFile::resource() const { return backEnd->resource(); }

QStringList
KDesktopFile::sortOrder() const
{
  Q_D(const KDesktopFile);
  return d->desktopGroup.readEntry("SortOrder", QStringList());
}

//void KDesktopFile::virtual_hook( int id, void* data )
//{ KConfig::virtual_hook( id, data ); }

QString KDesktopFile::readDocPath() const
{
  Q_D(const KDesktopFile);
  //legacy entry in kde3 apps
  if(d->desktopGroup.hasKey( "DocPath" ))
    return d->desktopGroup.readPathEntry( "DocPath", QString() );
  return d->desktopGroup.readPathEntry( "X-DocPath", QString() );
}

KDesktopFile* KDesktopFile::copyTo(const QString &file) const
{
  KDesktopFile *config = new KDesktopFile(QString());
  this->KConfig::copyTo(file, config);
//  config->setDesktopGroup();
  return config;
}

const char *KDesktopFile::resource() const
{
    Q_D(const KDesktopFile);
    return d->resourceType;
}

QString KDesktopFile::fileName() const
{
    return name();
}

bool KDesktopFile::noDisplay() const
{
    Q_D(const KDesktopFile);
    if (d->desktopGroup.readEntry("NoDisplay", false)) {
        return true;
    }
    if (d->desktopGroup.hasKey("OnlyShowIn")) {
        if (!d->desktopGroup.readXdgListEntry("OnlyShowIn").contains("KDE"))
            return true;
    }
    if (d->desktopGroup.hasKey("NotShowIn")) {
        if (d->desktopGroup.readXdgListEntry("NotShowIn").contains("KDE"))
            return true;
    }
    return false;
}
