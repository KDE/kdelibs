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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kctimefactory.h"
#include "ksycoca.h"
#include "ksycocatype.h"

#include <assert.h>

KCTimeInfo::KCTimeInfo()
 : KSycocaFactory( KST_CTimeInfo ), ctimeDict(977)
{
   ctimeDict.setAutoDelete(true);
   if (m_str)
   {
      (*m_str) >> m_dictOffset;
   }
   else
   {
      m_dictOffset = 0;
   }
}

KCTimeInfo::~KCTimeInfo()
{
}

void 
KCTimeInfo::saveHeader(QDataStream &str)
{
  KSycocaFactory::saveHeader(str);

  str << m_dictOffset;
}

void
KCTimeInfo::save(QDataStream &str)
{
  KSycocaFactory::save(str);

  m_dictOffset = str.device()->at();
  QDictIterator<Q_UINT32> it(ctimeDict);
  while( it.current())
  {
     str << it.currentKey() << *(it.current());
     ++it;
  }   
  str << QString::null << (Q_UINT32) 0;

  int endOfFactoryData = str.device()->at();

  saveHeader(str);
  str.device()->at(endOfFactoryData);
}

void 
KCTimeInfo::addCTime(const QString &path, Q_UINT32 ctime)
{
  assert(!path.isEmpty());
  ctimeDict.replace(path, new Q_UINT32(ctime));
}

Q_UINT32
KCTimeInfo::ctime(const QString &path)
{
  Q_UINT32 *ctimeP = ctimeDict[path];
  return ctimeP ? *ctimeP : 0;
}

void
KCTimeInfo::fillCTimeDict(QDict<Q_UINT32> &dict)
{
    assert(m_str);
    m_str->device()->at(m_dictOffset);
    QString path;
    Q_UINT32 ctime;
    while(true)
    {
      KSycocaEntry::read(*m_str, path);
      (*m_str) >> ctime;
      if (path.isEmpty()) break;
      dict.replace(path, new Q_UINT32(ctime));
    }
}
