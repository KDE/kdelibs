/*
 *
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
 *
 * $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
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

#include "autostart.h"

#include <kconfig.h>
#include <kdesktopfile.h>
#include <kglobal.h>
#include <kstandarddirs.h>

class AutoStartItem
{
public:
   QString name;
   QString service;
   QString startAfter;
   int     phase;
};

class AutoStartList: public QPtrList<AutoStartItem>
{
public:
   AutoStartList() { }
};

AutoStart::AutoStart()
  : m_phase(1)
{
  m_startList = new AutoStartList;
  m_startList->setAutoDelete(true);
  KGlobal::dirs()->addResourceType("autostart", "share/autostart");
}

AutoStart::~AutoStart()
{
	delete m_startList;
}

void
AutoStart::setPhase(int phase)
{
   if (phase > m_phase)
      m_phase = phase;
}

static QString extractName(QString path)
{
  int i = path.findRev('/');
  if (i >= 0)
     path = path.mid(i+1);
  i = path.findRev('.');
  if (i >= 0)
     path = path.left(i);
  return path;
}

static bool startCondition(const QString &condition)
{
  if (condition.isEmpty())
     return true;

  QStringList list = QStringList::split(':', condition);
  if (list.count() < 4) 
     return true;

  KConfig config(list[0]);
  config.setGroup(list[1]);

  bool defaultValue = (list[3].lower() == "true");

  return config.readBoolEntry(list[2], defaultValue);
}

void
AutoStart::loadAutoStartList()
{
   QStringList files = KGlobal::dirs()->findAllResources("autostart", "*.desktop", false, true);
   
   for(QStringList::ConstIterator it = files.begin();
       it != files.end();
       ++it)
   {
       KDesktopFile config(*it, true);
       if (!startCondition(config.readEntry("X-KDE-autostart-condition")))
          continue;
       if (!config.tryExec())
          continue;
       if (config.readBoolEntry("Hidden", false))
          continue;
       
       AutoStartItem *item = new AutoStartItem;
       item->name = extractName(*it);
       item->service = *it;
       item->startAfter = config.readEntry("X-KDE-autostart-after");
       item->phase = config.readNumEntry("X-KDE-autostart-phase", 1);
       if (item->phase < 1)
          item->phase = 1;
       m_startList->append(item);
   }
} 

QString
AutoStart::startService()
{
   if (m_startList->isEmpty())
      return 0;

   while(!m_started.isEmpty())
   {

     // Check for items that depend on previously started items
     QString lastItem = m_started[0];
     for(AutoStartItem *item = m_startList->first(); 
         item; item = m_startList->next())
     {
        if (item->phase == m_phase
        &&  item->startAfter == lastItem)
        {
           m_started.prepend(item->name);
           QString service = item->service;
           m_startList->remove();
           return service;
        }
     }
     m_started.remove(m_started.begin());
   }
   
   // Check for items that don't depend on anything
   AutoStartItem *item;
   for(item = m_startList->first();
       item; item = m_startList->next())
   {
      if (item->phase == m_phase
      &&  item->startAfter.isEmpty())
      {
         m_started.prepend(item->name);
         QString service = item->service;
         m_startList->remove();
         return service;
      }
   }

   // Just start something in this phase
   for(item = m_startList->first();
       item; item = m_startList->next())
   {
      if (item->phase == m_phase)
      {
         m_started.prepend(item->name);
         QString service = item->service;
         m_startList->remove();
         return service;
      }
   }

   return 0;
}
