/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kanq <taj@kde.org>

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
#ifndef _KGLOBAL_H
#define _KGLOBAL_H

#include <qstring.h>

class KStandardDirs;
class KConfig;
class KLocale;
class KIconLoader;
class KCharsets;
class QFont;
class KInstance;
class KStringDict;
class KStaticDeleterBase;
class KStaticDeleterList;


/**
 * Access to the KDE global objects.
 *
 * @author Sirtaj Singh Kang (taj@kde.org)
 * @version $Id$
 */
class KGlobal
{
public:

    /**
     * Returns the global instance.  There is always at least
     * one instance of a component in one application (in most
     * cases the application itself).
     */
    static KInstance            *instance();

    /**
     *  Returns the application standard dirs object.
     */
    static KStandardDirs	*dirs();

    /**
     *  Returns the general config object.
     */
    static KConfig		*config();

    /**
     *  Returns an iconloader object.
     */
    static KIconLoader	        *iconLoader();

    static KLocale              *locale();
    static KCharsets	        *charsets();

    /**
     * Creates a static QString.
     *
     * To be used inside functions(!) like:
     * static const QString &myString = KGlobal::staticQString("myText");
     *
     * !!! Do _NOT_ use: !!!
     * static QString myString = KGlobal::staticQString("myText");
     * This creates a static object (instead of a static reference)
     * and as you know static objects are EVIL.
     */
    static const QString        &staticQString(const char *);

    /**
     * Creates a static QString.
     *
     * To be used inside functions(!) like:
     * static const QString &myString = KGlobal::staticQString(i18n("My Text"));
     *
     * !!! Do _NOT_ use: !!!
     * static QString myString = KGlobal::staticQString(i18n("myText"));
     * This creates a static object (instead of a static reference)
     * and as you know static objects are EVIL.
     */
    static const QString        &staticQString(const QString &);

    static void registerStaticDeleter(KStaticDeleterBase *);
    static void unregisterStaticDeleter(KStaticDeleterBase *);

    static void deleteStaticDeleters();

    //private:
    static  KStringDict         *_stringDict;
    static  KInstance           *_instance;
    static  KLocale             *_locale;
    static  KCharsets	        *_charsets;
    static  KStaticDeleterList  *_staticDeleters;

    /**
     * The instance currently active (useful in a multi-instance
     * application, such as a KParts application).
     * Don't use this - it's mainly for KAboutDialog and KBugReport.
     */
    static  KInstance           *_activeInstance;
};

/** @relates KGlobal
 * A typesafe function to find the minimum of the two arguments.
 */
#define KMIN(a,b)	kMin(a,b)
/** @relates KGlobal
 * A typesafe function to find the maximum of the two arguments.
 */
#define KMAX(a,b)	kMax(a,b)


template<class T>
inline const T& kMin (const T& a, const T& b) { return a < b ? a : b; }

template<class T>
inline const T& kMax (const T& a, const T& b) { return b < a ? a : b; }

/**
 * @libdoc The KDE Core Functionality Library
 *
 * All KDE programs use this library to provide basic functionality such
 * as the configuration system, IPC, internationalization and locale
 * support, site-independent access to the filesystem and a large number
 * of other (but no less important) things.
 *
 * All KDE applications should link to the kdecore library. Also, using a
 * @ref KApplication derived class instead of @ref QApplication is almost
 * mandatory if you expect your application to behave nicely within the
 * KDE environment.
 */

#endif // _KGLOBAL_H

