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
#include "kconfig_p.h"
#include <QCoreApplication>

class GlobalSharedConfigList : public QList<KSharedConfig*>
{
public:
    // in addition to the list, we need to hold the main config,
    // so that it's not created and destroyed all the time.
    KSharedConfigPtr mainConfig;
};


Q_GLOBAL_STATIC(GlobalSharedConfigList, globalSharedConfigList)

KSharedConfigPtr KSharedConfig::openConfig(const QString& _fileName,
                                           OpenFlags flags,
                                           QStandardPaths::StandardLocation resType)
{
    QString fileName(_fileName);
    GlobalSharedConfigList *list = globalSharedConfigList();
    if (fileName.isEmpty()) {
        // Determine the config file name that KConfig will make up (see KConfigPrivate::changeFileName)
        fileName = KConfig::mainConfigName();
    }

    if (list) {
        for(QList<KSharedConfig*>::ConstIterator it = list->constBegin(); it != list->constEnd(); ++it) {
            if ( (*it)->name() == fileName &&
                 (*it)->d_ptr->openFlags == flags &&
                 (*it)->locationType() == resType
//                 (*it)->backEnd()->type() == backEnd
               ) {
                return KSharedConfigPtr(*it);
            }
        }
    }
    KSharedConfigPtr ptr(new KSharedConfig(fileName, flags, resType));
    if (_fileName.isEmpty() && flags == FullConfig && resType == QStandardPaths::ConfigLocation)
        list->mainConfig = ptr;
    return ptr;
}


KSharedConfig::KSharedConfig(const QString &fileName,
                             OpenFlags flags,
                             QStandardPaths::StandardLocation resType)
    : KConfig(fileName, flags, resType)
{
    globalSharedConfigList()->append(this);
}

KSharedConfig::~KSharedConfig()
{
#ifdef __GNUC__
#warning Qt5: re-enable once the new Q_GLOBAL_STATIC is in
#endif
#if 0
    if (!globalSharedConfigList.isDestroyed())
#else
    if (globalSharedConfigList())
#endif
        globalSharedConfigList()->removeAll(this);
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
