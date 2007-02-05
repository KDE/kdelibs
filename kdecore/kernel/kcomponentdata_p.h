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
#include <QtDebug>
#include <QByteArray>
#include <QString>
#include <kconfig.h>
#include <kaboutdata.h>
#include <kstandarddirs.h>

class KComponentDataPrivate
{
public:
    KComponentDataPrivate()
        : dirs(0),
        aboutData(0),
        ownAboutdata(true),
        syncing(false),
        refCount(1)
    {
    }

    ~KComponentDataPrivate()
    {
        kDebug(180) << k_funcinfo << endl;
        if (ownAboutdata) {
            delete aboutData;
            aboutData = 0;
        }
        delete dirs;
        dirs = 0;
    }

    inline void ref()
    {
        ++refCount;
        //qDebug() << k_funcinfo << refCount - 1 << "->" << refCount << kBacktrace() << endl;
    }

    inline void deref()
    {
        --refCount;
        //qDebug() << k_funcinfo << refCount + 1 << "->" << refCount << kBacktrace() << endl;
        if (refCount == 0) {
            delete this;
        } else {
            checkConfig();
        }
    }

    void checkConfig();

    KStandardDirs *dirs;
    QByteArray name;
    const KAboutData *aboutData;
    QString configName;
    KSharedConfig::Ptr sharedConfig;
    bool ownAboutdata;
    bool syncing;

private:
    int refCount;
    KComponentDataPrivate(const KComponentDataPrivate&);
    KComponentDataPrivate &operator=(const KComponentDataPrivate&);
};

#endif // KERNEL_KCOMPONENTDATA_P_H
// vim: sw=4 sts=4 et tw=100
