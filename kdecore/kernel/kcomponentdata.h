/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

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
#ifndef KCOMPONENTDATA_H
#define KCOMPONENTDATA_H

#include "kdelibs_export.h"
#include <kconfig.h>

class QByteArray;
class QString;
class KAboutData;
class KStandardDirs;
class KComponentDataPrivate;

/**
 * Access to KDE global objects for use in shared libraries.  In
 * practical terms, this class is used in KDE components.  This allows
 * components to store things that normally would be accessed by
 * KGlobal.
 *
 * @author Torben Weis
 */
class KDECORE_EXPORT KComponentData
{
    friend class KStandardDirs;
    KSharedConfig::Ptr &privateConfig() const;

public:
    KComponentData();
    KComponentData(const KComponentData&);
    KComponentData &operator=(const KComponentData&);
    bool operator==(const KComponentData&) const;
    bool operator!=(const KComponentData &rhs) const { return !operator==(rhs); }

    /**
     *  Constructor.
     *  @param componentName the name of the instance
     */
    explicit KComponentData(const QByteArray &componentName);

    /**
     *  Constructor.
     *  When building a KComponentData that is not your KApplication,
     *  make sure that the KAboutData and the KComponentData have the same life time.
     *  You have to destroy both, since the instance doesn't own the about data.
     *  Don't build a KAboutData on the stack in this case !
     *  Building a KAboutData on the stack is only ok for usage with
     *  KCmdLineArgs and KApplication (not destroyed until the app exits).
     *  @param aboutData data about this instance (see KAboutData)
     */
    explicit KComponentData(const KAboutData *aboutData);

    /**
     * Destructor.
     */
    virtual ~KComponentData();

    bool isValid() const;

    /**
     * Returns the application standard dirs object.
     * @return The KStandardDirs of the application.
     */
    KStandardDirs *dirs() const;

    /**
     * Returns the general config object ("appnamerc").
     * @return the KConfig object for the instance.
     */
    const KSharedConfig::Ptr &config() const;

    /**
     *  Returns the about data of this instance
     *  Warning, can be 0L
     * @return the about data of the instance, or 0 if it has
     *         not been set yet
     */
    const KAboutData *aboutData() const;

    /**
     * Returns the name of the instance
     * @return the instance name, can be null if the KComponentData has been
     *         created with a null name
     */
    QByteArray componentName() const;

protected:
    friend class KApplication;
    friend class KSharedConfigPtr;

    /**
     * Set name of default config file.
     * @param name the name of the default config file
     */
    void setConfigName(const QString &name);

    void _checkConfig();

    /** Standard trick to add virtuals later. @internal */
    virtual void virtual_hook( int id, void* data );

private:
    KComponentDataPrivate *d;
};

#endif // KCOMPONENTDATA_H
