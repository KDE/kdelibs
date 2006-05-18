/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_IFACES_CAPABILITY_H
#define SOLID_IFACES_CAPABILITY_H

#include <QObject>
#include <QFlags>

#include <kdelibs_export.h>

#include <solid/ifaces/enums.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * Base interface of all the capabilities.
     *
     * A capability describes what a device can do. A device generally has
     * a set of capabilities.
     *
     * @see Solid::Ifaces::AbstractCapability
     */
    class KDE_EXPORT Capability : public Enums::Capability
    {
    public:
        /**
         * Destroys a Capability object.
         */
        virtual ~Capability();

        /**
         * Retrieves a QObject representing this capability.
         *
         * Capabilities have properties and signals, but QObject can't
         * be inherited multiple times. Then we need to be able to get a
         * QObject instance representing a capability and holding its
         * properties and signals.
         *
         * In most implementation this method will simply return 'this'.
         * In this case the capabilities provided by the backend can subclass
         * Solid::Ifaces::AbstractCapability which provide the necessary
         * implementation.
         *
         * @return this capability as a QObject
         * @see Solid::Ifaces::AbstractCapability::qobject()
         */
	virtual QObject *qobject() = 0;

        /**
         * This method is the const version of the above method.
         *
         * @return this capability as a const QObject
         * @see Solid::Ifaces::AbstractCapability::qobject()
         */
        virtual const QObject *qobject() const = 0;
    };
}
}

#endif
