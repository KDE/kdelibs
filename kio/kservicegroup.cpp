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
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kdebug.h>
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
     m_strBaseGroupName = config.readEntry( "X-KDE-BaseGroup" );
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
       m_strComment >> groupList >> m_strBaseGroupName;

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
       m_strComment << groupList << m_strBaseGroupName;
}

KServiceGroup::List
KServiceGroup::entries(bool sort)
{
    KServiceGroup *group = this;

    // If the entries haven't been loaded yet, we have to reload ourselves
    // together with the entries. We can't only load the entries afterwards
    // since the offsets could have been changed if the database has changed.

    if (!m_bDeep) {

        group =
            KServiceGroupFactory::self()->findGroupByDesktopPath(relPath(), true);

        if (0 == group) // No guarantee that we still exist!
            return List();
    }

    if (!sort)
        return group->m_serviceList;


    // Sort the list alphabetically.
    // Groups come first, then services.

    QMap<QString,SPtr> slist;
    QMap<QString,SPtr> glist;
    for (List::ConstIterator it(group->m_serviceList.begin()); it != group->m_serviceList.end(); ++it)
        {
        // Choose the right map
        QMap<QString,SPtr> & map = (*it)->isType(KST_KServiceGroup) ? glist : slist;
        // Check for duplicates - QMap doesn't like that
        QString name = (*it)->name();
        int n = 1;
        while (map.contains(name))
           name = (*it)->name()+QString::number(++n); // we append a number to make the entry unique
        map.insert(name,SPtr(*it));
        }

    // Iterating over the QMap returns sorted items
    List lsort;
    for(QMap<QString,SPtr>::ConstIterator it = glist.begin(); it != glist.end(); ++it)
        lsort.append(it.data());
    for(QMap<QString,SPtr>::ConstIterator it = slist.begin(); it != slist.end(); ++it)
        lsort.append(it.data());

    // honor the SortOrder Key

    QString rp = relPath();
    if(rp == "/") rp = QString::null;

    QStringList order =
        KDesktopFile(rp + QString::fromUtf8(".directory")).sortOrder();

    if (order.isEmpty())
        return lsort;

    // Iterate through the sort spec list. If we find an entry that matches one
    // in the original list, take it out of the original list and add it to the
    // sorted list. Finally, add all entries that are still in the original list
    // to the end of the sorted list.

    List sorted;
    List orig = lsort;

    for (QStringList::ConstIterator it(order.begin()); it != order.end(); ++it)
        for (List::Iterator sit(orig.begin()); sit != orig.end(); ++sit)
            {
                if (*it == (*sit)->entryPath().mid((*sit)->entryPath().findRev('/') + 1))
                    {
                        sorted.append(*sit);
                        orig.remove(sit);
                        break;
                    }
            }

    for (List::Iterator sit(orig.begin()); sit != orig.end(); ++sit)
        sorted.append(*sit);

    return sorted;
}

KServiceGroup::Ptr
KServiceGroup::baseGroup( const QString & _baseGroupName )
{
    return KServiceGroupFactory::self()->findBaseGroup(_baseGroupName, true);
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

