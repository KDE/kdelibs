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

    /**
     * Destructor.
     */
    virtual ~KInstance();

    /**
     *  Returns the application standard dirs object.
     */
    KStandardDirs	*dirs() const;

    /**
     *  Returns the general config object.
     */
    KConfig            *config() const;

    /**
     *  Returns an iconloader object.
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
     */
    const KAboutData *aboutData() const;

    /**
     * @return the instance name
     */
    QCString          instanceName() const;

    /**
     * @return the KMimeSourceFactory set as default for this application.
     *
     * Mainly added for API completeness and future extensibility.
     */
    KMimeSourceFactory* mimeSourceFactory () const;

protected:
    /**
     *  Copy Constructor is not allowed
     */
    KInstance( const KInstance& );

private:
    mutable KStandardDirs       *_dirs;

    mutable KConfig             *_config;
    mutable KIconLoader         *_iconLoader;

    QCString                     _name;
    const KAboutData            *_aboutData;

    KInstancePrivate *d;
};

#endif

