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

#ifndef __k_build_protocol_info_factory_h__
#define __k_build_protocol_info_factory_h__

#include <kprotocolinfofactory.h>
#include <assert.h>
#include <qstringlist.h>

/**
 * Protocol Info factory for building ksycoca
 * @internal
 */
class KBuildProtocolInfoFactory : public KProtocolInfoFactory
{
public:
  /**
   * Create factory
   */
  KBuildProtocolInfoFactory();
  
  virtual ~KBuildProtocolInfoFactory();

  /**
   * Create new entry.
   */
  virtual KProtocolInfo* createEntry(const QString &, const char *);

  virtual KProtocolInfo* createEntry(int) { assert(0); return 0L; }

  /**
   * Add a new entry
   */
  virtual void addEntry( KSycocaEntry *newEntry, const char *resource );
  
  /**
   * Returns all resource types for this service factory
   */  
  static QStringList resourceTypes();
};

#endif
