/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

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

#ifndef TESTS_LOADFAKEBACKEND_H
#define TESTS_LOADFAKEBACKEND_H

#include "../factory.h"
#include <QtCore/QUrl>
#include <QtTest>
#include <stdlib.h>

namespace Phonon
{
void loadFakeBackend()
{
#ifdef USE_FAKE_BACKEND
    Factory::createBackend("phonon_fake", "0.1");
#endif
}
QUrl testUrl()
{
#ifdef USE_FAKE_BACKEND
    return QUrl("file:///foo.ogg");
#else
    QUrl url(getenv("PHONON_TESTURL"));
    if (!url.isValid()) {
        url = QUrl::fromLocalFile(getenv("PHONON_TESTURL"));
        if (!url.isValid()) {
            QWARN("You need to set PHONON_TESTURL to a valid URL. Expect to see failures.");
        }
    }
    return url;
#endif
}
} // namespace Phonon

#endif // TESTS_LOADFAKEBACKEND_H
