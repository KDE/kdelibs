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

#include "fakesmartcardreader.h"

using namespace Solid::Backends::Fake;

FakeSmartCardReader::FakeSmartCardReader(FakeDevice *device)
    : FakeDeviceInterface(device)
{

}

FakeSmartCardReader::~FakeSmartCardReader()
{

}

Solid::SmartCardReader::ReaderType FakeSmartCardReader::readerType() const
{
    QString type = fakeDevice()->property("smartcardReaderType").toString();

    if (type == "reader") {
        return Solid::SmartCardReader::CardReader;
    } else if (type == "cryptoToken") {
        return Solid::SmartCardReader::CryptoToken;
    } else {
        return Solid::SmartCardReader::UnknownReaderType;
    }
}

#include "backends/fakehw/fakesmartcardreader.moc"
