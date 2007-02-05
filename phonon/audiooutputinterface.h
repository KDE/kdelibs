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

#ifndef PHONON_AUDIOOUTPUTINTERFACE_H
#define PHONON_AUDIOOUTPUTINTERFACE_H

#include "phononnamespace.h"
#include <Qt/qglobal.h>

namespace Phonon
{
class PHONONCORE_EXPORT AudioOutputInterface
{
    public:
        virtual ~AudioOutputInterface() {}

        virtual float volume() const = 0;
        virtual void setVolume(float) = 0;

        virtual int outputDevice() const = 0;
        virtual bool setOutputDevice(int) = 0;
};
} // namespace Phonon

Q_DECLARE_INTERFACE(Phonon::AudioOutputInterface, "org.kde.Phonon.AudioOutputInterface/0.1" )

#endif // PHONON_AUDIOOUTPUTINTERFACE_H
