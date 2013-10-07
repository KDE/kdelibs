/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "autostart.h"

#include <kautostart.h>
#include <QStandardPaths>
#include <QDir>

class AutoStartItem
{
public:
   QString name;
   QString service;
   QString startAfter;
   int     phase;
};

AutoStart::AutoStart()
  : m_phase(-1), m_phasedone(false)
{
  m_startList = new AutoStartList;
}

AutoStart::~AutoStart()
{
  qDeleteAll(*m_startList);
  m_startList->clear();
  delete m_startList;
}

void
AutoStart::setPhase(int phase)
{
   if (phase > m_phase)
   {
      m_phase = phase;
      m_phasedone = false;
   }
}

void AutoStart::setPhaseDone()
{
   m_phasedone = true;
}

static QString extractName(QString path) // krazy:exclude=passbyvalue
{
  int i = path.lastIndexOf(QLatin1Char('/'));
  if (i >= 0)
     path = path.mid(i+1);
  i = path.lastIndexOf(QLatin1Char('.'));
  if (i >= 0)
     path = path.left(i);
  return path;
}

void
AutoStart::loadAutoStartList()
{
    // XDG autostart dirs

    // Make unique list of relative paths
    QStringList files;
    QStringList dirs = QStandardPaths::locateAll(QStandardPaths::ConfigLocation, QStringLiteral("autostart"), QStandardPaths::LocateDirectory);
    Q_FOREACH (const QString& dir, dirs) {
        Q_FOREACH (const QString& file, QDir(dir).entryList(QStringList() << QStringLiteral("*.desktop"))) {
            if (!files.contains(file))
                files.append(file);
        }
    }

   for (QStringList::ConstIterator it = files.constBegin(); it != files.constEnd(); ++it) {
       const QString file = QStandardPaths::locate(QStandardPaths::ConfigLocation, QStringLiteral("autostart/") + *it);
       KAutostart config(file);
       if (!config.autostarts(QStringLiteral("KDE"), KAutostart::CheckAll))
           continue;

       AutoStartItem *item = new AutoStartItem;
       item->name = extractName(*it);
       item->service = *it;
       item->startAfter = config.startAfter();
       item->phase = config.startPhase();
       if (item->phase < 0)
          item->phase = 0;
       m_startList->append(item);
   }
}

QString
AutoStart::startService()
{
   if (m_startList->isEmpty())
      return QString();

   while(!m_started.isEmpty())
   {

     // Check for items that depend on previously started items
     QString lastItem = m_started[0];
     QMutableListIterator<AutoStartItem *> it(*m_startList);
     while (it.hasNext())
     {
        AutoStartItem *item = it.next();
        if (item->phase == m_phase
        &&  item->startAfter == lastItem)
        {
           m_started.prepend(item->name);
           QString service = item->service;
           it.remove();
           delete item;
           return service;
        }
     }
     m_started.removeFirst();
   }

   // Check for items that don't depend on anything
   AutoStartItem *item;
   QMutableListIterator<AutoStartItem *> it(*m_startList);
   while (it.hasNext())
   {
      item = it.next();
      if (item->phase == m_phase
      &&  item->startAfter.isEmpty())
      {
         m_started.prepend(item->name);
         QString service = item->service;
         it.remove();
         delete item;
         return service;
      }
   }

   // Just start something in this phase
   it = *m_startList;
   while (it.hasNext())
   {
      item = it.next();
      if (item->phase == m_phase)
      {
         m_started.prepend(item->name);
         QString service = item->service;
         it.remove();
         delete item;
         return service;
      }
   }

   return QString();
}
