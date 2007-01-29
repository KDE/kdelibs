/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>

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

#include "config.h"
#include "kcomponentdata.h"

#include <stdlib.h>

#include <qcoreapplication.h>
#include <qbytearray.h>
#include <qstring.h>

#include "kaboutdata.h"
#include "kcmdlineargs.h"
#include "kconfig.h"
#include "kglobal.h"
#include "klocale.h"
#include "kstandarddirs.h"
#include <QtDebug>

class KComponentDataPrivate
{
public:
    KComponentDataPrivate ()
        : dirs(0),
        aboutData(0),
        ownAboutdata(false),
        refCount(1)
    {
    }

    ~KComponentDataPrivate ()
    {
        qDebug() << k_funcinfo;
        if (ownAboutdata) {
            delete aboutData;
            aboutData = 0;
        }
        delete dirs;
        dirs = 0;
    }

    void ref();
    void deref();
    void checkConfig();

    KStandardDirs *dirs;
    QByteArray name;
    const KAboutData *aboutData;
    QString configName;
    bool ownAboutdata;
    KSharedConfig::Ptr sharedConfig;

private:
    int refCount;
    KComponentDataPrivate(const KComponentDataPrivate&);
    KComponentDataPrivate &operator=(const KComponentDataPrivate&);
};

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

KComponentData::KComponentData(const QByteArray &name)
    : d(new KComponentDataPrivate)
{
    Q_ASSERT(!name.isEmpty());

    d->name = name;
    d->aboutData = new KAboutData(name, "", 0);
    d->ownAboutdata = true;

    if (name != "kdeinit4") {
        KGlobal::newComponentData(*this);
    }
}

KComponentData::KComponentData(const KAboutData *aboutData)
    : d(new KComponentDataPrivate)
{
    d->name = aboutData->appName();
    d->aboutData = aboutData;

    Q_ASSERT(!d->name.isEmpty());

    KGlobal::newComponentData(*this);
}

void KComponentData::_checkConfig()
{
    d->checkConfig();
}

inline void KComponentDataPrivate::checkConfig()
{
    if (sharedConfig.isUnique() && refCount == 1) { // sharedConfig is the only object holding a ref to us
        sharedConfig.clear(); // will delete sharedConfig and then deref this to 0
    }
}

inline void KComponentDataPrivate::ref()
{
    ++refCount;
}

inline void KComponentDataPrivate::deref()
{
    --refCount;
    if (refCount == 0) {
        delete this;
    } else if (sharedConfig.isUnique() && refCount == 1) {
        sharedConfig.clear(); // will delete sharedConfig and then deref this to 0
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

KStandardDirs *KComponentData::dirs() const
{
    Q_ASSERT(d);
    if (d->dirs == 0) {
        d->dirs = new KStandardDirs(*this);
        // install appdata resource type
        d->dirs->addResourceType("appdata", KStandardDirs::kde_default("data") + d->name + '/');

        if (d->sharedConfig) {
            if (d->dirs->addCustomized(d->sharedConfig.data())) {
                d->sharedConfig->reparseConfiguration();
            }
        } else {
            config(); // trigger adding of possible customized dirs
        }
    }

    return d->dirs;
}

bool kde_kiosk_exception = false; // flag to disable kiosk restrictions
bool kde_kiosk_admin = false;

KSharedConfig::Ptr &KComponentData::privateConfig() const
{
    Q_ASSERT(d);
    return d->sharedConfig;
}

const KSharedConfig::Ptr &KComponentData::config() const
{
    Q_ASSERT(d);
    if (!d->sharedConfig) {
        if (!d->configName.isEmpty()) {
            d->sharedConfig = KSharedConfig::openConfig(*this, d->configName);

            // Check whether custom config files are allowed.
            d->sharedConfig->setGroup("KDE Action Restrictions");
            QString kioskException = d->sharedConfig->readEntry("kiosk_exception");
            if (d->sharedConfig->readEntry("custom_config", true)) {
               d->sharedConfig->setGroup(QString());
            } else {
               d->sharedConfig = 0;
            }
        }

        if (!d->sharedConfig) {
            if (!d->name.isEmpty()) {
                d->sharedConfig = KSharedConfig::openConfig(*this, d->name + "rc");
            } else {
                d->sharedConfig = KSharedConfig::openConfig(*this, QString());
            }
        }

        // Check if we are excempt from kiosk restrictions
        if (kde_kiosk_admin && !kde_kiosk_exception && !QByteArray(getenv("KDE_KIOSK_NO_RESTRICTIONS")).isEmpty()) {
            kde_kiosk_exception = true;
            d->sharedConfig = 0;
            return config(); // Reread...
        }

        if (d->dirs) {
            if (d->dirs->addCustomized(d->sharedConfig.data())) {
                d->sharedConfig->reparseConfiguration();
            }
        }
    }

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
    return d->aboutData;
}

QByteArray KComponentData::componentName() const
{
    Q_ASSERT(d);
    return d->name;
}

void KComponentData::virtual_hook(int, void*)
{ /*BASE::virtual_hook(id, data);*/ }
