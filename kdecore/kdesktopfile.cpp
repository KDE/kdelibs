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

QString KDesktopFile::readType()
{
  return readEntry(QString::fromLatin1("Type"));
}

QString KDesktopFile::readIcon()
{
  return readEntry(QString::fromLatin1("Icon"));
}

QString KDesktopFile::readName()
{
  return readEntry(QString::fromLatin1("Name"));
}

QString KDesktopFile::readComment()
{
  return readEntry(QString::fromLatin1("Comment"));
}

QString KDesktopFile::readPath()
{
  return readEntry(QString::fromLatin1("Path"));
}

QString KDesktopFile::readDevice()
{
  return readEntry(QString::fromLatin1("Dev"));
}

QString KDesktopFile::readURL()
{
    return readEntry(QString::fromLatin1("URL"));
}

QStringList KDesktopFile::readActions()
{
    return readListEntry(QString::fromLatin1("Actions"), ';');
}

void KDesktopFile::setActionGroup(const QString &group)
{
    setGroup(QString::fromLatin1("Desktop Action ") + group);
}

bool KDesktopFile::hasActionGroup(const QString &group)
{
  return hasGroup(QString::fromLatin1("Desktop Action ") + group);
}

bool KDesktopFile::hasLinkType()
{
  return readEntry(QString::fromLatin1("Type")) == QString::fromLatin1("Link");
}

bool KDesktopFile::hasApplicationType()
{
  return readEntry(QString::fromLatin1("Type")) == QString::fromLatin1("Application");
}

bool KDesktopFile::hasMimeTypeType()
{
  return readEntry(QString::fromLatin1("Type")) == QString::fromLatin1("MimeType");
}

bool KDesktopFile::hasDeviceType()
{
  return readEntry(QString::fromLatin1("Type")) == QString::fromLatin1("FSDev") ||
         readEntry(QString::fromLatin1("Type")) == QString::fromLatin1("FSDevice");
}

bool KDesktopFile::tryExec()
{
  // if there is no TryExec field, just return OK.
  QString te = readEntry(QString::fromLatin1("TryExec"));
  if (te.isNull())
    return true;

  if (!te.isEmpty()) {
    if (te[0] == '/') {
      if (::access(te.utf8(), R_OK & X_OK))
	return false;
      else
	return true;
    } else {
      QStringList dirs = QStringList::split(':', QString::fromLatin1(::getenv("PATH")));
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
