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

#include "ksettings/dispatcher.h"

#include <q3signal.h>

#include <kstaticdeleter.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kcomponentdata.h>
#include <assert.h>
#include <kglobal.h>

namespace KSettings
{

//class Dispatcher::DispatcherPrivate
//{
//};

static KStaticDeleter<Dispatcher> ksd_kpd;

Dispatcher *Dispatcher::m_self = 0;

Dispatcher *Dispatcher::self()
{
    kDebug(701) << k_funcinfo << endl;
    if (m_self == 0)
        ksd_kpd.setObject(m_self, new Dispatcher());
    return m_self;
}

Dispatcher::Dispatcher(QObject *parent)
    : QObject(parent)
    //, d(0)
{
    kDebug(701) << k_funcinfo << endl;
}

Dispatcher::~Dispatcher()
{
    kDebug(701) << k_funcinfo << endl;
    //delete d;
}

void Dispatcher::registerComponent(const KComponentData &componentData, QObject *recv, const char *slot)
{
    Q_ASSERT(componentData.isValid());
    // keep the KComponentData around and call
    // componentData.config()->reparseConfiguration when the app should reparse
    QByteArray componentName = componentData.componentName();
    kDebug(701) << k_funcinfo << componentName << endl;
    m_componentName[recv] = componentName;
    Q3Signal *sig;
    if (m_componentInfo.contains(componentName)) {
        sig = m_componentInfo[componentName].signal;
    } else {
        sig = new Q3Signal(this, "signal dispatcher");
        m_componentInfo[componentName].signal = sig;
        m_componentInfo[componentName].componentData = componentData;
    }
    sig->connect(recv, slot);

    ++m_componentInfo[componentName].count;
    connect(recv, SIGNAL(destroyed(QObject *)), this, SLOT(unregisterComponent(QObject *)));
}

const KSharedConfig::Ptr &Dispatcher::configForComponentName(const QByteArray &componentName)
{
    kDebug(701) << k_funcinfo << endl;
    if (m_componentInfo.contains(componentName)) {
        KComponentData componentData = m_componentInfo[componentName].componentData;
        if (componentData.isValid()) {
            return componentData.config();
        }
    }
    kError(701) << "configForComponentName('" << componentName.constData()
        << "') could not find the KComponentData object" << endl;
    Q_ASSERT(!m_componentInfo.isEmpty());
    return m_componentInfo.constBegin()->componentData.config();
}

QList<QByteArray> Dispatcher::componentNames() const
{
    kDebug(701) << k_funcinfo << endl;
    QList<QByteArray> names;
    for (QMap<QByteArray, ComponentInfo>::ConstIterator it = m_componentInfo.begin(); it != m_componentInfo.end(); ++it) {
        if ((*it).count > 0) {
            names.append(it.key());
        }
    }
    return names;
}

void Dispatcher::reparseConfiguration(const QByteArray & componentName)
{
    kDebug(701) << k_funcinfo << componentName << endl;
    // check if the componentName is valid:
    if (! m_componentInfo.contains(componentName)) {
        return;
    }
    // first we reparse the config of the componentData so that the KConfig object
    // will be up to date
    KSharedConfig::Ptr config = m_componentInfo[componentName].componentData.config();
    config->reparseConfiguration();
    Q3Signal *sig = m_componentInfo[componentName].signal;
    if (sig) {
        kDebug(701) << "emit signal to componentData" << endl;
        sig->activate();
    }
}

void Dispatcher::syncConfiguration()
{
    for (QMap<QByteArray, ComponentInfo>::ConstIterator it = m_componentInfo.begin(); it != m_componentInfo.end(); ++it) {
        KSharedConfig::Ptr config = (*it).componentData.config();
        config->sync();
    }
}

void Dispatcher::unregisterComponent(QObject *obj)
{
    kDebug(701) << k_funcinfo << endl;
    QByteArray name = m_componentName[obj];
    m_componentName.remove(obj); //obj will be destroyed when we return, so we better remove this entry
    --m_componentInfo[name].count;
    if (m_componentInfo[name].count == 0) {
        delete m_componentInfo[name].signal;
        m_componentInfo.remove(name);
    }
}

} //namespace

#include "dispatcher.moc"

// vim: sw=4 sts=4 et
