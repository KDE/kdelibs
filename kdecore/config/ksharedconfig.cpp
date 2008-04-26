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

#include "ksharedconfig.h"
#include "kconfigbackend.h"
#include "kconfiggroup.h"
#include "kcomponentdata.h"
#include "kglobal.h"
#include "kconfig_p.h"

K_GLOBAL_STATIC(QList<KSharedConfig*>, globalSharedConfigList)

KSharedConfigPtr KSharedConfig::openConfig( const QString& fileName,
                                            OpenFlags flags ,
                                            const char *resType)
{
    return openConfig(KGlobal::mainComponent(), fileName, flags, resType);
}

KSharedConfigPtr KSharedConfig::openConfig( const KComponentData &componentData,
                                            const QString& fileName,
                                            OpenFlags flags,
                                            const char *resType)
{
    const QList<KSharedConfig*> *list = globalSharedConfigList;
    if (list) {
        for(QList<KSharedConfig*>::ConstIterator it = list->begin(); it != list->end(); ++it) {
            if ( (*it)->name() == fileName &&
                 (*it)->d_ptr->openFlags == flags &&
//                 qstrcmp((*it)->resource(), resType) == 0 &&
//                 (*it)->backEnd()->type() == backEnd &&
                 (*it)->componentData() == componentData
               ) {
                return KSharedConfigPtr(*it);
            }
        }
    }
    return KSharedConfigPtr(new KSharedConfig(componentData, fileName, flags, resType));
}


KSharedConfig::KSharedConfig( const KComponentData &componentData,
                              const QString &fileName,
                              OpenFlags flags,
                              const char *resType)
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

KConfigGroup KSharedConfig::groupImpl(const QByteArray &groupName)
{
    KSharedConfigPtr ptr(this);
    return KConfigGroup( ptr, groupName.constData());
}

const KConfigGroup KSharedConfig::groupImpl(const QByteArray &groupName) const
{
    const KSharedConfigPtr ptr(const_cast<KSharedConfig*>(this));
    return KConfigGroup( ptr, groupName.constData());
}
