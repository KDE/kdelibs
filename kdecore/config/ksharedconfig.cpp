/*
  This file is part of the KDE libraries
  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (C) 1997-1999 Matthias Kalle Dalheimer (kalle@kde.org)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "kconfigbackend.h"
#include "kcomponentdata.h"
#include "ksharedconfig.h"

K_GLOBAL_STATIC(QList<KSharedConfig*>, globalSharedConfigList)

KSharedConfigPtr KSharedConfig::openConfig( const QString& fileName,
                                            OpenFlags flags ,
                                            const char *resType )
{
    return openConfig(KGlobal::mainComponent(), fileName, flags, resType);
}

KSharedConfigPtr KSharedConfig::openConfig( const KComponentData &componentData,
                                            const QString& fileName,
                                            OpenFlags flags,
                                            const char *resType )
{
    bool useKDEGlobals = flags & IncludeGlobals;
    QList<KSharedConfig*> *list = globalSharedConfigList;
    if (list) {
        for(QList<KSharedConfig*>::ConstIterator it = list->begin(); it != list->end(); ++it) {
            if (
                    (*it)->backEnd->fileName() == fileName &&
                    (*it)->backEnd->useKDEGlobals == useKDEGlobals &&
                    (*it)->backEnd->resType == resType &&
                    (*it)->componentData() == componentData
               ) {
                return KSharedConfigPtr(*it);
            }
        }
    }
    return KSharedConfigPtr(new KSharedConfig(fileName, flags, resType, componentData));
}


KSharedConfig::KSharedConfig( const QString &fileName,
                              OpenFlags flags,
                              const char *resType,
                              const KComponentData &componentData)
    : KConfig(componentData, fileName, flags, resType)
{
    globalSharedConfigList->append(this);
}

KSharedConfig::~KSharedConfig()
{
    if (!globalSharedConfigList.isDestroyed()) {
        globalSharedConfigList->removeAll(this);
    }
}

KSharedConfigPtr::~KSharedConfigPtr()
{
    if (d) {
        if (!d->ref.deref()) {
            delete d;
        } else if (d->ref == 1 && d->componentData().isValid()) {
            // it might be KComponentData holding the last ref
            const_cast<KComponentData&>(d->componentData())._checkConfig();
        }
        d = 0;
    }
}

void KSharedConfigPtr::attach(KSharedConfig *p)
{
    if (d != p) {
        KSharedConfig *x = p;
        if (x) {
            x->ref.ref();
        }
        x = qAtomicSetPtr(&d, x);
        if (x) {
            if (!x->ref.deref()) {
                delete x;
            } else if (x->ref == 1 && d->componentData().isValid()) {
                // it might be KComponentData holding the last ref
                const_cast<KComponentData&>(x->componentData())._checkConfig();
            }
        }
    }
}

KConfigGroup KSharedConfig::group(const QByteArray &groupName)
{
    return KConfigGroup( KSharedConfigPtr(this), groupName);
}

const KConfigGroup KSharedConfig::group(const QByteArray &groupName) const
{
    return KConfigGroup( KSharedConfigPtr(const_cast<KSharedConfig*>(this)),
                         groupName);
}

KConfigGroup KSharedConfig::group(const char* groupName)
{
    return KConfigGroup( KSharedConfigPtr(this), groupName);
}

const KConfigGroup KSharedConfig::group(const char *groupName) const
{
    return KConfigGroup( KSharedConfigPtr(const_cast<KSharedConfig*>(this)), groupName);
}

KConfigGroup KSharedConfig::group(const QString& groupName)
{
    return KConfigGroup( KSharedConfigPtr(this), groupName);
}

const KConfigGroup KSharedConfig::group(const QString& groupName) const
{
    return KConfigGroup( KSharedConfigPtr(const_cast<KSharedConfig*>(this)), groupName);
}

QString KConfig::group() const
{
    return KConfigBase::mGroup.group();
}

QString KSharedConfig::group() const
{
    return KConfigBase::mGroup.group();
}
