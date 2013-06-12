/* This file is part of the KDE libraries
   Copyright (C) 1999 Sirtaj Singh Kanq <taj@kde.org>
   Copyright (C) 2007 Matthias Kretz <kretz@kde.org>
   Copyright (C) 2009 Olivier Goffart <ogoffart@kde.org>

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
#include <QThread>

#include <config-kde4support.h>

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <QtCore/QList>
#include <QtCore/QSet>

#include <k4aboutdata.h>
#include <kconfig.h>
#include <klocale.h>
#include <klocalizedstring.h>
#include <kcharsets.h>
#include "kstandarddirs.h"
#include <kcomponentdata.h>
#undef QT_NO_TRANSLATION
#include <QtCore/QCoreApplication>
#define QT_NO_TRANSLATION
#include <QtCore/QDebug>
#include <QtCore/QTextCodec>
#include <unistd.h> // umask

// ~KConfig needs qrand(). qrand() depends on a Q_GLOBAL_STATIC. With this Q_CONSTRUCTOR_FUNCTION we
// try to make qrand() live longer than any KConfig object.
Q_CONSTRUCTOR_FUNCTION(qrand)

typedef QSet<QString> KStringDict;
mode_t s_umsk;

class KGlobalPrivate
{
    public:
        inline KGlobalPrivate()
            : dirs(0),
            stringDict(0)
        {
            // the umask is read here before any threads are created to avoid race conditions
            mode_t tmp = 0;
            s_umsk = umask(tmp);
            umask(s_umsk);
        }

        inline ~KGlobalPrivate()
        {
            delete dirs;
            dirs = 0;
            delete stringDict;
            stringDict = 0;
        }

        KStandardDirs *dirs;
        KStringDict *stringDict;
        QStringList catalogsToInsert;
};

KCatalogLoader::KCatalogLoader(const QString &catalogName)
{
    KLocalizedString::insertCatalog(catalogName);
}

K_GLOBAL_STATIC(KGlobalPrivate, globalData)

#define PRIVATE_DATA KGlobalPrivate *d = globalData

KStandardDirs *KGlobal::dirs()
{
    PRIVATE_DATA;
    if (!d->dirs) {
        d->dirs = new KStandardDirs;
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        if (d->dirs->addCustomized(config.data())) {
            config->reparseConfiguration();
        }
    }
    return d->dirs;
}

KSharedConfig::Ptr KGlobal::config()
{
    return KComponentData::mainComponent().config();
}

const KComponentData &KGlobal::mainComponent()
{
    return KComponentData::mainComponent();
}

bool KGlobal::hasMainComponent()
{
    return KComponentData::hasMainComponent();
}

void KGlobal::insertCatalog(const QString& catalog)
{
    KLocalizedString::insertCatalog(catalog);
}

KLocale *KGlobal::locale()
{
    return KLocale::global();
}

bool KGlobal::hasLocale()
{
    return QCoreApplication::instance() != 0;
}

KCharsets *KGlobal::charsets()
{
    return KCharsets::charsets();
}

mode_t KGlobal::umask()
{
    // Don't use PRIVATE_DATA here. This is called by ~KGlobalPrivate -> ~KConfig -> sync -> KSaveFile, so there's no KGlobalPrivate anymore.
    return s_umsk;
}

KComponentData KGlobal::activeComponent()
{
    return KComponentData::activeComponent();
}

void KGlobal::setActiveComponent(const KComponentData &c)
{
    KComponentData::setActiveComponent(c);
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
#if 0 // Should move to Qt if wanted
    // Caption set from command line ?
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs("kde");
    if (args && args->isSet("caption")) {
        return args->getOption("caption");
    } else
#endif
    {
        // We have some about data ?
        const KComponentData& cData = KComponentData::mainComponent();
        if (cData.isValid() && cData.aboutData()) {
            return cData.aboutData()->programName();
        } else {
            // Last resort : application name
            return QCoreApplication::instance()->applicationName();
        }
    }
}

/**
 * This counter indicates when to quit the application.
 * It starts at 0, is incremented by KMainWindow, systray icons, running jobs, etc.
 * and decremented again when those things are destroyed.
 * This mechanism allows dialogs and jobs to outlive the last window closed
 * e.g. a file copy for a file manager, or 'compacting folders on exit' for a mail client,
 * the job progress widget with "keep open" checked, etc.
 */
static int s_refCount = 0;
static bool s_allowQuit = false;

void KGlobal::ref()
{
    ++s_refCount;
    //kDebug() << "KGlobal::ref() : refCount = " << s_refCount;
}

void KGlobal::deref()
{
    --s_refCount;
    //kDebug() << "KGlobal::deref() : refCount = " << s_refCount;
    if (s_refCount <= 0 && s_allowQuit) {
        QCoreApplication::instance()->quit();
    }
}

void KGlobal::setAllowQuit(bool allowQuit)
{
    s_allowQuit = allowQuit;
}

#undef PRIVATE_DATA

QObject* KGlobal::findDirectChild_helper(const QObject* parent, const QMetaObject& mo)
{
    if (!parent)
        return 0;

    const QObjectList &children = parent->children();
    for (int i = 0; i < children.size(); ++i) {
        QObject* obj = children.at(i);
        if (mo.cast(obj)) {
            return obj;
        }
    }
    return 0;

}
