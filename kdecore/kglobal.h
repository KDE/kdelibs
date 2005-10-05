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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef _KGLOBAL_H
#define _KGLOBAL_H

#include "kdelibs_export.h"

class KInstance;
class KCharsets;
class KConfig;
class KSharedConfig;
class KIconLoader;
class KLocale;
class KStandardDirs;
class KStaticDeleterBase;
class KStaticDeleterList;
class KStringDict;
class QString;

/**
 * Access to the KDE global objects.
 * KGlobal provides you with pointers of many central
 * objects that exist only once in the process. It is also
 * responsible for managing instances of KStaticDeleterBase.
 *
 * @see KStaticDeleterBase
 * @author Sirtaj Singh Kang (taj@kde.org)
 */
class KDECORE_EXPORT KGlobal
{
public:

    /**
     * Returns the global instance.  There is always at least
     * one instance of a component in one application (in most
     * cases the application itself).
     * @return the global instance
     */
    static KInstance            *instance();

    /**
     *  Returns the application standard dirs object.
     * @return the global standard dir object
     */
    static KStandardDirs	*dirs();

    /**
     *  Returns the general config object.
     * @return the global configuration object.
     */
    static KConfig		*config();

    /**
     *  Returns the general config object.
     * @return the global configuration object.
     */
    static KSharedConfig        *sharedConfig();

    /**
     *  Returns an iconloader object.
     * @return the global iconloader object
     */
    static KIconLoader	        *iconLoader();

    /**
     * Returns the global locale object.
     * @return the global locale object
     */
    static KLocale              *locale();

    /**
     * The global charset manager.
     * @return the global charset manager
     */
    static KCharsets	        *charsets();

    /**
     * Creates a static QString.
     *
     * To be used inside functions(!) like:
     * @code
     * static const QString &myString = KGlobal::staticQString("myText");
     * @endcode
     *
     * @attention Do @b NOT use code such as:
     * @code
     * static QString myString = KGlobal::staticQString("myText");
     * @endcode
     * This creates a static object (instead of a static reference)
     * and as you know static objects are EVIL.
     * @param str the string to create
     * @return the static string
     */
    static const QString&        staticQString(const char *str);

    /**
     * Creates a static QString.
     *
     * To be used inside functions(!) like:
     * @code
     * static const QString &myString = KGlobal::staticQString(i18n("My Text"));
     * @endcode
     *
     * @attention Do @b NOT use code such as:
     * @code
     * static QString myString = KGlobal::staticQString(i18n("myText"));
     * @endcode
     * This creates a static object (instead of a static reference)
     * and as you know static objects are EVIL.
     * @param str the string to create
     * @return the static string
     */
    static const QString&        staticQString(const QString &str);

    /**
     * Registers a static deleter.
     * @param d the static deleter to register
     * @see KStaticDeleterBase
     * @see KStaticDeleter
     */
    static void registerStaticDeleter(KStaticDeleterBase *d);

    /**
     * Unregisters a static deleter.
     * @param d the static deleter to unregister
     * @see KStaticDeleterBase
     * @see KStaticDeleter
     */
    static void unregisterStaticDeleter(KStaticDeleterBase *d);

    /**
     * Calls KStaticDeleterBase::destructObject() on all
     * registered static deleters and unregisters them all.
     * @see KStaticDeleterBase
     * @see KStaticDeleter
     */
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
     * @internal
     */
    static void setActiveInstance(KInstance *d);
    static KInstance *activeInstance() { return _activeInstance; }

    static  KInstance           *_activeInstance;
};

#ifdef QT3_SUPPORT
/**
 * \relates KGlobal
 * A typesafe function to find the smaller of the two arguments.
 * @deprecated, used qMin instead
 */
#define KMIN(a,b)	qMin(a,b)
/**
 * \relates KGlobal
 * A typesafe function to find the larger of the two arguments.
 * @deprecated, used qMax instead
 */
#define KMAX(a,b)	qMax(a,b)
/**
 * \relates KGlobal
 * A typesafe function to determine the absolute value of the argument.
 * @deprecated, used qAbs instead
 */
#define KABS(a)	qAbs(a)
/**
 * \relates KGlobal
 * A typesafe function that returns x if it's between low and high values.
 * low if x is smaller than low and high if x is bigger than high.
 * @deprecated, used qBound instead. Warning, the argument order differs.
 */
#define KCLAMP(x,low,high) qBound(low,x,high)

#define kMin qMin
#define kMax qMax
#define kAbs qAbs

/**
 * \relates KGlobal
 * A typesafe function that returns x if it's between low and high values.
 * low if x is smaller than low and high if x is bigger than high.
 * @deprecated, used qBound instead. Warning, the argument order differs.
 */

template<class T>
inline KDE_DEPRECATED T kClamp( const T& x, const T& low, const T& high )
{
    if ( x < low )       return low;
    else if ( high < x ) return high;
                         return x;
}

#endif

/**
 * Locale-independent qstricmp. Use this for comparing ascii keywords
 * in a case-insensitive way.
 * qstricmp fails with e.g. the Turkish locale where 'I'.toLower() != 'i'
 * @since 3.4
 */
int KDECORE_EXPORT kasciistricmp( const char *str1, const char *str2 );


/**
 * \mainpage The KDE Core Functionality Library
 *
 * All KDE programs use this library to provide basic functionality such
 * as the configuration system, IPC, internationalization and locale
 * support, site-independent access to the filesystem and a large number
 * of other (but no less important) things.
 *
 * All KDE applications should link to the kdecore library. Also, using a
 * KApplication derived class instead of QApplication is almost
 * mandatory if you expect your application to behave nicely within the
 * KDE environment.
 */

#endif // _KGLOBAL_H

