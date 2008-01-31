/*  This file is part of the KDE project
    Copyright (C) 2008 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) version 3.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef PHONON_EXPERIMENTAL_MEDIASOURCE_P_H
#define PHONON_EXPERIMENTAL_MEDIASOURCE_P_H

#include "mediasource.h"
#include "../mediasource_p.h"
#include "objectdescription.h"

namespace Phonon
{
namespace Experimental
{

class MediaSourcePrivate : public Phonon::MediaSourcePrivate
{
    public:
        MediaSourcePrivate(MediaSource::Type t)
            : Phonon::MediaSourcePrivate(static_cast<Phonon::MediaSource::Type>(t))
        {
        }

        VideoCaptureDevice videoCaptureDevice;
};

} // namespace Experimental
} // namespace Phonon

#endif // PHONON_EXPERIMENTAL_MEDIASOURCE_P_H
