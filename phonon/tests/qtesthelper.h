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

#ifndef QTESTHELPER_H
#define QTESTHELPER_H

#include <qtest_kde.h>

namespace QTest
{
    template<>
    char *toString(const Phonon::State &state)
    {
        switch (state) {
            case Phonon::LoadingState:
                return qstrdup("LoadingState");
            case Phonon::StoppedState:
                return qstrdup("StoppedState");
            case Phonon::PlayingState:
                return qstrdup("PlayingState");
            case Phonon::BufferingState:
                return qstrdup("BufferingState");
            case Phonon::PausedState:
                return qstrdup("PausedState");
            case Phonon::ErrorState:
                return qstrdup("ErrorState");
        }
        return 0;
    }
} // namespace QTest

#endif // QTESTHELPER_H
