/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

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

#include "dispatcher.h"
#include "dispatcher_p.h"

#include <kdebug.h>
#include <kconfig.h>
#include <kcomponentdata.h>
#include <assert.h>
#include <kglobal.h>

namespace KSettings
{

namespace Dispatcher
{

K_GLOBAL_STATIC(DispatcherPrivate, d)

void registerComponent(const KComponentData &componentData, QObject *recv, const char *slot)
{
    Q_ASSERT(componentData.isValid());
    // keep the KComponentData around and call
    // componentData.config()->reparseConfiguration when the app should reparse
    QString componentName = componentData.componentName();
    kDebug(701) << componentName;
    d->m_componentName[recv] = componentName;
    if (!d->m_componentInfo.contains(componentName)) {
        d->m_componentInfo[componentName].componentData = componentData;
    }
    d->m_componentInfo[componentName].slotList.append(ComponentInfo::Slot(recv, slot));

    ++(d->m_componentInfo[componentName].count);
    QObject::connect(recv, SIGNAL(destroyed(QObject *)), d, SLOT(unregisterComponent(QObject *)));
}

KSharedConfig::Ptr configForComponentName(const QString &componentName)
{
    kDebug(701) ;
    if (d->m_componentInfo.contains(componentName)) {
        KComponentData componentData = d->m_componentInfo[componentName].componentData;
        if (componentData.isValid()) {
            return componentData.config();
        }
    }
    kError(701) << "configForComponentName('" << componentName.constData()
        << "') could not find the KComponentData object" << endl;
    Q_ASSERT(!d->m_componentInfo.isEmpty());
    return d->m_componentInfo.constBegin()->componentData.config();
}

QList<QString> componentNames()
{
    kDebug(701) ;
    QList<QString> names;
    for (QMap<QString, ComponentInfo>::ConstIterator it = d->m_componentInfo.constBegin(); it != d->m_componentInfo.constEnd(); ++it) {
        if ((*it).count > 0) {
            names.append(it.key());
        }
    }
    return names;
}

void reparseConfiguration(const QString & componentName)
{
    kDebug(701) << componentName;
    // check if the componentName is valid:
    if (! d->m_componentInfo.contains(componentName)) {
        return;
    }
    // first we reparse the config of the componentData so that the KConfig object
    // will be up to date
    KSharedConfig::Ptr config = d->m_componentInfo[componentName].componentData.config();
    config->reparseConfiguration();
    foreach(const ComponentInfo::Slot& slot, d->m_componentInfo[componentName].slotList ) {
        QMetaObject::invokeMethod(slot.first, slot.second);
    }
}

void syncConfiguration()
{
    for (QMap<QString, ComponentInfo>::ConstIterator it = d->m_componentInfo.constBegin(); it != d->m_componentInfo.constEnd(); ++it) {
        KSharedConfig::Ptr config = (*it).componentData.config();
        config->sync();
    }
}

void DispatcherPrivate::unregisterComponent(QObject *obj)
{
    if (!m_componentName.contains(obj)) {
      kWarning(701) << k_funcinfo << "Tried to unregister an object which is not already registered.";
      return;
    }

    QString name = m_componentName[obj];
    m_componentName.remove(obj); //obj will be destroyed when we return, so we better remove this entry
    --(m_componentInfo[name].count);
    kDebug(701) << "componentName=" << name << "refcount=" << m_componentInfo[name].count;
    Q_ASSERT(m_componentInfo[name].count >= 0);
    if (m_componentInfo[name].count == 0) {
        m_componentInfo.remove(name);
    }
}

} // namespace Dispatcher
} // namespace KSettings

#include "dispatcher_p.moc"
