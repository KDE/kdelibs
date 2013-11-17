/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2007 Matthias Kretz <kretz@kde.org>
   Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>

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

#include "kcomponentdata.h"
#include "kcomponentdata_p.h"

#include <QtCore/QCoreApplication>

#include "kaboutdata.h"
#include "k4aboutdata.h"
#include "kconfig.h"
#include "kconfiggroup.h"
#include <QtDebug>

#ifndef NDEBUG
#define MYASSERT(x) if (!x) \
   qFatal("Fatal error: you need to have a KComponentData object before\n" \
         "you do anything that requires it! Examples of this are config\n" \
         "objects, standard directories or translations.");
#else
#define MYASSERT(x) /* nope */
#endif

KComponentData::KComponentData()
    : d(0)
{
}

KComponentData::KComponentData(const KComponentData &rhs)
    : d(rhs.d)
{
    if (d) {
        d->ref();
    }
}

KComponentData &KComponentData::operator=(const KComponentData &rhs)
{
    if (rhs.d != d) {
        if (rhs.d) {
            rhs.d->ref();
        }
        if (d) {
            d->deref();
        }
        d = rhs.d;
    }
    return *this;
}

bool KComponentData::operator==(const KComponentData &rhs) const
{
    return d == rhs.d;
}

bool KComponentData::operator!=(const KComponentData &rhs) const
{
  return !operator==(rhs);
}

enum KdeLibraryPathsAdded {
    NeedLazyInit,
    LazyInitDone,
    KdeLibraryPathsAddedDone
};
static KdeLibraryPathsAdded kdeLibraryPathsAdded = NeedLazyInit;

class KComponentDataStatic
{
public:
    KComponentData mainComponent; // holds a refcount
    KComponentData activeComponent;

    void newComponentData(const KComponentData &c)
    {
        if (mainComponent.isValid()) {
            return;
        }
        mainComponent = c;
        KAboutData::setApplicationData(KAboutData(*c.aboutData()));
        KConfig::setMainConfigName(c.aboutData()->appName() + QLatin1String("rc"));
#if 0 // TEMP_KF5_REENABLE
        KLocale::setMainCatalog(c.catalogName());
#endif
        KComponentData::setActiveComponent(c);
    }
};


/**
 * This component may be used in applications that doesn't have a
 * main component (such as pure Qt applications).
 */
static KComponentData initFakeComponent()
{
    QString name = QCoreApplication::applicationName();
    if(name.isEmpty() && QCoreApplication::instance())
        name = qAppName();
    if(name.isEmpty())
        name = QString::fromLatin1("kde");
    return KComponentData(name.toLatin1(), name.toLatin1(),
                          KComponentData::SkipMainComponentRegistration);
}

Q_GLOBAL_STATIC(KComponentDataStatic, globalStatic)
Q_GLOBAL_STATIC_WITH_ARGS(KComponentData, fakeComponent, (initFakeComponent()))

KComponentData::KComponentData(const QByteArray &name, const QByteArray &catalog, MainComponentRegistration registerAsMain)
    : d(new KComponentDataPrivate(K4AboutData(name, catalog, KLocalizedString(), QByteArray())))
{
    Q_ASSERT(!name.isEmpty());

    if (kdeLibraryPathsAdded == NeedLazyInit) {
        kdeLibraryPathsAdded = LazyInitDone;
        d->lazyInit();
    }

    if (registerAsMain == RegisterAsMainComponent) {
        globalStatic()->newComponentData(*this);
    }
}

KComponentData::KComponentData(const K4AboutData *aboutData, MainComponentRegistration registerAsMain)
    : d(new KComponentDataPrivate(*aboutData))
{
    Q_ASSERT(!aboutData->appName().isEmpty());

    if (kdeLibraryPathsAdded == NeedLazyInit) {
        kdeLibraryPathsAdded = LazyInitDone;
        d->lazyInit();
    }

    if (registerAsMain == RegisterAsMainComponent) {
        globalStatic()->newComponentData(*this);
    }
}

