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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

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
    Private() { m_bNoDisplay = false; m_bShowEmptyMenu = false;m_bShowInlineHeader=false;m_bInlineAlias=false; m_bAllowInline = false;m_inlineValue = 4;}
  bool m_bNoDisplay;
    bool m_bShowEmptyMenu;
    bool m_bShowInlineHeader;
    bool m_bInlineAlias;
    bool m_bAllowInline;
    int m_inlineValue;
    QStringList suppressGenericNames;
    QString directoryEntryPath;
    QStringList sortOrder;
};

KServiceGroup::KServiceGroup( const QString & name )
 : KSycocaEntry(name), m_childCount(-1)
{
  d = new KServiceGroup::Private;
  m_bDeleted = false;
}

KServiceGroup::KServiceGroup( const QString &configFile, const QString & _relpath )
 : KSycocaEntry(_relpath), m_childCount(-1)
{
  d = new KServiceGroup::Private;
  m_bDeleted = false;

  QString cfg = configFile;
  if (cfg.isEmpty())
     cfg = _relpath+".directory";

  d->directoryEntryPath = cfg;

  KConfig config( cfg, true, false, "apps" );

  config.setDesktopGroup();

  m_strCaption = config.readEntry( "Name" );
  m_strIcon = config.readEntry( "Icon" );
  m_strComment = config.readEntry( "Comment" );
  m_bDeleted = config.readBoolEntry( "Hidden", false );
  d->m_bNoDisplay = config.readBoolEntry( "NoDisplay", false );
  QStringList tmpList;
  if (config.hasKey("OnlyShowIn"))
  {
     if (!config.readListEntry("OnlyShowIn", ';').contains("KDE"))
        d->m_bNoDisplay = true;
  }
  if (config.hasKey("NotShowIn"))
  {
     if (config.readListEntry("NotShowIn", ';').contains("KDE"))
        d->m_bNoDisplay = true;
  }

  m_strBaseGroupName = config.readEntry( "X-KDE-BaseGroup" );
  d->suppressGenericNames = config.readListEntry( "X-KDE-SuppressGenericNames" );
//  d->sortOrder = config.readListEntry("SortOrder");

  // Fill in defaults.
  if (m_strCaption.isEmpty())
  {
     m_strCaption = _relpath;
     if (m_strCaption.endsWith(QLatin1Char('/')))
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


bool KServiceGroup::showInlineHeader() const
{
    return d->m_bShowInlineHeader;
}

bool KServiceGroup::showEmptyMenu() const
{
    return d->m_bShowEmptyMenu;
}

bool KServiceGroup::inlineAlias() const
{
    return d->m_bInlineAlias;
}

void KServiceGroup::setInlineAlias(bool _b)
{
    d->m_bInlineAlias = _b;
}

void KServiceGroup::setShowEmptyMenu(bool _b)
{
    d->m_bShowEmptyMenu=_b;
}

void KServiceGroup::setShowInlineHeader(bool _b)
{
    d->m_bShowInlineHeader=_b;
}

int KServiceGroup::inlineValue() const
{
    return d->m_inlineValue;
}

void KServiceGroup::setInlineValue(int _val)
{
    d->m_inlineValue = _val;
}

bool KServiceGroup::allowInline() const
{
    return d->m_bAllowInline;
}

void KServiceGroup::setAllowInline(bool _b)
{
    d->m_bAllowInline = _b;
}

bool KServiceGroup::noDisplay() const
{
  return d->m_bNoDisplay || m_strCaption.startsWith(".");
}

QStringList KServiceGroup::suppressGenericNames() const
{
  return d->suppressGenericNames;
}

void KServiceGroup::load( QDataStream& s )
{
  QStringList groupList;
  Q_INT8 noDisplay;
  Q_INT8 _showEmptyMenu;
  Q_INT8 inlineHeader;
  Q_INT8 _inlineAlias;
  Q_INT8 _allowInline;
  s >> m_strCaption >> m_strIcon >>
      m_strComment >> groupList >> m_strBaseGroupName >> m_childCount >>
      noDisplay >> d->suppressGenericNames >> d->directoryEntryPath >>
      d->sortOrder >> _showEmptyMenu >> inlineHeader >> _inlineAlias >> _allowInline;

  d->m_bNoDisplay = (noDisplay != 0);
  d->m_bShowEmptyMenu = ( _showEmptyMenu != 0 );
  d->m_bShowInlineHeader = ( inlineHeader != 0 );
  d->m_bInlineAlias = ( _inlineAlias != 0 );
  d->m_bAllowInline = ( _allowInline != 0 );

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
           if (serviceGroup)
              m_serviceList.append( SPtr(serviceGroup) );
        }
        else
        {
           KService *service;
           service = KServiceFactory::self()->findServiceByDesktopPath(path);
           if (service)
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
  Q_INT8 _showEmptyMenu = d->m_bShowEmptyMenu ? 1 : 0;
  Q_INT8 inlineHeader = d->m_bShowInlineHeader ? 1 : 0;
  Q_INT8 _inlineAlias = d->m_bInlineAlias ? 1 : 0;
  Q_INT8 _allowInline = d->m_bAllowInline ? 1 : 0;
  s << m_strCaption << m_strIcon <<
      m_strComment << groupList << m_strBaseGroupName << m_childCount <<
      noDisplay << d->suppressGenericNames << d->directoryEntryPath <<
      d->sortOrder <<_showEmptyMenu <<inlineHeader<<_inlineAlias<<_allowInline;
}

KServiceGroup::List
KServiceGroup::entries(bool sort)
{
   return entries(sort, true);
}

KServiceGroup::List
KServiceGroup::entries(bool sort, bool excludeNoDisplay)
{
   return entries(sort, excludeNoDisplay, false);
}

static void addItem(KServiceGroup::List &sorted, const KSycocaEntry::Ptr &p, bool &addSeparator)
{
   if (addSeparator && !sorted.isEmpty())
      sorted.append(new KServiceSeparator());
   sorted.append(p);
   addSeparator = false;
}

KServiceGroup::List
KServiceGroup::entries(bool sort, bool excludeNoDisplay, bool allowSeparators, bool sortByGenericName)
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

    KSortableValueList<SPtr,QByteArray> slist;
    KSortableValueList<SPtr,QByteArray> glist;
    for (List::ConstIterator it(group->m_serviceList.begin()); it != group->m_serviceList.end(); ++it)
    {
        KSycocaEntry *p = (*it);
	bool noDisplay = p->isType(KST_KServiceGroup) ?
                                   static_cast<KServiceGroup *>(p)->noDisplay() :
                                   static_cast<KService *>(p)->noDisplay();
        if (excludeNoDisplay && noDisplay)
           continue;
        // Choose the right list
        KSortableValueList<SPtr,QByteArray> & list = p->isType(KST_KServiceGroup) ? glist : slist;
        QString name;
        if (p->isType(KST_KServiceGroup))
          name = static_cast<KServiceGroup *>(p)->caption();
        else if (sortByGenericName)
          name = static_cast<KService *>(p)->genericName() + " " + p->name();
        else
          name = p->name() + " " + static_cast<KService *>(p)->genericName();

        QByteArray key( name.length() * 4 + 1 );
        // strxfrm() crashes on Solaris
#ifndef USE_SOLARIS
        // maybe it'd be better to use wcsxfrm() where available
        size_t ln = strxfrm( key.data(), name.local8Bit().data(), key.size());
        if( ln != size_t( -1 ))
        {
            if( (int)ln >= key.size())
            { // didn't fit?
                key.resize( ln + 1 );
                if( strxfrm( key.data(), name.local8Bit().data(), key.size()) == size_t( -1 ))
                    key = name.local8Bit();
            }
        }
        else
#endif
        {
            key = name.local8Bit();
        }
        list.insert(key,SPtr(*it));
    }
    // Now sort
    slist.sort();
    glist.sort();

    if (d->sortOrder.isEmpty())
    {
       d->sortOrder << ":M";
       d->sortOrder << ":F";
       d->sortOrder << ":OIH IL[4]"; //just inline header
    }

    QString rp = relPath();
    if(rp == "/") rp = QString::null;

    // Iterate through the sort spec list.
    // If an entry gets mentioned explicitly, we remove it from the sorted list
    for (QStringList::ConstIterator it(d->sortOrder.begin()); it != d->sortOrder.end(); ++it)
    {
        const QString &item = *it;
        if (item.isEmpty()) continue;
        if (item[0] == '/')
        {
          QString groupPath = rp + item.mid(1) + "/";
           // Remove entry from sorted list of services.
          for(KSortableValueList<SPtr,QByteArray>::Iterator it2 = glist.begin(); it2 != glist.end(); ++it2)
          {
             KServiceGroup *group = (KServiceGroup *)((KSycocaEntry *)((*it2).value()));
             if (group->relPath() == groupPath)
             {
                glist.remove(it2);
                break;
             }
          }
        }
        else if (item[0] != ':')
        {
           // Remove entry from sorted list of services.
           // TODO: Remove item from sortOrder-list if not found
           // TODO: This prevents duplicates
          for(KSortableValueList<SPtr,QByteArray>::Iterator it2 = slist.begin(); it2 != slist.end(); ++it2)
          {
             KService *service = (KService *)((KSycocaEntry *)((*it2).value()));
             if (service->menuId() == item)
             {
                slist.remove(it2);
                break;
             }
          }
        }
    }

    List sorted;

    bool needSeparator = false;
    // Iterate through the sort spec list.
    // Add the entries to the list according to the sort spec.
    for (QStringList::ConstIterator it(d->sortOrder.begin()); it != d->sortOrder.end(); ++it)
    {
        const QString &item = *it;
        if (item.isEmpty()) continue;
        if (item[0] == ':')
        {
          // Special condition...
          if (item == ":S")
          {
             if (allowSeparators)
                needSeparator = true;
          }
          else if ( item.contains( ":O" ) )
          {
              //todo parse attribute:
              QString tmp(  item );
              tmp = tmp.remove(":O");
              QStringList optionAttribute = QStringList::split(" ",tmp);
              if( optionAttribute.count()==0)
                  optionAttribute.append(tmp);
              bool showEmptyMenu = false;
              bool showInline = false;
              bool showInlineHeader = false;
              bool showInlineAlias = false;
              int inlineValue = -1;

              for ( QStringList::Iterator it3 = optionAttribute.begin(); it3 != optionAttribute.end(); ++it3 )
              {
                  parseAttribute( *it3,  showEmptyMenu, showInline, showInlineHeader, showInlineAlias, inlineValue );
              }
              for(KSortableValueList<SPtr,QByteArray>::Iterator it2 = glist.begin(); it2 != glist.end(); ++it2)
              {
                  KServiceGroup *group = (KServiceGroup *)((KSycocaEntry *)(*it2).value());
                  group->setShowEmptyMenu(  showEmptyMenu  );
                  group->setAllowInline( showInline );
                  group->setShowInlineHeader( showInlineHeader );
                  group->setInlineAlias( showInlineAlias );
                  group->setInlineValue( inlineValue );
              }

          }
          else if (item == ":M")
          {
            // Add sorted list of sub-menus
            for(KSortableValueList<SPtr,QByteArray>::Iterator it2 = glist.begin(); it2 != glist.end(); ++it2)
            {
              addItem(sorted, (*it2).value(), needSeparator);
            }
          }
          else if (item == ":F")
          {
            // Add sorted list of services
            for(KSortableValueList<SPtr,QByteArray>::Iterator it2 = slist.begin(); it2 != slist.end(); ++it2)
            {
              addItem(sorted, (*it2).value(), needSeparator);
            }
          }
          else if (item == ":A")
          {
            // Add sorted lists of services and submenus
            KSortableValueList<SPtr,QByteArray>::Iterator it_s = slist.begin();
            KSortableValueList<SPtr,QByteArray>::Iterator it_g = glist.begin();

            while(true)
            {
               if (it_s == slist.end())
               {
                  if (it_g == glist.end())
                     break; // Done

                  // Insert remaining sub-menu
                  addItem(sorted, (*it_g).value(), needSeparator);
                  it_g++;
               }
               else if (it_g == glist.end())
               {
                  // Insert remaining service
                  addItem(sorted, (*it_s).value(), needSeparator);
                  it_s++;
               }
               else if ((*it_g).index() < (*it_s).index())
               {
                  // Insert sub-menu first
                  addItem(sorted, (*it_g).value(), needSeparator);
                  it_g++;
               }
               else
               {
                  // Insert service first
                  addItem(sorted, (*it_s).value(), needSeparator);
                  it_s++;
               }
            }
          }
        }
        else if (item[0] == '/')
        {
            QString groupPath = rp + item.mid(1) + "/";

            for (List::ConstIterator it2(group->m_serviceList.begin()); it2 != group->m_serviceList.end(); ++it2)
            {
                if (!(*it2)->isType(KST_KServiceGroup))
                    continue;
                KServiceGroup *group = (KServiceGroup *)((KSycocaEntry *)(*it2));
                if (group->relPath() == groupPath)
                {
                    if (!excludeNoDisplay || !group->noDisplay())
                    {
                        ++it;
                        const QString &nextItem =
                            (it == d->sortOrder.end()) ? QString() : *it;

                        if ( nextItem.startsWith( ":O" ) )
                        {
                            QString tmp(  nextItem );
                            tmp = tmp.remove(":O");
                            QStringList optionAttribute = QStringList::split(" ",tmp);
                            if( optionAttribute.count()==0)
                                optionAttribute.append(tmp);
                            bool bShowEmptyMenu = false;
                            bool bShowInline = false;
                            bool bShowInlineHeader = false;
                            bool bShowInlineAlias = false;
                            int inlineValue = -1;
                            for ( QStringList::Iterator it3 = optionAttribute.begin(); it3 != optionAttribute.end(); ++it3 )
                            {
                                parseAttribute( *it3 , bShowEmptyMenu, bShowInline, bShowInlineHeader, bShowInlineAlias , inlineValue );
                                group->setShowEmptyMenu( bShowEmptyMenu );
                                group->setAllowInline( bShowInline );
                                group->setShowInlineHeader( bShowInlineHeader );
                                group->setInlineAlias( bShowInlineAlias );
                                group->setInlineValue( inlineValue );
                            }
                        }
                        else
                            it--;

                        addItem(sorted, (group), needSeparator);
                    }
                    break;
                }
            }
        }
        else
        {
            for (List::ConstIterator it2(group->m_serviceList.begin()); it2 != group->m_serviceList.end(); ++it2)
            {
                if (!(*it2)->isType(KST_KService))
                    continue;
                KService *service = (KService *)((KSycocaEntry *)(*it2));
                if (service->menuId() == item)
                {
                    if (!excludeNoDisplay || !service->noDisplay())
                        addItem(sorted, (*it2), needSeparator);
                    break;
                }
            }
        }
    }

    return sorted;
}

void KServiceGroup::parseAttribute( const QString &item ,  bool &showEmptyMenu, bool &showInline, bool &showInlineHeader, bool & showInlineAlias , int &inlineValue )
{
    if( item == "ME") //menu empty
        showEmptyMenu=true;
    else if ( item == "NME") //not menu empty
        showEmptyMenu=false;
    else if( item == "I") //inline menu !
        showInline = true;
    else if ( item == "NI") //not inline menu!
        showInline = false;
    else if( item == "IH") //inline  header!
        showInlineHeader= true;
    else if ( item == "NIH") //not inline  header!
        showInlineHeader = false;
    else if( item == "IA") //inline alias!
        showInlineAlias = true;
    else if (  item == "NIA") //not inline alias!
        showInlineAlias = false;
    else if( ( item ).contains( "IL" )) //inline limite!
    {
        QString tmp( item );
        tmp = tmp.remove( "IL[" );
        tmp = tmp.remove( "]" );
        bool ok;
        int _inlineValue = tmp.toInt(&ok);
        if ( !ok ) //error
            _inlineValue = -1;
        inlineValue =  _inlineValue;
    }
    else
        kdDebug()<<" This attribute is not supported :"<<item<<endl;
}

void KServiceGroup::setLayoutInfo(const QStringList &layout)
{
    d->sortOrder = layout;
}

QStringList KServiceGroup::layoutInfo() const
{
    return d->sortOrder;
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

KServiceGroup::Ptr
KServiceGroup::childGroup(const QString &parent)
{
   return KServiceGroupFactory::self()->findGroupByDesktopPath("#parent#"+parent, true);
}

QString
KServiceGroup::directoryEntryPath() const
{
   return d->directoryEntryPath;
}


void KServiceGroup::virtual_hook( int id, void* data )
{ KSycocaEntry::virtual_hook( id, data ); }


KServiceSeparator::KServiceSeparator( )
 : KSycocaEntry("separator")
{
}
