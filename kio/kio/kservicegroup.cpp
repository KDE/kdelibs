/*  This file is part of the KDE libraries
 *  Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

// $Id$

#include <kiconloader.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <ksortablevaluelist.h>
#include "kservicefactory.h"
#include "kservicegroupfactory.h"
#include "kservicegroup.h"
#include "kservice.h"
#include "ksycoca.h"

class KServiceGroup::Private
{
public:
  Private() { m_bNoDisplay = false; }
  bool m_bNoDisplay;
};

KServiceGroup::KServiceGroup( const QString &configFile, const QString & _relpath )
 : KSycocaEntry(_relpath), m_childCount(-1)
{
  d = new KServiceGroup::Private;
  m_bDeleted = false;

  if (!configFile.isEmpty())
  {
     KConfig config( _relpath+".directory", true, false, "apps" );

     config.setDesktopGroup();

     m_strCaption = config.readEntry( "Name" );
     m_strIcon = config.readEntry( "Icon" );
     m_strComment = config.readEntry( "Comment" );
     m_bDeleted = config.readBoolEntry( "Hidden", false );
     d->m_bNoDisplay = config.readBoolEntry( "NoDisplay", false );
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
  d = new KServiceGroup::Private;
  m_bDeep = deep;
  load( _str );
}

KServiceGroup::~KServiceGroup()
{
  delete d;
}

int KServiceGroup::childCount()
{
  if (m_childCount == -1)
  {
     m_childCount = 0;

     for( List::ConstIterator it = m_serviceList.begin();
          it != m_serviceList.end(); it++)
     {
        KSycocaEntry *p = (*it);
        if (p->isType(KST_KService))
        {
           KService *service = static_cast<KService *>(p);
           if (!service->noDisplay())
              m_childCount++;
        }
        else if (p->isType(KST_KServiceGroup))
        {
           KServiceGroup *serviceGroup = static_cast<KServiceGroup *>(p);
           m_childCount += serviceGroup->childCount();
        }
     }
  }
  return m_childCount;
}


bool KServiceGroup::noDisplay() const
{
    return d->m_bNoDisplay;
}

void KServiceGroup::load( QDataStream& s )
{
  QStringList groupList;
  Q_INT8 noDisplay;
  s >> m_strCaption >> m_strIcon >>
      m_strComment >> groupList >> m_strBaseGroupName >> m_childCount >>
      noDisplay;

  d->m_bNoDisplay = (noDisplay != 0);

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

  (void) childCount();

  Q_INT8 noDisplay = d->m_bNoDisplay ? 1 : 0;
  s << m_strCaption << m_strIcon <<
      m_strComment << groupList << m_strBaseGroupName << m_childCount <<
      noDisplay;
}

KServiceGroup::List
KServiceGroup::entries(bool sort)
{
   return entries(sort, true);
}

KServiceGroup::List
KServiceGroup::entries(bool sort, bool excludeNoDisplay)
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


    // Sort the list alphabetically, according to locale.
    // Groups come first, then services.

    KSortableValueList<SPtr,QCString> slist;
    KSortableValueList<SPtr,QCString> glist;
    for (List::ConstIterator it(group->m_serviceList.begin()); it != group->m_serviceList.end(); ++it)
    {
        // Choose the right list
        KSortableValueList<SPtr,QCString> & list = (*it)->isType(KST_KServiceGroup) ? glist : slist;
        QCString key( (*it)->name().length() * 4 + 1 );
        // strxfrm() crashes on Solaris
#ifndef USE_SOLARIS
        // maybe it'd be better to use wcsxfrm() where available
        size_t ln = strxfrm( key.data(), (*it)->name().local8Bit().data(), key.size());
        if( ln != size_t( -1 ))
        {
            if( ln >= key.size())
            { // didn't fit?
                key.resize( ln + 1 );
                if( strxfrm( key.data(), (*it)->name().local8Bit().data(), key.size()) == size_t( -1 ))
                    key = (*it)->name().local8Bit();
            }
        }
        else
#endif
        {
            key = (*it)->name().local8Bit();
        }
        list.insert(key,SPtr(*it));
    }
    // Now sort
    slist.sort();
    glist.sort();

    List lsort;
    for(KSortableValueList<SPtr,QCString>::ConstIterator it = glist.begin(); it != glist.end(); ++it)
    {
        if (excludeNoDisplay)
        {
           KServiceGroup *serviceGroup = (KServiceGroup *)((KSycocaEntry *)((*it).value()));
           if (serviceGroup->noDisplay())
              continue;
        }
        lsort.append((*it).value());
    }
    for(KSortableValueList<SPtr,QCString>::ConstIterator it = slist.begin(); it != slist.end(); ++it)
    {
        if (excludeNoDisplay)
        {
           KService *service = (KService *)((KSycocaEntry *)((*it).value()));
           if (service->noDisplay())
              continue;
        }
        lsort.append((*it).value());
    }

    // honor the SortOrder Key

    QString rp = relPath();
    if(rp == "/") rp = QString::null;

    QStringList order =
        KDesktopFile(rp + QString::fromUtf8(".directory")).sortOrder();

    if (order.isEmpty())
        return lsort;

    //kdDebug() << "Honouring sort order " << order.join(",") << endl;

    // Iterate through the sort spec list. If we find an entry that matches one
    // in the original list, take it out of the original list and add it to the
    // sorted list. Finally, add all entries that are still in the original list
    // to the end of the sorted list.

    List sorted;
    List orig = lsort;

    for (QStringList::ConstIterator it(order.begin()); it != order.end(); ++it)
    {
        //kdDebug() << "order has : " << *it << endl;
        for (List::Iterator sit(orig.begin()); sit != orig.end(); ++sit)
            {
                QString entry = (*sit)->entryPath();
                // Groups have a trailing slash, we need to remove it first
                if ( entry[entry.length()-1] == '/' )
                   entry.truncate(entry.length()-1);
                //kdDebug() << "Comparing to : " << entry.mid(entry.findRev('/')+1) << endl;
                if (*it == entry.mid(entry.findRev('/')+1))
                    {
                        //kdDebug() << "Appending to sorted : " << *it << endl;
                        sorted.append(*sit);
                        orig.remove(sit);
                        break;
                    }
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

void KServiceGroup::virtual_hook( int id, void* data )
{ KSycocaEntry::virtual_hook( id, data ); }
