/* This file is part of the KDE project
   Copyright (C) 2000 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __k_ctime_factory_h__
#define __k_ctime_factory_h__

#include <ksycocafactory.h>
#include <qdict.h>

/**
 * Service group factory for building ksycoca
 * @internal
 */
class KCTimeInfo : public KSycocaFactory
{
  K_SYCOCAFACTORY( KST_CTimeInfo )
public:
  /**
   * Create factory
   */
  KCTimeInfo();

  virtual ~KCTimeInfo();

  /**
   * Write out header information
   */
  virtual void saveHeader(QDataStream &str);

  /**
   * Write out data 
   */
  virtual void save(QDataStream &str);

  KSycocaEntry * createEntry(const QString &, const char *) { return 0; }
  KSycocaEntry * createEntry(int) { return 0; }
  
  void addCTime(const QString &path, Q_UINT32 ctime);

  Q_UINT32 ctime(const QString &path);

  void fillCTimeDict(QDict<Q_UINT32> &dict);

protected:
  QDict<Q_UINT32> ctimeDict;
  int m_dictOffset;
};

#endif
