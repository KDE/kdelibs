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

/*
 * kglobal.cpp -- Implementation of class KGlobal.
 * Author:	Sirtaj Singh Kang
 * Generated:	Sat May  1 02:08:43 EST 1999
 */

#undef KDE3_SUPPORT

#include <qglobal.h>
#include <QList>
#include <qset.h>
#include <qwindowdefs.h>
#include "kglobal.h"

#include <kaboutdata.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kstandarddirs.h>
#include <kinstance.h>
#include <qcoreapplication.h>
#include "kstaticdeleter.h"

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

KSharedConfig* KGlobal::sharedConfig()
{
    MYASSERT(_instance);

    return _instance->sharedConfig();
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
        if( _instance->aboutData())
            _instance->aboutData()->translateInternalProgramName();
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

void KGlobal::setActiveInstance(KInstance *i)
{
    _activeInstance = i;
    if (i && _locale)
	_locale->setActiveCatalog(QString::fromUtf8(i->instanceName()));
}

void KGlobal::setMainInstance( KInstance* i )
{
    _instance = i;
    setActiveInstance( i );
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
   return staticQString(QLatin1String(str));
}

class KStringDict : public QSet<QString>
{
public:
   KStringDict() { }
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
      kglobal_init();
    }

   return *_stringDict->insert(str);
}

class KStaticDeleterList: public QList<KStaticDeleterBase*>
{
public:
   KStaticDeleterList() { }
};

void
KGlobal::registerStaticDeleter(KStaticDeleterBase *obj)
{
   if (!_staticDeleters)
      kglobal_init();
   if (_staticDeleters->indexOf(obj) == -1)
      _staticDeleters->append(obj);
}

void
KGlobal::unregisterStaticDeleter(KStaticDeleterBase *obj)
{
   if (_staticDeleters)
      _staticDeleters->removeAll(obj);
}

void
KGlobal::deleteStaticDeleters()
{
    if (!KGlobal::_staticDeleters)
        return;

    for(;_staticDeleters->count();)
    {
        _staticDeleters->takeFirst()->destructObject();
    }

    delete KGlobal::_staticDeleters;
    KGlobal::_staticDeleters = 0;
}

/**
 * This counter indicates when to quit the application.
 * It starts at 1, is decremented in KMainWindow when the last window is closed, but
 * is incremented by operations that should outlive the last window closed
 * (e.g. a file copy for a file manager, or 'compacting folders on exit' for a mail client).
 */
static int s_refCount = 1;

void KGlobal::ref()
{
    ++s_refCount;
    //kDebug() << "KGlobal::ref() : refCount = " << s_refCount << endl;
}

void KGlobal::deref()
{
    --s_refCount;
    //kDebug() << "KGlobal::deref() : refCount = " << s_refCount << endl;
    if ( s_refCount <= 0 )
        QCoreApplication::instance()->quit();
}

// The Variables

KStringDict     *KGlobal::_stringDict   = 0;
KInstance       *KGlobal::_instance     = 0;
KInstance       *KGlobal::_activeInstance = 0;
KLocale         *KGlobal::_locale	= 0;
KCharsets       *KGlobal::_charsets	= 0;
KStaticDeleterList *KGlobal::_staticDeleters = 0;

#ifdef Q_OS_WIN
#include <windows.h>
static void kglobal_init();
static void kglobal_freeAll();
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID impLoad )
{
	if (reason == DLL_PROCESS_ATTACH) {
		kglobal_init();
	}
	if (reason == DLL_PROCESS_DETACH)
        kglobal_freeAll();
    return true;
}
# define ATTRIBUTE_DESTRUCTOR
#else
# define ATTRIBUTE_DESTRUCTOR __attribute__((destructor))
#endif
static ATTRIBUTE_DESTRUCTOR void kglobal_freeAll()
{
    delete KGlobal::_locale;
    KGlobal::_locale = 0;
    delete KGlobal::_charsets;
    KGlobal::_charsets = 0;
    delete KGlobal::_stringDict;
    KGlobal::_stringDict = 0;
    KGlobal::deleteStaticDeleters();
    // so that we don't hold a reference and see memory leaks :/
    KGlobal::setActiveInstance(0);
}

static void kglobal_init()
{
    if (KGlobal::_staticDeleters)
        return;

    KGlobal::_staticDeleters = new KStaticDeleterList;
}


