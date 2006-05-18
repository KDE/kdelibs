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

#ifndef SOLID_IFACES_ABSTRACTCAPABILITY_H
#define SOLID_IFACES_ABSTRACTCAPABILITY_H

#include <QObject>

#include <kdelibs_export.h>

#include <solid/ifaces/capability.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * Default implementation of the Solid::Ifaces::Capability interface.
     *
     * @see Solid::Ifaces::Capability
     */
    class KDE_EXPORT AbstractCapability : public QObject, virtual public Capability
    {
        Q_OBJECT
    public:
        /**
         * Creates a new AbstractCapability object.
         *
         * @param parent the parent QObject
         */
        AbstractCapability( QObject *parent = 0 );

        /**
         * Destroys an AbstractCapability object.
         */
        virtual ~AbstractCapability();

        /**
         * Override from Solid::Ifaces::Capability. It simply returns
         * 'this' since we inherit from QObject.
         *
         * @return this capability as a QObject
         * @see Solid::Ifaces::Capability::qobject()
         */
	virtual const QObject *qobject() const;

        /**
         * This method is the const version of the above method.
         *
         * @return this capability as a const QObject
         * @see Solid::Ifaces::Capability::qobject()
         */
	virtual QObject *qobject();
    };
}
}

#endif
