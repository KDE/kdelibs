/*  This file is part of the KDE libraries
 *  Copyright (C) 1999 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kbuildservicetypefactory.h"
#include "ksycoca.h"
#include "ksycocadict.h"

#include <kglobal.h>
#include <kstddirs.h>
#include <kmessageboxwrapper.h>
#include <kdebug.h>
#include <klocale.h>
#include <assert.h>

KBuildServiceTypeFactory::KBuildServiceTypeFactory() :
  KServiceTypeFactory()
{
   // Read servicetypes first, since they might be needed to read mimetype properties
   (*m_pathList) += KGlobal::dirs()->resourceDirs( "servicetypes" );
   (*m_pathList) += KGlobal::dirs()->resourceDirs( "mime" );
}

KServiceType * KBuildServiceTypeFactory::findServiceTypeByName(const QString &_name)
{
   assert (KSycoca::self()->isBuilding());
   // We're building a database - the service type must be in memory
   KSycocaEntry * servType = (*m_entryDict)[ _name ];
   return (KServiceType *) servType;
}


KSycocaEntry * KBuildServiceTypeFactory::createEntry(const QString &file)
{
  //debug("KBuildServiceTypeFactory::createEntry(%s)",file.ascii());
  // Just a backup file ?
  if ( file.right(1) == "~" || file.right(4) == ".bak" || ( file[0] == '%' && file.right(1) == "%" ) )
      return 0;

  KSimpleConfig cfg( file, true);
  cfg.setDesktopGroup();

  // TODO check Type field first
  QString mime = cfg.readEntry( "MimeType" );
  QString service = cfg.readEntry( "X-KDE-ServiceType" );

  if ( mime.isEmpty() && service.isEmpty() )
  {
    QString tmp = i18n( "The service/mime type config file\n%1\n"
			"does not contain a ServiceType=...\nor MimeType=... entry").arg( file );
    KMessageBoxWrapper::error( 0L, tmp);
    return 0;
  }
  
  KServiceType* e;
  if ( mime == "inode/directory" )
    e = new KFolderType( file );
  else if ( mime == "application/x-desktop" )
    e = new KDEDesktopMimeType( file );
  else if ( mime == "application/x-executable" || mime == "application/x-shellscript" )
    e = new KExecMimeType( file );
  else if ( !mime.isEmpty() )
    e = new KMimeType( file );
  else
    e = new KServiceType( file );

  if ( !(e->isValid()) )
  {
    kdebug( KDEBUG_WARN, 7012, "Invalid ServiceType : %s", file.ascii() );
    delete e;
    return 0;
  }

  return e;
}
