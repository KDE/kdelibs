/*  This file is part of the KDE project
    Copyright (C) 2009 Christopher Blauvelt <cblauvelt@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 3 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef SOLID_SMARTCARDREADER_H
#define SOLID_SMARTCARDREADER_H

#include <solid/solid_export.h>

#include <solid/deviceinterface.h>

namespace Solid
{
    class SmartCardReaderPrivate;
    class Device;

    /**
     * This device interface is available on smart card readers.
     */
    class SOLID_EXPORT SmartCardReader : public DeviceInterface
    {
        Q_OBJECT
        Q_ENUMS(ReaderType)
        Q_PROPERTY(ReaderType readerType READ readerType)
        Q_DECLARE_PRIVATE(SmartCardReader)
        friend class Device;

    public:
        /**
         * This enum type defines the type of smart card reader attached
         *
         * - CardReader : A generic smart card reader
         * - CryptoToken : A smart card reader with a card built into the device
         */
        enum ReaderType { UnknownReaderType = -1,
                          CardReader, CryptoToken };

    private:
        /**
         * Creates a new SmartCardReader object.
         * You generally won't need this. It's created when necessary using
         * Device::as().
         *
         * @param backendObject the device interface object provided by the backend
         * @see Solid::Device::as()
         */
        explicit SmartCardReader(QObject *backendObject);

    public:
        /**
         * Destroys a SmartCardReader object.
         */
        virtual ~SmartCardReader();


        /**
         * Get the Solid::DeviceInterface::Type of the SmartCardReader device interface.
         *
         * @return the SmartCardReader device interface type
         * @see Solid::DeviceInterface::Type
         */
        static Type deviceInterfaceType() { return DeviceInterface::SmartCardReader; }

        /**
         * Retrieves the type of this smart card reader.
         *
         * @return the smart card reader type
         * @see Solid::Ifaces::Enums::SmartCardReader::ReaderType
         */
        ReaderType readerType() const;
    };
}

#endif
