/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000 Waldo Bastian <bastian@kde.org>
   

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef __kprotocolinfofactory_h__
#define __kprotocolinfofactory_h__

#include "kprotocolinfo.h"
#include "ksycocafactory.h"

#include <qstring.h>
#include <qstringlist.h>
#include <kurl.h>

class KProtocolInfoPrivate;

/**
 * KProtocolInfoFactory is a factory for getting 
 * @ref KProtocolInfo. The factory is a singleton 
 * (only one instance can exist).
 *
 * @short Factory for KProtocolInfo
 */
class KProtocolInfoFactory : public KSycocaFactory
{ 
  K_SYCOCAFACTORY( KST_KProtocolInfoFactory )
public:
  /**
   * The instance of the KProtocolInfoFactory.
   * @return the factory instance
   */
  static KProtocolInfoFactory* self() 
  { if ( !_self) new KProtocolInfoFactory(); return _self; }
  /// @internal
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

  /* 
   * Returns protocol info for @p url
   *
   * Does take proxy setings into account. If @p url is 
   * handled by a proxy, the protocol used for the proxy is
   * returned.
   * @param url the URL to search for
   * @return the pointer to the KProtocolInfo, or 0 if not found
   */
  KProtocolInfo *findProtocol(const KURL &url);

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
//  void scanConfig( const QString& _dir );

  static KProtocolInfoFactory *_self;
  
  QString m_lastProtocol;
  KProtocolInfo::Ptr m_lastInfo;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KProtocolInfoFactoryPrivate* d;
};

#endif
