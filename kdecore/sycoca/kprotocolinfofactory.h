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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef kprotocolinfofactory_h
#define kprotocolinfofactory_h

#include "kprotocolinfo.h"

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <kurl.h>
#include <ksycocafactory.h>


/**
 * @internal
 *
 * KProtocolInfoFactory is a factory for getting
 * KProtocolInfo. The factory is a singleton
 * (only one instance can exist).
 *
 * @short Factory for KProtocolInfo
 *
 * Exported for kbuildsycoca, but not installed.
 */
class KDECORE_EXPORT KProtocolInfoFactory : public KSycocaFactory
{
  K_SYCOCAFACTORY( KST_KProtocolInfoFactory )
public:
  /**
   * The instance of the KProtocolInfoFactory.
   * @return the factory instance
   */
  static KProtocolInfoFactory* self();

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
  KProtocolInfo::Ptr findProtocol(const QString &protocol);

  /**
   * @return all protocols
   */
  KProtocolInfo::List allProtocols() const;

  /**
   * Returns list of all known protocols.
   * @return a list of all protocols
   */
  QStringList protocols() const;

protected:
  /**
   * @internal Not used.
   */
  virtual KSycocaEntry *createEntry(const QString &, const char *) const
    { return 0; }

  /**
   * @internal
   */
  virtual KProtocolInfo *createEntry(int offset) const;

protected:
  /** Virtual hook, used to add new "virtual" functions while maintaining
      binary compatibility. Unused in this class.
  */
  virtual void virtual_hook( int id, void* data );
private:
  QMap<QString,KProtocolInfo::Ptr> m_cache;
  class KProtocolInfoFactoryPrivate* d;
};

#endif
