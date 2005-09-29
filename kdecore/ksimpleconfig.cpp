/* This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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
// $Id$

#include <config.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <stdlib.h>
#include <unistd.h>

#include <qfileinfo.h>
#include <qdir.h>

#include "kglobal.h"
#include "kstandarddirs.h"
#include "kconfigbackend.h"

#include "ksimpleconfig.h"

KSimpleConfig::KSimpleConfig(const QString &fileName, bool bReadOnly)
  : KConfig(QString::fromLatin1(""), bReadOnly, false)
{
  // the difference between KConfig and KSimpleConfig is just that
  // for KSimpleConfig an absolute filename is guaranteed
  if (!fileName.isNull() && QDir::isRelativePath(fileName)) {
     backEnd->changeFileName( KGlobal::dirs()->
	saveLocation("config", QString::null, !bReadOnly)+fileName, "config", false);
  } else {
     backEnd->changeFileName(fileName, "config", false);
  }
  setReadOnly( bReadOnly );
  reparseConfiguration();
}

KSimpleConfig::KSimpleConfig(KConfigBackEnd *backEnd, bool bReadOnly)
  : KConfig(backEnd, bReadOnly)
{}

KSimpleConfig::~KSimpleConfig()
{
  // we need to call the KSimpleConfig version of sync.  Relying on the
  // regular KConfig sync is bad, because the KSimpleConfig sync has
  // different behavior.  Syncing here will insure that the sync() call
  // in the KConfig destructor doesn't actually do anything.
  sync();
}

void KSimpleConfig::sync()
{
   if (isReadOnly())
       return;
   backEnd->sync(false);

   if (isDirty())
     rollback();
}

void KSimpleConfig::virtual_hook( int id, void* data )
{ KConfig::virtual_hook( id, data ); }

#include "ksimpleconfig.moc"
