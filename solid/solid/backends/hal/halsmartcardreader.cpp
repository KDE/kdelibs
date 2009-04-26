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

#include "halsmartcardreader.h"

#include "haldevice.h"

#include <QtCore/QStringList>

using namespace Solid::Backends::Hal;

SmartCardReader::SmartCardReader(HalDevice *device)
    : DeviceInterface(device)
{

}

SmartCardReader::~SmartCardReader()
{

}

Solid::SmartCardReader::ReaderType SmartCardReader::readerType() const
{
    Solid::SmartCardReader::ReaderType type;
    QStringList capabilities = m_device->property("info.capabilities").toStringList();

    if (capabilities.contains("card_reader")) {
        type = Solid::SmartCardReader::CardReader;
    }
    if (capabilities.contains("crypto_token")) {
        type = Solid::SmartCardReader::CryptoToken;
    }

    return type;
}

#include "backends/hal/halsmartcardreader.moc"
