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

#include "kconfigbackend.h"

#include "kdesktopfile.h"
#include "kdesktopfile.moc"

KDesktopFile::KDesktopFile(const QString &pFileName, bool bReadOnly,
			   const QString &resType)
  : KConfig("", bReadOnly, false)
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

  if(len > 8 && path.right(8) == ".desktop")
    return true;
  else if(len > 7 && path.right(7) == ".kdelnk")
    return true;
  else
    return false;
}

QString KDesktopFile::readType()
{
  return readEntry("Type");
}

QString KDesktopFile::readIcon()
{
  return readEntry("Icon");
}

QString KDesktopFile::readMiniIcon()
{
  return readEntry("MiniIcon");
}

QString KDesktopFile::readName()
{
  return readEntry("Name");
}

QString KDesktopFile::readComment()
{
  return readEntry("Comment");
}

QString KDesktopFile::readPath()
{
  return readEntry("Path");
}

QString KDesktopFile::readURL()
{
    return readEntry("URL");
}

bool KDesktopFile::hasLinkType()
{
  return readEntry("Type") == "Link";
}

bool KDesktopFile::hasApplicationType()
{
  return readEntry("Type") == "Application";
}

bool KDesktopFile::hasMimeTypeType()
{
  return readEntry("Type") == "MimeType";
}

bool KDesktopFile::hasDeviceType()
{
  return readEntry("Type") == "FSDev" || readEntry("Type") == "FSDevice";
}

bool KDesktopFile::tryExec()
{
  // if there is no TryExec field, just return OK.
  QString te(readEntry("TryExec"));
  if (te.isNull())
    return true;

  if (!te.isEmpty()) {
    if (te[0] == '/') {
      if (::access(te.utf8(), R_OK & X_OK))
	return false;
      else
	return true;
    } else {
      QStringList dirs = QStringList::split(":", ::getenv("PATH"));
      QStringList::Iterator it(dirs.begin());
      for (; it != dirs.end(); ++it) {
	QString fName = *it + te;
	if (::access(fName.utf8(), R_OK & X_OK) == 0)
	  return true;
      }
      // didn't match at all
      return false;
    }
  }
  return true;
}
