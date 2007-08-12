/*  This file is part of the KDE project
    Copyright (C) 2007 Trolltech ASA. <thierry.bastian@trolltech.com>

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

#ifndef PHONON_VOLUMEFADEREFFECTINTERFACE_H
#define PHONON_VOLUMEFADEREFFECTINTERFACE_H

#include "volumefadereffect.h"
#include <QtCore/QObject>

namespace Phonon
{
class VolumeFaderInterface
{
    public:
        virtual ~VolumeFaderInterface() {}

        virtual float volume() const { return 1.0; }
        virtual void setVolume(float) {}
        virtual Phonon::VolumeFaderEffect::FadeCurve fadeCurve() const {
            return VolumeFaderEffect::Fade3Decibel;
        }
        virtual void setFadeCurve(Phonon::VolumeFaderEffect::FadeCurve) {}
        virtual void fadeTo(float, int) {}
        virtual QVariant parameterValue(int) const { return QVariant(); }
        virtual void setParameterValue(int, const QVariant &) {}

};
}

Q_DECLARE_INTERFACE(Phonon::VolumeFaderInterface, "VolumeFaderInterface3.phonon.kde.org")

#endif // PHONON_VOLUMEFADEREFFECTINTERFACE_H
