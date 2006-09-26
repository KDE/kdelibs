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

#ifndef SOLID_CAPABILITY_H
#define SOLID_CAPABILITY_H

#include <QObject>
#include <QFlags>

#include <kdelibs_export.h>

#include <solid/ifaces/enums.h>

namespace Solid
{
    namespace Ifaces
    {
        class Capability;
    }

    /**
     * Base class of all the capabilities.
     *
     * A capability describes what a device can do. A device generally has
     * a set of capabilities.
     */
    class KDE_EXPORT Capability : public QObject, public Ifaces::Enums::Capability
    {
        Q_OBJECT
    public:
        /**
         * Creates a new Capability object.
         *
         * @param backendObject the capability object provided by the backend
         */
        Capability( QObject *backendObject );

        /**
         * Destroys a Capability object.
         */
        virtual ~Capability();

    protected:
        QObject *backendObject();
        const QObject *backendObject() const;

    private:
        class Private;
        Private *d;
    };
}

#endif
