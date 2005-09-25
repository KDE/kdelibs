/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000,2003 Waldo Bastian <bastian@kde.org>
   

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef __kprotocolinfofactory_h__
#define __kprotocolinfofactory_h__

#include "../kio/kio/kprotocolinfo.h"

#include <q3valuelist.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <kurl.h>
#include <ksycocafactory.h>

class KProtocolInfoPrivate;

/**
 * KProtocolInfoFactory is a factory for getting 
 * KProtocolInfo. The factory is a singleton 
 * (only one instance can exist).
 *
 * @short Factory for KProtocolInfo
 */
class KDECORE_EXPORT KProtocolInfoFactory : public KSycocaFactory
{ 
  K_SYCOCAFACTORY( KST_KProtocolInfoFactory )
public:
  /**
   * The instance of the KProtocolInfoFactory.
   * @return the factory instance
   */
  static KProtocolInfoFactory* self() 
  { if ( !_self) new KProtocolInfoFactory(); return _self; }
  /** \internal */
  KProtocolInfoFactory();
  virtual ~KProtocolInfoFactory();

  /*
   * Returns protocol info for @p protocol.
   *
   * Does not take proxy settings into account.
   * @param protocol the protocol to search for
   * @return the pointer to the KProtocolInfo, or 0 if not found
   */
  KProtocolInfo *findProtocol(const QString &protocol);

  /**
   * Returns list of all known protocols.
   * @return a list of all protocols
   */
  QStringList protocols();
protected:

  /**
   * @internal Not used.
   */
  virtual KSycocaEntry *createEntry(const QString &, const char *)
    { return 0; }                                                    

  /**
   * @internal
   */
  virtual KProtocolInfo *createEntry(int offset);

protected:
  static KProtocolInfoFactory *_self;
  
  QMap<QString,KProtocolInfo::Ptr> m_cache;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KProtocolInfoFactoryPrivate* d;
};

#endif
