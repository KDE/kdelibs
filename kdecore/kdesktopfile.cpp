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

#include "kconfigbackend.h"

#include "kdesktopfile.h"
#include "kdesktopfile.moc"

KDesktopFile::KDesktopFile(const QString &pFileName, bool bReadOnly,
			   const char * resType)
  : KConfig(QString::fromLatin1(""), bReadOnly, false)
{
  // KConfigBackEnd will try to locate the filename that is provided
  // based on the resource type specified, _only_ if the filename
  // is not an absolute path.
  backEnd->changeFileName(pFileName, resType, false);
  parseConfigFiles();
  setDesktopGroup();
}

KDesktopFile::~KDesktopFile()
{
  // no need to do anything
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

QString KDesktopFile::readPath() const
{
  return readEntry("Path");
}

QString KDesktopFile::readDevice() const
{
  return readEntry("Dev");
}

QString KDesktopFile::readURL() const
{
    if (hasDeviceType()) {
	QString devURL;
	
	// in this case, we do some magic. :)
	QCString fstabFile;
	int indexDevice = 0;  // device on first column
	int indexMountPoint = 1; // mount point on second column
	int indexFSType = 2; // fstype on third column
	if (QFile::exists(QString::fromLatin1("/etc/fstab"))) { // Linux, ...
	    fstabFile = "/etc/fstab";
	} else if (QFile::exists(QString::fromLatin1("/etc/vfstab"))) { // Solaris
	    fstabFile = "/etc/vfstab";
	    indexMountPoint++;
	    indexFSType++;
	}
	// insert your favorite location for fstab here
	
	if ( !fstabFile.isEmpty() ) {
	    QFile f( fstabFile );
	    f.open( IO_ReadOnly );
	    QTextStream stream( &f );
	    while ( !stream.eof() ) {
		QString line = stream.readLine();
		line = line.simplifyWhiteSpace();
		if (!line.isEmpty() && line[0] == '/') { // skip comments but also
		    QStringList lst = QStringList::split( ' ', line );
		    if ( lst[indexDevice] == readDevice() )
			devURL = lst[indexMountPoint];
		}
	    }
	    f.close();
	}
	return devURL;

    } else {
	return readEntry("URL");
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
  // if there is no TryExec field, just return OK.
  QString te = readEntry("TryExec");
  if (te.isNull())
    return true;

  if (!te.isEmpty()) {
    if (te[0] == '/') {
      if (::access(QFile::encodeName(te), R_OK & X_OK))
	return false;
      else
	return true;
    } else {
      QStringList dirs = QStringList::split(':', QString::fromLatin1(::getenv("PATH")));
      QStringList::Iterator it(dirs.begin());
      for (; it != dirs.end(); ++it) {
	QString fName = *it + "/" + te;
	if (::access(QFile::encodeName(fName), R_OK & X_OK) == 0)
	  return true;
      }
      // didn't match at all
      return false;
    }
  }
  return true;
}

/**
 * @return the filename as passed to the constructor.
 */
QString
KDesktopFile::filename() const { return backEnd->filename(); }

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

