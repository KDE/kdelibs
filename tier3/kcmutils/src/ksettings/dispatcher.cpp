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

#include <QDebug>

namespace KSettings
{

namespace Dispatcher
{

Q_GLOBAL_STATIC(DispatcherPrivate, d)

void registerComponent(const QString &componentName, QObject *recv, const char *slot)
{
    Q_ASSERT(!componentName.isEmpty());
    // qDebug() << componentName;
    d()->m_componentName[recv] = componentName;
    d()->m_componentInfo[componentName].slotList.append(ComponentInfo::Slot(recv, slot));

    ++(d()->m_componentInfo[componentName].count);
    QObject::connect(recv, SIGNAL(destroyed(QObject*)), d(), SLOT(unregisterComponent(QObject*)));
}

KSharedConfig::Ptr configForComponentName(const QString &componentName)
{
    // qDebug() ;
    return KSharedConfig::openConfig(componentName + QStringLiteral("rc"));
}

QList<QString> componentNames()
{
    // qDebug() ;
    QList<QString> names;
    for (QMap<QString, ComponentInfo>::ConstIterator it = d()->m_componentInfo.constBegin(); it != d()->m_componentInfo.constEnd(); ++it) {
        if ((*it).count > 0) {
            names.append(it.key());
        }
    }
    return names;
}

void reparseConfiguration(const QString & componentName)
{
    // qDebug() << componentName;
    // check if the componentName is valid:
    if (! d()->m_componentInfo.contains(componentName)) {
        return;
    }
    // first we reparse the config so that the KConfig object will be up to date
    KSharedConfig::Ptr config = configForComponentName(componentName);
    config->reparseConfiguration();

    foreach(const ComponentInfo::Slot& slot, d()->m_componentInfo[componentName].slotList ) {
        QMetaObject::invokeMethod(slot.first, slot.second);
    }
}

void syncConfiguration()
{
    for (QMap<QString, ComponentInfo>::ConstIterator it = d()->m_componentInfo.constBegin(); it != d()->m_componentInfo.constEnd(); ++it) {
        KSharedConfig::Ptr config = configForComponentName(it.key());
        config->sync();
    }
}

void DispatcherPrivate::unregisterComponent(QObject *obj)
{
    if (!m_componentName.contains(obj)) {
      qWarning() << Q_FUNC_INFO << "Tried to unregister an object which is not already registered.";
      return;
    }

    QString name = m_componentName[obj];
    m_componentName.remove(obj); //obj will be destroyed when we return, so we better remove this entry
    --(m_componentInfo[name].count);
    // qDebug() << "componentName=" << name << "refcount=" << m_componentInfo[name].count;
    Q_ASSERT(m_componentInfo[name].count >= 0);
    if (m_componentInfo[name].count == 0) {
        m_componentInfo.remove(name);
    }
}

} // namespace Dispatcher
} // namespace KSettings

#include "moc_dispatcher_p.cpp"
