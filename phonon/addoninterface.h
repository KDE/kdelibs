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

#ifndef PHONON_ADDONINTERFACE_H
#define PHONON_ADDONINTERFACE_H

#include "phonon_export.h"
#include <QList>
#include <QVariant>

namespace Phonon
{
/**
 * \short Interface for Menu, Chapter, Angle and Track control.
 *
 * \ingroup Backend
 * \author Matthias Kretz <kretz@kde.org>
 */
class PHONONCORE_EXPORT AddonInterface
{
    public:
        virtual ~AddonInterface() {}

        enum Interface {
            MenuInterface    = 1,
            ChapterInterface = 2,
            AngleInterface   = 3,
            TrackInterface   = 4
        };

        enum MenuCommand {
            Menu1Button = 0x0001
        };
        enum ChapterCommand {
            availableChapters,
            chapter,
            setChapter
        };
        enum AngleCommand {
            availableAngles,
            angle,
            setAngle
        };
        enum TrackCommand {
            availableTracks,
            track,
            setTrack,
            autoplayTracks,
            setAutoplayTracks
        };

        virtual bool hasInterface(Interface interface) const = 0;

        virtual QVariant interfaceCall(Interface interface, int command,
                const QList<QVariant> &arguments = QList<QVariant>()) = 0;
};

} // namespace Phonon

Q_DECLARE_INTERFACE(Phonon::AddonInterface, "AddonInterface0.2.phonon.kde.org")

#endif // PHONON_ADDONINTERFACE_H
