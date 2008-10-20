/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#ifndef SOLID_OPTICALDRIVE_H
#define SOLID_OPTICALDRIVE_H

#include <QtCore/QList>
#include <QtCore/QVariant>

#include <solid/solid_export.h>
#include <solid/solidnamespace.h>

#include <solid/storagedrive.h>

namespace Solid
{
    class OpticalDrivePrivate;
    class Device;

    /**
     * This device interface is available on CD-R*,DVD*,Blu-Ray,HD-DVD drives.
     *
     * A Cdrom is a storage that can handle optical discs.
     */
    class SOLID_EXPORT OpticalDrive : public StorageDrive
    {
        Q_OBJECT
        Q_ENUMS(MediumType)
        Q_FLAGS(MediumTypes)
        Q_PROPERTY(MediumTypes supportedMedia READ supportedMedia)
        Q_PROPERTY(int readSpeed READ readSpeed)
        Q_PROPERTY(int writeSpeed READ writeSpeed)
        Q_PROPERTY(QList<int> writeSpeeds READ writeSpeeds)
        Q_DECLARE_PRIVATE(OpticalDrive)
        friend class Device;

    public:
        /**
         * This enum type defines the type of medium a cdrom drive supports.
         *
         * - Cdr : A Recordable Compact Disc (CD-R)
         * - Cdrw : A ReWritable Compact Disc (CD-RW)
         * - Dvd : A Digital Versatile Disc (DVD)
         * - Dvdr : A Recordable Digital Versatile Disc (DVD-R)
         * - Dvdrw : A ReWritable Digital Versatile Disc (DVD-RW)
         * - Dvdram : A Random Access Memory Digital Versatile Disc (DVD-RAM)
         * - Dvdplusr : A Recordable Digital Versatile Disc (DVD+R)
         * - Dvdplusrw : A ReWritable Digital Versatile Disc (DVD+RW)
         * - Dvdplusdl : A Dual Layer Digital Versatile Disc (DVD+R DL)
         * - Dvdplusdlrw : A Dual Layer Digital Versatile Disc (DVD+RW DL)
         * - Bd : A Blu-ray Disc (BD)
         * - Bdr : A Blu-ray Disc Recordable (BD-R)
         * - Bdre : A Blu-ray Disc Recordable and Eraseable (BD-RE)
         * - HdDvd : A High Density Digital Versatile Disc (HD DVD)
         * - HdDvdr : A High Density Digital Versatile Disc Recordable (HD DVD-R)
         * - HdDvdrw : A High Density Digital Versatile Disc ReWritable (HD DVD-RW)
         */
        enum MediumType { Cdr=0x00001, Cdrw=0x00002, Dvd=0x00004, Dvdr=0x00008,
                          Dvdrw=0x00010, Dvdram=0x00020, Dvdplusr=0x00040,
                          Dvdplusrw=0x00080, Dvdplusdl=0x00100, Dvdplusdlrw=0x00200,
                          Bd=0x00400, Bdr=0x00800, Bdre=0x01000,
                          HdDvd=0x02000, HdDvdr=0x04000, HdDvdrw=0x08000 };

        /**
         * This type stores an OR combination of MediumType values.
         */
        Q_DECLARE_FLAGS(MediumTypes, MediumType)


    private:
        /**
         * Creates a new Cdrom object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit OpticalDrive(QObject *backendObject);

    public:
        /**
         * Destroys a Cdrom object.
         */
        virtual ~OpticalDrive();


        /**
         * Get the Solid::DeviceInterface::Type of the Cdrom device interface.
         *
         * @return the Cdrom device interface type
         * @see Solid::Ifaces::Enums::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::OpticalDrive; }


        /**
         * Retrieves the medium types this drive supports.
         *
         * @return the flag set indicating the supported medium types
         */
        MediumTypes supportedMedia() const;

        /**
         * Retrieves the maximum read speed of this drive in kilobytes per second.
         *
         * @return the maximum read speed
         */
        int readSpeed() const;

        /**
         * Retrieves the maximum write speed of this drive in kilobytes per second.
         *
         * @return the maximum write speed
         */
        int writeSpeed() const;

        /**
         * Retrieves the list of supported write speeds of this drive in
         * kilobytes per second.
         *
         * @return the list of supported write speeds
         */
        QList<int> writeSpeeds() const;

        /**
         * Ejects any disc that could be contained in this drive.
         * If this drive is empty, but has a tray it'll be opened.
         *
         * @return the status of the eject operation
         */
        bool eject();

    Q_SIGNALS:
        /**
         * This signal is emitted when the eject button is pressed
         * on the drive.
         *
         * Please note that some (broken) drives doesn't report this event.
         * @param udi the UDI of the drive
         */
        void ejectPressed(const QString &udi);

        void ejectDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

    };
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Solid::OpticalDrive::MediumTypes)

#endif // SOLID_OPTICALDRIVE_H
