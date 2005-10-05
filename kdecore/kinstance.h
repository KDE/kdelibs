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
#ifndef _KINSTANCE_H
#define _KINSTANCE_H

#include "kdelibs_export.h"

class QByteArray;
class QString;
class KAboutData;
class KConfig;
class KIconLoader;
class KMimeSourceFactory;
class KSharedConfig;
class KStandardDirs;

/**
 * Access to KDE global objects for use in shared libraries.  In
 * practical terms, this class is used in KDE components.  This allows
 * components to store things that normally would be accessed by
 * KGlobal.
 *
 * @author Torben Weis
 */
class KDECORE_EXPORT KInstance
{
    friend class KStandardDirs;
    KConfig* privateConfig() const;

 public:
    /**
     *  Constructor.
     *  @param instanceName the name of the instance
     */
    KInstance( const QByteArray& instanceName) ;

    /**
     *  Constructor.
     *  When building a KInstance that is not your KApplication,
     *  make sure that the KAboutData and the KInstance have the same life time.
     *  You have to destroy both, since the instance doesn't own the about data.
     *  Don't build a KAboutData on the stack in this case !
     *  Building a KAboutData on the stack is only ok for usage with
     *  KCmdLineArgs and KApplication (not destroyed until the app exits).
     *  @param aboutData data about this instance (see KAboutData)
     */
    KInstance( const KAboutData * aboutData );

    /**
     * @internal
     * Only for K(Unique)Application
     * Initialize from src and delete it.
     */

    KInstance( KInstance* src );

    /**
     * Destructor.
     */
    virtual ~KInstance();

    /**
     * Returns the application standard dirs object.
     * @return The KStandardDirs of the application.
     */
    KStandardDirs*	dirs() const;

    /**
     * Returns the general config object ("appnamerc").
     * @return the KConfig object for the instance.
     */
    KConfig*            config() const;

    /**
     * Returns the general config object ("appnamerc").
     * @return the KConfig object for the instance.
     */
    KSharedConfig*      sharedConfig() const;

    /**
     *  Returns an iconloader object.
     * @return the iconloader object.
     */
    KIconLoader*        iconLoader() const;

    /**
     * Re-allocate the global iconloader.
     */
    void newIconLoader() const;

    /**
     *  Returns the about data of this instance
     *  Warning, can be 0L
     * @return the about data of the instance, or 0 if it has
     *         not been set yet
     */
    const KAboutData* aboutData() const;

    /**
     * Returns the name of the instance
     * @return the instance name, can be null if the KInstance has been
     *         created with a null name
     */
    QByteArray          instanceName() const;

    /**
     * Returns the KMimeSourceFactory of the instance.
     * Mainly added for API completeness and future extensibility.
     * @return the KMimeSourceFactory set as default for this application.
     */
    KMimeSourceFactory* mimeSourceFactory () const;

protected:
    /**
     * Set name of default config file.
     * @param name the name of the default config file
     * @since 3.1
     */
    void setConfigName(const QString &name);
    virtual void virtual_hook( int id, void* data );

private:
    // Copying is not allowed
    KInstance( const KInstance& );
    KInstance& operator=(const KInstance&);

    class Private;
    Private * d; // can't be const because of special KApplication constructor
};

#endif

