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

#ifndef KSETTINGS_DISPATCHER_H
#define KSETTINGS_DISPATCHER_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <kutils_export.h>
#include <kcomponentdata.h>

namespace KSettings
{

/**
 * @short Dispatch change notifications from the KCMs to the program.
 *
 * Since your program does not have direct control over the KCMs that get loaded
 * into KSettings::Dialog you need a way to get notified. This is what you do:
 * \code
 * Dispatcher::registerComponent(componentData(), this, "loadSettings");
 * \endcode
 *
 * @author Matthias Kretz <kretz@kde.org>
 */
namespace Dispatcher
{
    /**
     * Register a slot to be called when the configuration for the componentData
     * has changed. @p componentData is the KComponentData object
     * that is passed to KGenericFactory (if it is used). You can query
     * it with KGenericFactory<YourClassName>::componentData().
     * componentData.componentName() is also the same name that is put into the
     * .desktop file of the KCMs for the X-KDE-ParentComponents.
     *
     * @param componentData     The KComponentData object
     * @param recv         The object that should receive the signal
     * @param slot         The slot to be called: "slotName"
     */
    KUTILS_EXPORT void registerComponent(const KComponentData &componentData, QObject *recv, const char *slot);

    /**
     * @return the KConfig object that belongs to the componentName
     */
    KUTILS_EXPORT KSharedConfig::Ptr configForComponentName(const QString &componentName);

    /**
     * @return a list of all the componentData names that are currently
     * registered
     */
    KUTILS_EXPORT QList<QString> componentNames();

    /**
     * Call this function when the configuration belonging to the associated
     * componentData name has changed. The registered slot will be called.
     *
     * @param componentName The value of X-KDE-ParentComponents.
     */
    KUTILS_EXPORT void reparseConfiguration(const QString &componentName);

    /**
     * When this function is called the KConfig objects of all the registered
     * instances are sync()ed. This is useful when some other KConfig
     * objects will read/write from/to the same config file, so that you
     * can first write out the current state of the KConfig objects.
     */
    KUTILS_EXPORT void syncConfiguration();
} // namespace Dispatcher

}
#endif // KSETTINGS_DISPATCHER_H
