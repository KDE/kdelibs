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

#ifndef SOLID_IFACES_SMARTCARDREADER_H
#define SOLID_IFACES_SMARTCARDREADER_H

#include <solid/ifaces/deviceinterface.h>
#include <solid/smartcardreader.h>

namespace Solid
{
namespace Ifaces
{
    /**
     * This device interface is available on smart card readers.
     */
    class SmartCardReader : virtual public DeviceInterface
    {
    public:
        /**
         * Destroys a SmartCardReader object.
         */
        virtual ~SmartCardReader();


        /**
         * Retrieves the type of this smart card reader.
         *
         * @return the reader type
         * @see Solid::SmartCardReader::ReaderType
         */
        virtual Solid::SmartCardReader::ReaderType readerType() const = 0;
    };
}
}

Q_DECLARE_INTERFACE(Solid::Ifaces::SmartCardReader, "org.kde.Solid.Ifaces.SmartCardReader/0.1")

#endif // SOLID_IFACES_SMARTCARDREADER_H
