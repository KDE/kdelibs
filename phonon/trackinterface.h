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

#ifndef PHONON_TRACKINTERFACE_H
#define PHONON_TRACKINTERFACE_H

#include <kdelibs_export.h>
#include <ksharedptr.h>

namespace Phonon
{
class AbstractMediaProducer;
class TrackInterfacePrivate;
class PHONONCORE_EXPORT TrackInterface
{
    friend class AbstractMediaProducer;
    public:
        //TrackInterface();
        TrackInterface(AbstractMediaProducer *);
        ~TrackInterface();
        TrackInterface(const TrackInterface &);
        TrackInterface &operator=(const TrackInterface &);
        bool operator==(const TrackInterface &);

        bool isValid() const;

        int availableTracks() const;
        int currentTrack() const;
        void setCurrentTrack(int trackNumber);

    private:
        KSharedPtr<TrackInterfacePrivate> d;
};
} // namespace Phonon
#endif // PHONON_TRACKINTERFACE_H
