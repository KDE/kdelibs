/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kanq <taj@kde.org>
   Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

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
 * kglobal.cpp -- Implementation of namespace KGlobal.
 * Author:	Sirtaj Singh Kang
 * Generated:	Sat May  1 02:08:43 EST 1999
 */

#undef KDE3_SUPPORT

#include "kglobal.h"
#include "kglobal_p.h"

#include <config.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <unistd.h> // umask() for mingw

#include <QtCore/QList>
#include <QtCore/QSet>

#include <kaboutdata.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QTextCodec>
#include "kcmdlineargs.h"
#include <unistd.h> // umask

#ifndef NDEBUG
#define MYASSERT(x) if (!x) \
   qFatal("Fatal error: you need to have a KComponentData object before\n" \
         "you do anything that requires it! Examples of this are config\n" \
         "objects, standard directories or translations.");
#else
#define MYASSERT(x) /* nope */
#endif

// ~KConfig needs qrand(). qrand() depends on a Q_GLOBAL_STATIC. With this Q_CONSTRUCTOR_FUNCTION we
// try to make qrand() live longer than any KConfig object.
Q_CONSTRUCTOR_FUNCTION(qrand)

typedef QSet<QString> KStringDict;
mode_t s_umsk;

class KGlobalPrivate
{
    public:
        inline KGlobalPrivate()
            : stringDict(0),
            locale(0),
            charsets(0)
        {
            // the umask is read here before any threads are created to avoid race conditions
            mode_t tmp = 0;
            s_umsk = umask(tmp);
            umask(s_umsk);
        }

        inline ~KGlobalPrivate()
        {
            delete locale;
            locale = 0;
            delete charsets;
            charsets = 0;
            delete stringDict;
            stringDict = 0;
        }

        KComponentData activeComponent;
        KComponentData mainComponent; // holds a refcount
        KStringDict *stringDict;
        KLocale *locale;
        KCharsets *charsets;
};

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

        d->locale = new KLocale(d->mainComponent.catalogName());
        QTextCodec::setCodecForLocale(d->locale->codecForEncoding());
        d->mainComponent.aboutData()->translateInternalProgramName();
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

mode_t KGlobal::umask()
{
    // Don't use PRIVATE_DATA here. This is called by ~KGlobalPrivate -> ~KConfig -> sync -> KSaveFile, so there's no KGlobalPrivate anymore.
    return s_umsk;
}

KComponentData KGlobal::activeComponent()
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
        d->locale->setActiveCatalog(c.catalogName());
    }
}

void KGlobal::newComponentData(const KComponentData &c)
{
    PRIVATE_DATA;
    if (d->mainComponent.isValid()) {
        return;
    }
    d->mainComponent = c;
    KGlobal::setActiveComponent(c);
}

void KGlobal::setLocale(KLocale *locale, CopyCatalogs copy)
{
    PRIVATE_DATA;
    if (copy == DoCopyCatalogs && d->locale)
        d->locale->copyCatalogsTo(locale);
    delete d->locale;
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

QString KGlobal::caption()
{
    PRIVATE_DATA;
    // Caption set from command line ?
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
    if (args && args->isSet("caption")) {
        return args->getOption("caption");
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
    //kDebug() << "KGlobal::ref() : refCount = " << s_refCount;
}

void KGlobal::deref()
{
    --s_refCount;
    //kDebug() << "KGlobal::deref() : refCount = " << s_refCount;
    if (s_refCount <= 0) {
        QCoreApplication::instance()->quit();
    }
}

#undef PRIVATE_DATA
