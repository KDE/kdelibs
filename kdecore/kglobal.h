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
 * Access the KDE global objects.
 *
 * @author Sirtaj Singh Kang (taj@kde.org)
 * @version $Id$
 */
class KGlobal
{
public:

    /**
     * Retrieve the global instance.  There is always at least
     * one instance of a component in one application (in most
     * cases the application itself).
     */
    static KInstance            *instance();

    /**
     *  Retrieve the application standard dirs object.
     */
    static KStandardDirs	*dirs();

    /**
     *  Retrieve the general config object.
     */
    static KConfig		*config();

    /**
     *  Retrieve an iconloader object.
     */
    static KIconLoader	        *iconLoader();

    static KLocale              *locale();
    static KCharsets	        *charsets();

    /**
     * Create a static QString
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
     * Create a static QString
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


class KStaticDeleterBase {
public:
    virtual void destructObject() = 0;
};

/**
 * little helper class to clean up static objects that are
 * held as pointer.
 *
 * A typical use is
 * static KStaticDeleter<MyClass> sd;
 *
 * MyClass::self() {
 *   if (!_self) { _self = sd.setObject(new MyClass()); }
 * }
 */
template<class type> class KStaticDeleter : public KStaticDeleterBase {
public:
    KStaticDeleter() { deleteit = 0; }
    /**
     * sets the object to delete and registers the object to be
     * deleted to KGlobal. if the given object is 0, the former
     * registration is unregistred
     **/
    type *setObject( type *obj) {
        deleteit = obj;
	if (obj)
            KGlobal::registerStaticDeleter(this);
	else
	    KGlobal::unregisterStaticDeleter(this);
        return obj;
    }
    virtual void destructObject() {
    	delete deleteit; deleteit = 0;
    }
    virtual ~KStaticDeleter() {
    	KGlobal::unregisterStaticDeleter(this);
    	delete deleteit; deleteit = 0;
    }
private:
    type *deleteit;
};

#endif // _KGLOBAL_H

