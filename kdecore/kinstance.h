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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef _KINSTANCE_H
#define _KINSTANCE_H

class KStandardDirs;
class KAboutData;
class KConfig;
class KIconLoader;
class KCharsets;
class QFont;
class KInstancePrivate;
class KMimeSourceFactory;

#include <qstring.h>


/**
 * Access to KDE global objects for use in shared libraries.  In
 * practical terms, this class is used in KDE components.  This allows
 * components to store things that normally would be accessed by
 * @ref KGlobal.
 *
 * @author Torben Weis
 * @version $Id$
 */
class KInstance
{
    friend class KStandardDirs;

 public:
    /**
     *  Constructor
     *  @param instanceName the name of the instance
     */
    KInstance( const QCString& instanceName) ;

    /**
     *  Constructor
     *  @param aboutData data about this instance (see @ref KAboutData)
     *
     *  When building a KInstance that is not your KApplication,
     *  make sure that the KAboutData and the KInstance have the same life time.
     *  You have to destroy both, since the instance doesn't own the about data.
     *  Don't build a KAboutData on the stack in this case !
     *  Building a KAboutData on the stack is only ok for usage with
     *  KCmdLineArgs and KApplication (not destroyed until the app exits).
     */
    KInstance( const KAboutData * aboutData );

    /*
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
    KStandardDirs	*dirs() const;

    /**
     * Returns the general config object ("appnamerc").
     * @return the KConfig object for the instance.
     */
    KConfig            *config() const;

    /**
     *  Returns an iconloader object.
     * @return the iconloader object.
     */
    KIconLoader	       *iconLoader() const;

    /**
     * Re-allocate the global iconloader.
     */
    void newIconLoader() const;

    KCharsets	     *charsets() const;

    /**
     *  Returns the about data of this instance
     *  Warning, can be 0L
     * @return the about data of the instance, or 0 if it has 
     *         not been set yet
     */
    const KAboutData *aboutData() const;

    /**
     * Returns the name of the instance
     * @return the instance name, can be null if the KInstance has been 
     *         created with a null name
     */
    QCString          instanceName() const;

    /**
     * Returns the @ref KMimeSourceFactory of the instance.
     * Mainly added for API completeness and future extensibility.
     * @return the KMimeSourceFactory set as default for this application.
     */
    KMimeSourceFactory* mimeSourceFactory () const;

protected:
    /**
     *  Copy Constructor is not allowed
     */
    KInstance( const KInstance& );

    /**
     * Set name of default config file.
     */
    void setConfigName(const QString &);

private:
    mutable KStandardDirs       *_dirs;

    mutable KConfig             *_config;
    mutable KIconLoader         *_iconLoader;

    QCString                     _name;
    const KAboutData            *_aboutData;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    KInstancePrivate *d;
};

#endif

