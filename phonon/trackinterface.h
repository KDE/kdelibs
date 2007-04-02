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

#include "phonon_export.h"
#include <QObject>

namespace Phonon
{
class AbstractMediaProducer;
class TrackInterfacePrivate;

class PHONONCORE_EXPORT TrackInterface : public QObject
{
    Q_OBJECT
    public:
        TrackInterface(AbstractMediaProducer *);
        ~TrackInterface();

        bool isValid() const;

        int availableTracks() const;
        int currentTrack() const;

        bool autoplayTracks() const;

    public Q_SLOTS:
        void setCurrentTrack(int trackNumber);
        void setAutoplayTracks(bool);

        /**
         * Skips to the next track.
         *
         * If it was playing before the track change it will start playback on the next track if
         * autoplayTracks is enabled.
         */
        void nextTrack();

        /**
         * Skips to the previous track.
         *
         * If it was playing before the track change it will start playback on the previous track if
         * autoplayTracks is enabled.
         */
        void previousTrack();

    Q_SIGNALS:
        void availableTracksChanged(int availableTracks);
        void trackChanged(int trackNumber);

    private:
        TrackInterfacePrivate *const d;
};
} // namespace Phonon
#endif // PHONON_TRACKINTERFACE_H
