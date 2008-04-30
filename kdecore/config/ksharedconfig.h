/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (C) 1997-1999 Matthias Kalle Dalheimer (kalle@kde.org)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef KSHAREDCONFIG_H
#define KSHAREDCONFIG_H

#include <kconfig.h>
#include <ksharedptr.h>

/**
 * \class KSharedConfig ksharedconfig.h <KSharedConfig>
 * 
 * KConfig variant using shared memory
 *
 * KSharedConfig provides a reference counted, shared memory variant
 * of KConfig.
 */
class KDECORE_EXPORT KSharedConfig : public KConfig, public QSharedData //krazy:exclude=dpointer (only for refcounting)
{
public:
  typedef KSharedPtr<KSharedConfig> Ptr;

public:
  /**
   * Constructs a KSharedConfig object.
   *
   * @param fileName A file to parse in addition to the
   *        system-wide file(s).  If it is not provided, only global
   *        KDE configuration data will be read (depending on the value of
   *        @p mode).
   * @param mode
   * @param resourceType the place to look in (config, data, etc) @see KStandardDirs.
   * @param backend
   */
   static KSharedConfig::Ptr openConfig(const QString& fileName = QString(),
                                        OpenFlags mode = FullConfig,
                                        const char *resourceType = "config");

   static KSharedConfig::Ptr openConfig(const KComponentData &componentData,
                                        const QString &fileName = QString(),
                                        OpenFlags mode = FullConfig,
                                        const char *resourceType = "config");

    virtual ~KSharedConfig();

private:
    virtual KConfigGroup groupImpl(const QByteArray& aGroup);
    virtual const KConfigGroup groupImpl(const QByteArray& aGroup) const;

    KSharedConfig(const KComponentData& componentData, const QString& file, OpenFlags mode,
                  const char* resourceType);
};

typedef KSharedConfig::Ptr KSharedConfigPtr;

#endif // multiple inclusion guard
