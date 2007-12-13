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

#include "phonon/phononnamespace.h"
#include <QtCore/QtGlobal>

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

namespace Phonon
{
/** \class AudioOutputInterface audiooutputinterface.h Phonon/AudioOutputInterface
 * \short Interface for AudioOutput objects
 *
 * \ingroup Backend
 * \author Matthias Kretz <kretz@kde.org>
 */
class AudioOutputInterface
{
    public:
        virtual ~AudioOutputInterface() {}

        virtual qreal volume() const = 0;
        virtual void setVolume(qreal) = 0;

        virtual int outputDevice() const = 0;
        virtual bool setOutputDevice(int) = 0;
};
} // namespace Phonon

Q_DECLARE_INTERFACE(Phonon::AudioOutputInterface, "AudioOutputInterface2.phonon.kde.org")

QT_END_NAMESPACE
QT_END_HEADER

#endif // PHONON_AUDIOOUTPUTINTERFACE_H