KComponentData::KComponentData(const K4AboutData &aboutData, MainComponentRegistration registerAsMain)
    : d(new KComponentDataPrivate(aboutData))
{
    Q_ASSERT(!aboutData.appName().isEmpty());

    if (kdeLibraryPathsAdded == NeedLazyInit) {
        kdeLibraryPathsAdded = LazyInitDone;
        d->lazyInit();
    }

    if (registerAsMain == RegisterAsMainComponent) {
        globalStatic()->newComponentData(*this);
    }
}

KComponentData::~KComponentData()
{
    if (d) {
        d->deref();
        d = 0;
    }
}

bool KComponentData::isValid() const
{
    return (d != 0);
}

void KComponentDataPrivate::lazyInit()
{
    if (!sharedConfig) {
        configInit();
    }

#if 0 // obsolete in KF5
#ifdef Q_OS_WIN
    if (QCoreApplication::instance() && kdeLibraryPathsAdded != KdeLibraryPathsAddedDone) {
#else
    // the first KComponentData sets the KDE Qt plugin paths
    if (kdeLibraryPathsAdded != KdeLibraryPathsAddedDone) {
#endif
        kdeLibraryPathsAdded = KdeLibraryPathsAddedDone;
        const QStringList &plugins = dirs->resourceDirs("qtplugins");
        QStringList::ConstIterator it = plugins.begin();
        while (it != plugins.end()) {
            QCoreApplication::addLibraryPath(*it);
            ++it;
        }
    }
#endif
}

extern KCONFIGCORE_EXPORT bool kde_kiosk_exception;
bool kde_kiosk_admin = false;

void KComponentDataPrivate::configInit()
{
    Q_ASSERT(!sharedConfig);

    if (!configName.isEmpty()) {
        sharedConfig = KSharedConfig::openConfig(configName);

        //FIXME: this is broken and I don't know how to repair it
        // Check whether custom config files are allowed.
        KConfigGroup cg(sharedConfig, "KDE Action Restrictions");
        QString kioskException = cg.readEntry("kiosk_exception");
        if (!cg.readEntry("custom_config", true)) {
           sharedConfig = 0;
        }
    }

    if (!sharedConfig) {
        // was: KSharedConfig::openConfig(component)
        sharedConfig = KSharedConfig::openConfig(aboutData.appName() + QLatin1String("rc"));
    }

    // Check if we are excempt from kiosk restrictions
    if (kde_kiosk_admin && !kde_kiosk_exception && !qgetenv("KDE_KIOSK_NO_RESTRICTIONS").isEmpty()) {
        kde_kiosk_exception = true;
        sharedConfig = 0;
        configInit(); // Reread...
    }
}

const KSharedConfig::Ptr &KComponentData::config() const
{
    Q_ASSERT(d);
    d->lazyInit();

    return d->sharedConfig;
}

void KComponentData::setConfigName(const QString &configName)
{
    Q_ASSERT(d);
    d->configName = configName;
}

const K4AboutData *KComponentData::aboutData() const
{
    Q_ASSERT(d);
    return &d->aboutData;
}

void KComponentData::setAboutData(const K4AboutData &aboutData)
{
    d->aboutData = aboutData;
}

QString KComponentData::componentName() const
{
    Q_ASSERT(d);
    return d->aboutData.appName();
}

QString KComponentData::catalogName() const
{
    Q_ASSERT(d);
    return d->aboutData.catalogName();
}

bool KComponentData::hasMainComponent()
{
    KComponentDataStatic* s = globalStatic();
    return s && s->mainComponent.isValid();
}

const KComponentData& KComponentData::mainComponent()
{
    KComponentDataStatic* s = globalStatic();
    return s && s->mainComponent.isValid() ? s->mainComponent : *fakeComponent();
}

const KComponentData& KComponentData::activeComponent()
{
    MYASSERT(globalStatic()->activeComponent.isValid());
    return globalStatic()->activeComponent;
}

void KComponentData::setActiveComponent(const KComponentData &c)
{
    globalStatic()->activeComponent = c;
#if 0 // TEMP_KF5_REENABLE
    if (c.isValid()) {
        KLocale::global()->setActiveCatalog(c.catalogName());
    }
#endif
}

KComponentData::operator KAboutData() const
{
    return KAboutData(*aboutData());
}

void KComponentData::virtual_hook(int, void*)
{ /*BASE::virtual_hook(id, data);*/ }

