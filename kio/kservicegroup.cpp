/*  This file is part of the KDE libraries
 *  Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
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

// $Id$

#include <kiconloader.h>
#include "kservicefactory.h"
#include "kservicegroupfactory.h"
#include "kservicegroup.h"
#include "kservice.h"
#include "ksycoca.h"

KServiceGroup::KServiceGroup( const QString &configFile, const QString & _relpath )
 : KSycocaEntry(_relpath)
{
  m_bDeleted = false;

  if (!configFile.isEmpty())
  {
     KDesktopFile config( configFile );

     config.setDesktopGroup();

     m_strCaption = config.readEntry( "Name" );
     m_strIcon = config.readEntry( "Icon" );
     m_strComment = config.readEntry( "Comment" );
     m_bDeleted = config.readBoolEntry( "Hidden", false );
  }
  // Fill in defaults.
  if (m_strCaption.isEmpty())
  {
     m_strCaption = _relpath;
     if (m_strCaption.right(1) == "/")
        m_strCaption = m_strCaption.left(m_strCaption.length()-1);
     int i = m_strCaption.findRev('/');
     if (i > 0)
        m_strCaption = m_strCaption.mid(i+1);
  }
  if (m_strIcon.isEmpty())
     m_strIcon = "folder";
}

KServiceGroup::KServiceGroup( QDataStream& _str, int offset, bool deep ) : 
	KSycocaEntry( _str, offset )
{
  m_bDeep = deep;
  load( _str );
}

KServiceGroup::~KServiceGroup()
{
}

void KServiceGroup::load( QDataStream& s )
{
  QStringList groupList;

  s >> m_strCaption >> m_strIcon >>
       m_strComment >> groupList;

  if (m_bDeep)
  {
     for(QStringList::ConstIterator it = groupList.begin(); 
	 it != groupList.end(); it++)
     {
        QString path = *it;
        if (path[path.length()-1] == '/')
        {
           KServiceGroup *serviceGroup;
           serviceGroup = KServiceGroupFactory::self()->findGroupByDesktopPath(path, false);
           m_serviceList.append( SPtr(serviceGroup) );
        }
        else
        {
           KService *service;
           service = KServiceFactory::self()->findServiceByDesktopPath(path);
           m_serviceList.append( SPtr(service) );
        }
     }
  }
}

void KServiceGroup::addEntry( KSycocaEntry *entry)
{
  m_serviceList.append(entry);
}

void KServiceGroup::save( QDataStream& s )
{
  KSycocaEntry::save( s );

  QStringList groupList;
  for( List::ConstIterator it = m_serviceList.begin();
       it != m_serviceList.end(); it++)
  {
     KSycocaEntry *p = (*it);
     if (p->isType(KST_KService)) 
     {
        KService *service = static_cast<KService *>(p);
        groupList.append( service->desktopEntryPath());
     }
     else if (p->isType(KST_KServiceGroup))
     {
        KServiceGroup *serviceGroup = static_cast<KServiceGroup *>(p);
        groupList.append( serviceGroup->relPath());
     }
     else
     {
	//fprintf(stderr, "KServiceGroup: Unexpected object in list!\n");
     }
  }

  s << m_strCaption << m_strIcon << 
       m_strComment << groupList;
}

KServiceGroup::List 
KServiceGroup::entries()
{
   KServiceGroup *group = this;
   // If the entries haven't been loaded yet, we have to reload ourselves
   // together with the entries. We can't only load the entries afterwards
   // since the offsets could have been changed if the database has changed.
   if (!m_bDeep)
   {
      group = KServiceGroupFactory::self()->findGroupByDesktopPath(relPath(), true);
      if (!group) // No guarantee that we still exist!
         return List();
   }
   return group->m_serviceList;
}

KServiceGroup::Ptr 
KServiceGroup::root()
{
   return KServiceGroupFactory::self()->findGroupByDesktopPath("/", true);
}

KServiceGroup::Ptr 
KServiceGroup::group(const QString &relPath)
{
   if (relPath.isEmpty()) return root();
   return KServiceGroupFactory::self()->findGroupByDesktopPath(relPath, true);
}

