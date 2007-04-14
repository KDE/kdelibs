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

#include "kglobal.h"

#include <QtCore/QList>
#include <QtCore/QSet>

#include <kaboutdata.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <QtCore/QCoreApplication>
#include "kstaticdeleter.h"
#include "kcmdlineargs.h"

#ifndef NDEBUG
#define MYASSERT(x) if (!x) \
   qFatal("Fatal error: you need to have a KComponentData object before\n" \
         "you do anything that requires it! Examples of this are config\n" \
         "objects, standard directories or translations.");
#else
#define MYASSERT(x) /* nope */
#endif

typedef QSet<QString> KStringDict;
typedef QList<KStaticDeleterBase *> KStaticDeleterList;

class KGlobalPrivate
{
    public:
        inline KGlobalPrivate()
            : mainComponentPtr(0),
            stringDict(0),
            locale(0),
            charsets(0),
            staticDeleters(new KStaticDeleterList)
        {
        }

        inline ~KGlobalPrivate()
        {
            delete locale;
            locale = 0;
            delete charsets;
            charsets = 0;
            delete stringDict;
            stringDict = 0;
            mainComponentPtr = 0;

            deleteStaticDeleters();
        }

        void deleteStaticDeleters();

        KComponentData activeComponent;
        KComponentData mainComponent; // holds a refcount
        KComponentData* mainComponentPtr; // remembers the address of the first created componentdata (usually in main()).
        KStringDict *stringDict;
        KLocale *locale;
        KCharsets *charsets;
        KStaticDeleterList *staticDeleters;
};

void KGlobalPrivate::deleteStaticDeleters()
{
    if (!staticDeleters) {
        return;
    }

    while (!staticDeleters->isEmpty()) {
        staticDeleters->takeLast()->destructObject();
    }

    delete staticDeleters;
    staticDeleters = 0;
}

K_GLOBAL_STATIC(KGlobalPrivate, globalData)

#define PRIVATE_DATA KGlobalPrivate *d = globalData

KStandardDirs *KGlobal::dirs()
{
    PRIVATE_DATA;
    MYASSERT(d->mainComponent.isValid());
    return d->mainComponent.dirs();
}

KSharedConfig::Ptr KGlobal::config()
{
    PRIVATE_DATA;
    MYASSERT(d->mainComponent.isValid());
    return d->mainComponent.config();
}

const KComponentData &KGlobal::mainComponent()
{
    PRIVATE_DATA;
    MYASSERT(d->mainComponent.isValid());
    return d->mainComponent;
}

bool KGlobal::hasMainComponent()
{
    if (globalData.isDestroyed()) {
        return false;
    }
    PRIVATE_DATA;
    return d->mainComponent.isValid();
}

KLocale *KGlobal::locale()
{
    PRIVATE_DATA;
    if (d->locale == 0) {
        if (!d->mainComponent.isValid()) {
            return 0;
        }

        // will set d->locale if it works - otherwise 0 is returned
        KLocale::initInstance();
        if (d->mainComponent.aboutData()) {
            d->mainComponent.aboutData()->translateInternalProgramName();
        }
    }

    return d->locale;
}

bool KGlobal::hasLocale()
{
    if (globalData.isDestroyed()) {
        return false;
    }
    PRIVATE_DATA;
    return (d->locale != 0);
}

KCharsets *KGlobal::charsets()
{
    PRIVATE_DATA;
    if (d->charsets == 0) {
        d->charsets = new KCharsets;
    }

    return d->charsets;
}

const KComponentData &KGlobal::activeComponent()
{
    PRIVATE_DATA;
    MYASSERT(d->activeComponent.isValid());
    return d->activeComponent;
}

void KGlobal::setActiveComponent(const KComponentData &c)
{
    PRIVATE_DATA;
    d->activeComponent = c;
    if (c.isValid() && d->locale) {
        d->locale->setActiveCatalog(QString::fromUtf8(c.componentName()));
    }
}

void KGlobal::newComponentData(KComponentData *c)
{
    PRIVATE_DATA;
    if (d->mainComponent.isValid()) {
        return;
    }
    d->mainComponent = *c;
    d->mainComponentPtr = c;
    KGlobal::setActiveComponent(*c);
}

void KGlobal::deletedComponentData(KComponentData *c)
{
    if (globalData.isDestroyed()) {
        return;
    }
    PRIVATE_DATA;
    if (d->mainComponentPtr == c) {
        // The main component data (usually the one created on the stack in main())
        // just got deleted. We better sync the global kconfig before the qt globals
        // go away and prevent us from using things that require qt globals, like QTemporaryFile.
        d->mainComponentPtr = 0;
        if (c->privateConfig()) {
            c->privateConfig()->sync();
        }
    }
}

void KGlobal::setLocale(KLocale *locale)
{
    PRIVATE_DATA;
    d->locale = locale;
}

/**
 * Create a static QString
 *
 * To be used inside functions(!) like:
 * static const QString &myString = KGlobal::staticQString("myText");
 */
const QString &KGlobal::staticQString(const char *str)
{
    return staticQString(QLatin1String(str));
}

/**
 * Create a static QString
 *
 * To be used inside functions(!) like:
 * static const QString &myString = KGlobal::staticQString(i18n("My Text"));
 */
const QString &KGlobal::staticQString(const QString &str)
{
    PRIVATE_DATA;
    if (!d->stringDict) {
        d->stringDict = new KStringDict;
    }

   return *d->stringDict->insert(str);
}

void KGlobal::registerStaticDeleter(KStaticDeleterBase *obj)
{
    PRIVATE_DATA;
    Q_ASSERT(d->staticDeleters);
    if (d->staticDeleters->indexOf(obj) == -1) {
        d->staticDeleters->append(obj);
    }
}

void KGlobal::unregisterStaticDeleter(KStaticDeleterBase *obj)
{
    if (globalData.isDestroyed()) {
        return;
    }
    PRIVATE_DATA;
    if (d->staticDeleters) {
        d->staticDeleters->removeAll(obj);
    }
}

void KGlobal::deleteStaticDeleters()
{
    PRIVATE_DATA;
    d->deleteStaticDeleters();
}

QString KGlobal::caption()
{
    PRIVATE_DATA;
    // Caption set from command line ?
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
    if (args && args->isSet("caption")) {
        return QString::fromLocal8Bit(args->getOption("caption"));
    } else {
        // We have some about data ?
        if (d->mainComponent.aboutData()) {
            return d->mainComponent.aboutData()->programName();
        } else {
            // Last resort : application name
            return QCoreApplication::instance()->applicationName();
        }
    }
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
    if (s_refCount <= 0) {
        QCoreApplication::instance()->quit();
    }
}

#undef PRIVATE_DATA
