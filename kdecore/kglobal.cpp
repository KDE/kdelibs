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
/*
* kglobal.cpp -- Implementation of class KGlobal.
* Author:	Sirtaj Singh Kang
* Version:	$Id$
* Generated:	Sat May  1 02:08:43 EST 1999
*/

#include <qglobal.h>
#include <qdict.h>
#include <qptrlist.h>
#include "kglobal.h"

#include <kapplication.h>

#include <kconfig.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kinstance.h>
#include "kstaticdeleter.h"

#include <qfont.h>

#ifndef NDEBUG
#define MYASSERT(x) if (!x) \
   qFatal("Fatal error: you need to have a KInstance object before\n" \
         "you do anything that requires it! Examples of this are config\n" \
         "objects, standard directories or translations.");
#else
#define MYASSERT(x) /* nope */
#endif

static void kglobal_init();

KStandardDirs *KGlobal::dirs()
{
    MYASSERT(_instance);

    return _instance->dirs();
}

KConfig	*KGlobal::config()
{
    MYASSERT(_instance);

    return _instance->config();
}

KIconLoader *KGlobal::iconLoader()
{
    MYASSERT(_instance);

    return _instance->iconLoader();
}

KInstance *KGlobal::instance()
{
    MYASSERT(_instance);
    return _instance;
}

KLocale	*KGlobal::locale()
{
    if( _locale == 0 ) {
	if (!_instance)
	   return 0;
        kglobal_init();

        // will set _locale if it works - otherwise 0 is returned
        KLocale::initInstance();
    }

    return _locale;
}

KCharsets *KGlobal::charsets()
{
    if( _charsets == 0 ) {
        _charsets =new KCharsets();
        kglobal_init();
    }

    return _charsets;
}



/**
 * Create a static QString
 *
 * To be used inside functions(!) like:
 * static const QString &myString = KGlobal::staticQString("myText");
 */
const QString &
KGlobal::staticQString(const char *str)
{
   return staticQString(QString::fromLatin1(str));
}

class KStringDict : public QDict<QString>
{
public:
   KStringDict() : QDict<QString>() { };
};

/**
 * Create a static QString
 *
 * To be used inside functions(!) like:
 * static const QString &myString = KGlobal::staticQString(i18n("My Text"));
 */
const QString &
KGlobal::staticQString(const QString &str)
{
    if (!_stringDict) {
      _stringDict = new KStringDict;
      _stringDict->setAutoDelete( true );
      kglobal_init();
    }
   QString *result = _stringDict->find(str);
   if (!result)
   {
      result = new QString(str);
      _stringDict->insert(str, result);
   }
   return *result;
}

class KStaticDeleterList: public QPtrList<KStaticDeleterBase>
{
public:
   KStaticDeleterList() { };
};

void
KGlobal::registerStaticDeleter(KStaticDeleterBase *obj)
{
   if (!_staticDeleters)
      kglobal_init();
   if (_staticDeleters->find(obj) == -1)
      _staticDeleters->append(obj);
}

void
KGlobal::unregisterStaticDeleter(KStaticDeleterBase *obj)
{
   if (_staticDeleters)
      _staticDeleters->removeRef(obj);
}

void
KGlobal::deleteStaticDeleters()
{
    if (!KGlobal::_staticDeleters)
        return;

    for(KStaticDeleterBase *ptr = KGlobal::_staticDeleters->first();
	ptr;
	ptr = KGlobal::_staticDeleters->next())
    {
        ptr->destructObject();
    }
    delete KGlobal::_staticDeleters;
    KGlobal::_staticDeleters = 0;
}

// The Variables

KStringDict     *KGlobal::_stringDict   = 0;
KInstance       *KGlobal::_instance     = 0;
KInstance       *KGlobal::_activeInstance = 0;
KLocale         *KGlobal::_locale	= 0;
KCharsets       *KGlobal::_charsets	= 0;
KStaticDeleterList *KGlobal::_staticDeleters = 0;

static void kglobal_freeAll()
{
    delete KGlobal::_locale;
    KGlobal::_locale = 0;
    delete KGlobal::_charsets;
    KGlobal::_charsets = 0;
    delete KGlobal::_stringDict;
    KGlobal::_stringDict = 0;
    KGlobal::deleteStaticDeleters();
    // so that we don't hold a reference and see memory leaks :/
    KGlobal::_activeInstance = 0;
}

static bool addedFreeAll = false;

static void kglobal_init()
{
    if (addedFreeAll)
        return;

    addedFreeAll = true;
    KGlobal::_staticDeleters = new KStaticDeleterList;

    qAddPostRoutine( kglobal_freeAll );
}
