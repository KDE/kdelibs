/*  This file is part of the KDE project
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

#ifndef KERNEL_KCOMPONENTDATA_P_H
#define KERNEL_KCOMPONENTDATA_P_H

#include "kcomponentdata.h"
#include <QAtomicInt>
#include <QDebug>
#include <QCoreApplication>
#include <QString>

#include <kconfig.h>
#include <kaboutdata.h>
#if 0 // TEMP_KF5_REENABLE
#include <klocale.h>
#endif

class KComponentDataPrivate
{
public:
    KComponentDataPrivate(const KAboutData &aboutData_)
        : aboutData(aboutData_),
        shouldRemoveCatalog(false),
        refCount(1)
    {
#if 0 // TEMP_KF5_REENABLE
        if (QCoreApplication::instance()) {
            // KLocal::global() needs an app name
            KLocale::global()->insertCatalog(aboutData.catalogName());
            shouldRemoveCatalog = true;
        }
#endif
    }

    ~KComponentDataPrivate()
    {
        refCount.fetchAndStoreOrdered(-0x00FFFFFF); //prevent a reentering of the dtor
#if 0 // TEMP_KF5_REENABLE
        if (shouldRemoveCatalog && KLocale::global())
            KLocale::global()->removeCatalog(aboutData.catalogName());
#endif

        sharedConfig = 0;   //delete the config object first, because it could access the standard dirs while syncing
    }

    inline void ref()
    {
        refCount.ref();
        //qDebug() << refCount - 1 << "->" << refCount << kBacktrace() << endl;
    }

    inline void deref()
    {
        const int refc = refCount.fetchAndAddOrdered(-1) - 1;
        //qDebug() << refCount + 1 << "->" << refCount << kBacktrace() << endl;
        if (refc == 0) {
            delete this;
        }
    }

    void lazyInit();
    void configInit();  //call this only from lazyInit()!

    KAboutData aboutData;
    QString configName;
    KSharedConfig::Ptr sharedConfig;

private:
    bool shouldRemoveCatalog;
    QAtomicInt refCount;
    KComponentDataPrivate(const KComponentDataPrivate&);
    KComponentDataPrivate &operator=(const KComponentDataPrivate&);
};

#endif // KERNEL_KCOMPONENTDATA_P_H
// vim: sw=4 sts=4 et tw=100
