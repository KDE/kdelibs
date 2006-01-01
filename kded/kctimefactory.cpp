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
 : KSycocaFactory( KST_CTimeInfo ), ctimeDict()
{
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

  m_dictOffset = str.device()->pos();
  Dict::const_iterator it = ctimeDict.begin();
  const Dict::const_iterator end = ctimeDict.end();
  for ( ; it != end; ++it )
  {
     str << it.key() << it.value();
  }
  str << QString() << (quint32) 0;

  int endOfFactoryData = str.device()->pos();

  saveHeader(str);
  str.device()->seek(endOfFactoryData);
}

void
KCTimeInfo::addCTime(const QString &path, quint32 ctime)
{
  assert(!path.isEmpty());
  ctimeDict.insert(path, ctime);
}

quint32
KCTimeInfo::ctime(const QString &path)
{
  return ctimeDict.value( path, 0 );
}

void
KCTimeInfo::fillCTimeDict(Dict &dict)
{
    assert(m_str);
    m_str->device()->seek(m_dictOffset);
    QString path;
    quint32 ctime;
    while(true)
    {
      KSycocaEntry::read(*m_str, path);
      (*m_str) >> ctime;
      if (path.isEmpty()) break;
      dict.insert(path, ctime);
    }
}
