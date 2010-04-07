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
#include "kcmdlineargs.h"
#include "kconfig.h"
#include "kglobal.h"
#include "kglobal_p.h"
#include "klocale.h"
#include "kconfiggroup.h"
#include "kstandarddirs.h"
#include <QtDebug>

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

enum KdeLibraryPathsAdded {
    NeedLazyInit,
    LazyInitDone,
    KdeLibraryPathsAddedDone
};
static KdeLibraryPathsAdded kdeLibraryPathsAdded = NeedLazyInit;

KComponentData::KComponentData(const QByteArray &name, const QByteArray &catalog, MainComponentRegistration registerAsMain)
    : d(new KComponentDataPrivate(KAboutData(name, catalog, KLocalizedString(), "", KLocalizedString())))
{
    Q_ASSERT(!name.isEmpty());

    if (kdeLibraryPathsAdded == NeedLazyInit) {
        kdeLibraryPathsAdded = LazyInitDone;
        d->lazyInit(*this);
    }

    if (registerAsMain == RegisterAsMainComponent) {
        KGlobal::newComponentData(*this);
    }
}

KComponentData::KComponentData(const KAboutData *aboutData, MainComponentRegistration registerAsMain)
    : d(new KComponentDataPrivate(*aboutData))
{
    Q_ASSERT(!aboutData->appName().isEmpty());

    if (kdeLibraryPathsAdded == NeedLazyInit) {
        kdeLibraryPathsAdded = LazyInitDone;
        d->lazyInit(*this);
    }

    if (registerAsMain == RegisterAsMainComponent) {
        KGlobal::newComponentData(*this);
    }
}

KComponentData::KComponentData(const KAboutData &aboutData, MainComponentRegistration registerAsMain)
    : d(new KComponentDataPrivate(aboutData))
{
    Q_ASSERT(!aboutData.appName().isEmpty());

    if (kdeLibraryPathsAdded == NeedLazyInit) {
        kdeLibraryPathsAdded = LazyInitDone;
        d->lazyInit(*this);
    }

    if (registerAsMain == RegisterAsMainComponent) {
        KGlobal::newComponentData(*this);
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

void KComponentDataPrivate::lazyInit(const KComponentData &component)
{
    if (dirs == 0) {
        dirs = new KStandardDirs();
        // install appdata resource type
        dirs->addResourceType("appdata", "data", aboutData.appName() + QLatin1Char('/'), true);

        configInit(component);

        if (dirs->addCustomized(sharedConfig.data()))
            sharedConfig->reparseConfiguration();
    }
    
    // the first KComponentData sets the KDE Qt plugin paths
    if (QCoreApplication::instance() && dirs && kdeLibraryPathsAdded != KdeLibraryPathsAddedDone) {
        kdeLibraryPathsAdded = KdeLibraryPathsAddedDone;
        const QStringList &plugins = dirs->resourceDirs("qtplugins");
        QStringList::ConstIterator it = plugins.begin();
        while (it != plugins.end()) {
            QCoreApplication::addLibraryPath(*it);
            ++it;
        }
    }
}

bool kde_kiosk_exception = false; // flag to disable kiosk restrictions
bool kde_kiosk_admin = false;

void KComponentDataPrivate::configInit(const KComponentData &component)
{
    Q_ASSERT(!sharedConfig);

    if (!configName.isEmpty()) {
        sharedConfig = KSharedConfig::openConfig(component, configName);

        //FIXME: this is broken and I don't know how to repair it
        // Check whether custom config files are allowed.
        KConfigGroup cg(sharedConfig, "KDE Action Restrictions");
        QString kioskException = cg.readEntry("kiosk_exception");
        if (!cg.readEntry("custom_config", true)) {
           sharedConfig = 0;
        }
    }
    
    if (!sharedConfig) {
        sharedConfig = KSharedConfig::openConfig(component);
    }

    // Check if we are excempt from kiosk restrictions
    if (kde_kiosk_admin && !kde_kiosk_exception && !qgetenv("KDE_KIOSK_NO_RESTRICTIONS").isEmpty()) {
        kde_kiosk_exception = true;
        sharedConfig = 0;
        configInit(component); // Reread...
    }
}

KStandardDirs *KComponentData::dirs() const
{
    Q_ASSERT(d);
    d->lazyInit(*this);

    return d->dirs;
}

const KSharedConfig::Ptr &KComponentData::config() const
{
    Q_ASSERT(d);
    d->lazyInit(*this);

    return d->sharedConfig;
}

void KComponentData::setConfigName(const QString &configName)
{
    Q_ASSERT(d);
    d->configName = configName;
}

const KAboutData *KComponentData::aboutData() const
{
    Q_ASSERT(d);
    return &d->aboutData;
}

void KComponentData::setAboutData(const KAboutData &aboutData)
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

void KComponentData::virtual_hook(int, void*)
{ /*BASE::virtual_hook(id, data);*/ }
