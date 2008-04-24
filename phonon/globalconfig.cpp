/*  This file is part of the KDE project
    Copyright (C) 2006-2008 Matthias Kretz <kretz@kde.org>

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

#include "globalconfig_p.h"

#include "factory_p.h"
#include "objectdescription.h"
#include "phonondefs_p.h"
#include "platformplugin.h"
#include "backendinterface.h"
#include "qsettingsgroup_p.h"
#include "phononnamespace_p.h"

#include <QtCore/QList>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

namespace Phonon
{

GlobalConfig::GlobalConfig(QObject *parent)
    : QObject(parent)
    , m_config(QLatin1String("kde.org"), QLatin1String("libphonon"))
{
}

GlobalConfig::~GlobalConfig()
{
}

enum WhatToFilter {
    FilterAdvancedDevices = 1,
    FilterHardwareDevices = 2
};

static void filter(ObjectDescriptionType type, BackendInterface *backendIface, QList<int> *list, int whatToFilter)
{
    QMutableListIterator<int> it(*list);
    while (it.hasNext()) {
        const QHash<QByteArray, QVariant> properties = backendIface->objectDescriptionProperties(type, it.next());
        QVariant var;
        if (whatToFilter & FilterAdvancedDevices) {
            var = properties.value("isAdvanced");
            if (var.isValid() && var.toBool()) {
                it.remove();
                continue;
            }
        }
        if (whatToFilter & FilterHardwareDevices) {
            var = properties.value("isHardwareDevice");
            if (var.isValid() && var.toBool()) {
                it.remove();
                continue;
            }
        }
    }
}

static QList<int> listSortedByConfig(const QSettingsGroup &backendConfig, Phonon::Category category, QList<int> &defaultList)
{
    if (defaultList.size() <= 1) {
        // nothing to sort
        return defaultList;
    } else {
        // make entries unique
        QSet<int> seen;
        QMutableListIterator<int> it(defaultList);
        while (it.hasNext()) {
            if (seen.contains(it.next())) {
                it.remove();
            } else {
                seen.insert(it.value());
            }
        }
    }

    QString categoryKey = QLatin1String("Category_") + QString::number(static_cast<int>(category));
    if (!backendConfig.hasKey(categoryKey)) {
        // no list in config for the given category
        categoryKey = QLatin1String("Category_") + QString::number(static_cast<int>(Phonon::NoCategory));
        if (!backendConfig.hasKey(categoryKey)) {
            // no list in config for NoCategory
            return defaultList;
        }
    }

    //Now the list from m_config
    QList<int> deviceList = backendConfig.value(categoryKey, QList<int>());

    //if there are devices in m_config that the backend doesn't report, remove them from the list
    QMutableListIterator<int> i(deviceList);
    while (i.hasNext()) {
        if (0 == defaultList.removeAll(i.next())) {
            i.remove();
        }
    }

    //if the backend reports more devices that are not in m_config append them to the list
    deviceList += defaultList;

    return deviceList;
}

QList<int> GlobalConfig::audioOutputDeviceListFor(Phonon::Category category, HideAdvancedDevicesOverride override) const
{
    //The devices need to be stored independently for every backend
    const QSettingsGroup backendConfig(&m_config, QLatin1String("AudioOutputDevice")); // + Factory::identifier());
    const QSettingsGroup generalGroup(&m_config, QLatin1String("General"));
    const bool hideAdvancedDevices = (override == FromSettings
            ? generalGroup.value(QLatin1String("HideAdvancedDevices"), true)
            : static_cast<bool>(override));

    PlatformPlugin *platformPlugin = Factory::platformPlugin();
    BackendInterface *backendIface = qobject_cast<BackendInterface *>(Factory::backend());

    QList<int> defaultList;
    if (platformPlugin) {
        // the platform plugin lists the audio devices for the platform
        // this list already is in default order (as defined by the platform plugin)
        defaultList = platformPlugin->objectDescriptionIndexes(Phonon::AudioOutputDeviceType);
        QMutableListIterator<int> it(defaultList);
        while (it.hasNext()) {
            AudioOutputDevice objDesc = AudioOutputDevice::fromIndex(it.next());
            const QVariant var = objDesc.property("isAdvanced");
            if (var.isValid() && var.toBool()) {
                it.remove();
            }
        }
    }

    // lookup the available devices directly from the backend (mostly for virtual devices)
    if (backendIface) {
        // this list already is in default order (as defined by the backend)
        QList<int> list = backendIface->objectDescriptionIndexes(Phonon::AudioOutputDeviceType);
        if (hideAdvancedDevices || !defaultList.isEmpty()) {
            filter(AudioOutputDeviceType, backendIface, &list,
                    (hideAdvancedDevices ? FilterAdvancedDevices : 0)
                    // the platform plugin already provided the hardware devices
                    | (defaultList.isEmpty() ? 0 : FilterHardwareDevices)
                    );
        }
        defaultList += list;
    }

    return listSortedByConfig(backendConfig, category, defaultList);
}

int GlobalConfig::audioOutputDeviceFor(Phonon::Category category) const
{
    QList<int> ret = audioOutputDeviceListFor(category);
    if (ret.isEmpty())
        return -1;
    return ret.first();
}

QList<int> GlobalConfig::audioCaptureDeviceListFor(Phonon::Category category, HideAdvancedDevicesOverride override) const
{
    //The devices need to be stored independently for every backend
    const QSettingsGroup backendConfig(&m_config, QLatin1String("AudioCaptureDevice")); // + Factory::identifier());
    const QSettingsGroup generalGroup(&m_config, QLatin1String("General"));
    const bool hideAdvancedDevices = (override == FromSettings
            ? generalGroup.value(QLatin1String("HideAdvancedDevices"), true)
            : static_cast<bool>(override));

    PlatformPlugin *platformPlugin = Factory::platformPlugin();
    BackendInterface *backendIface = qobject_cast<BackendInterface *>(Factory::backend());

    QList<int> defaultList;
    if (platformPlugin) {
        // the platform plugin lists the audio devices for the platform
        // this list already is in default order (as defined by the platform plugin)
        defaultList = platformPlugin->objectDescriptionIndexes(Phonon::AudioCaptureDeviceType);
        QMutableListIterator<int> it(defaultList);
        while (it.hasNext()) {
            AudioCaptureDevice objDesc = AudioCaptureDevice::fromIndex(it.next());
            const QVariant var = objDesc.property("isAdvanced");
            if (var.isValid() && var.toBool()) {
                it.remove();
            }
        }
    }

    // lookup the available devices directly from the backend (mostly for virtual devices)
    if (backendIface) {
        // this list already is in default order (as defined by the backend)
        QList<int> list = backendIface->objectDescriptionIndexes(Phonon::AudioCaptureDeviceType);
        if (hideAdvancedDevices || !defaultList.isEmpty()) {
            filter(AudioCaptureDeviceType, backendIface, &list,
                    (hideAdvancedDevices ? FilterAdvancedDevices : 0)
                    // the platform plugin already provided the hardware devices
                    | (defaultList.isEmpty() ? 0 : FilterHardwareDevices)
                  );
        }
        defaultList += list;
    }

    return listSortedByConfig(backendConfig, category, defaultList);
}

int GlobalConfig::audioCaptureDeviceFor(Phonon::Category category) const
{
    QList<int> ret = audioCaptureDeviceListFor(category);
    if (ret.isEmpty())
        return -1;
    return ret.first();
}

} // namespace Phonon

QT_END_NAMESPACE

#include "moc_globalconfig_p.cpp"

// vim: sw=4 ts=4
