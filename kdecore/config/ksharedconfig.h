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

#include "kconfig.h"

class KSharedConfigPtr;

/**
 * KConfig variant using shared memory
 *
 * KSharedConfig provides a reference counted, shared memory variant
 * of KConfig.
 */
class KDECORE_EXPORT KSharedConfig : public KConfig, public QSharedData
{
public:
  typedef KSharedConfigPtr Ptr;

public:
  /**
   * Constructs a KConfig object.
   *
   * @param fileName A file to parse in addition to the
   *        system-wide file(s).  If it is not provided, only global
   *        KDE configuration data will be read (depending on the value of
   *        @p bUseKDEGlobals).
   * @param bReadOnly Set the config object's read-only status. Note that the
   *        object will automatically become read-only if either the user does not have
   *        write permission to @p fileName or if no file was specified.
   * @param bUseKDEGlobals Toggle reading the global KDE configuration file.
   * @param resType the place to look in (config, data, etc) See KStandardDirs.
   */
   static KSharedConfig::Ptr openConfig(const QString& fileName = QString(),
                                        KConfig::OpenFlags = KConfig::IncludeGlobals,
                                        const char *resType = "config");

   static KSharedConfig::Ptr openConfig(const KComponentData &componentData,
                                        const QString &fileName = QString(),
                                        KConfig::OpenFlags = KConfig::IncludeGlobals,
                                        const char *resType = "config");

    ~KSharedConfig();

    QString group() const;
    KConfigGroup group(const QByteArray &groupName);
    const KConfigGroup group(const QByteArray &groupName) const;
    KConfigGroup group(const char* groupName);
    const KConfigGroup group(const char *groupName) const;
    KConfigGroup group(const QString& groupName);
    const KConfigGroup group(const QString& groupName) const;

private:
   KSharedConfig(const QString& fileName,
                 KConfig::OpenFlags,
                 const char *resType,
                 const KComponentData &componentData);
};

class KDECORE_EXPORT KSharedConfigPtr : public KSharedPtr<KSharedConfig>
{
public:
    /**
     * Creates a null pointer.
     */
    inline KSharedConfigPtr()
        : KSharedPtr<KSharedConfig>()
    {}

    /**
     * Creates a new pointer.
     * @param p the pointer
     */
    inline explicit KSharedConfigPtr(KSharedConfig *p)
        : KSharedPtr<KSharedConfig>(p)
    {}

    /**
     * Copies a pointer.
     * @param o the pointer to copy
     */
    inline KSharedConfigPtr(const KSharedConfigPtr& o)
        : KSharedPtr<KSharedConfig>(o)
    {}

    /**
     * Unreferences the object that this pointer points to. If it was
     * the last reference, the object will be deleted.
     */
    ~KSharedConfigPtr();

    inline KSharedConfigPtr &operator= ( const KSharedConfigPtr& o ) { attach(o.d); return *this; }
    inline KSharedConfigPtr &operator= ( KSharedConfig *p ) { attach(p); return *this; }

    void attach(KSharedConfig *p);
};

#endif // multiple inclusion guard
