/*  This file is part of the KDE project
    Copyright (C) 2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_DVBHW_H
#define SOLID_DVBHW_H

#include <kdelibs_export.h>

#include <solid/capability.h>

namespace Solid
{
    class DvbHwPrivate;

    /**
     * This capability is available on Digital Video Broadcast (DVB) devices.
     *
     * A DVB device is a device implementing the open standards for digital
     * television maintained by the DVB Project
     * It is possible to interact with such a device using a special device
     * file in the system.
     */
    class SOLID_EXPORT DvbHw : public Capability
    {
        Q_OBJECT
        Q_ENUMS( DeviceType )
        Q_PROPERTY( QString device READ device )
        Q_PROPERTY( int deviceAdapter READ deviceAdapter )
        Q_PROPERTY( DeviceType deviceType READ deviceType )
        Q_PROPERTY( int deviceIndex READ deviceIndex )
        Q_DECLARE_PRIVATE(DvbHw)

    public:
        /**
         * This enum type defines the type of a dvb device.
         *
         * - DvbAudio : An audio device.
         * - DvbCa : A common access device.
         * - DvbDemux : A demultiplexer device.
         * - DvbDvr : A dvr device.
         * - DvbFrontend : A frontend device.
         * - DvbNet : A network device.
         * - DvbOsd : An osd device.
         * - DvbSec : A sec device.
         * - DvbVideo : A video device.
         * - DvbUnknown : An unidentified device.
         */
        enum DeviceType { DvbUnknown, DvbAudio, DvbCa, DvbDemux, DvbDvr,
                          DvbFrontend, DvbNet, DvbOsd, DvbSec, DvbVideo };


        /**
         * Creates a new DvbHw object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the capability object provided by the backend
         * @see Solid::Device::as()
         */
        explicit DvbHw( QObject *backendObject );

        /**
         * Destroys a DvbHw object.
         */
        virtual ~DvbHw();


        /**
         * Get the Solid::Capability::Type of the DvbHw capability.
         *
         * @return the DvbHw capability type
         * @see Solid::Ifaces::Enums::Capability::Type
         */
        static Type capabilityType() { return Capability::DvbHw; }


        /**
         * Retrieves the absolute path of the special file to interact
         * with the device.
         *
         * @return the absolute path of the special file to interact with
         * the device
         */
        QString device() const;


         /**
          * Retrieves the adapter number of this dvb device.
          * Note that -1 is returned in the case the adapter couldn't be
          * determined.
          *
          * @return the adapter number of this dvb device or -1
          */
         int deviceAdapter() const;


         /**
          * Retrieves the type of this dvb device.
          *
          * @return the device type of this dvb device
          * @see Solid::DvbHw::DeviceType
          */
         DeviceType deviceType() const;


         /**
          * Retrieves the index of this dvb device.
          * Note that -1 is returned in the case the device couldn't be
          * identified (deviceType() == DvbUnknown).
          *
          * @return the index of this dvb device or -1
          * @see Solid::DvbHw::deviceType
          */
         int deviceIndex() const;

    protected:
        /**
         * @internal
         */
        DvbHw(DvbHwPrivate &dd, QObject *backendObject);
    };
}

#endif
