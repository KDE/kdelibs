/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#include "volumefadereffect.h"
#include "volumefadereffect_p.h"
#include "factory.h"
#include <cmath>

#define PHONON_CLASSNAME VolumeFaderEffect

namespace Phonon
{
PHONON_HEIR_IMPL(Effect)

PHONON_GETTER(float, volume, d->currentVolume)
PHONON_SETTER(setVolume, currentVolume, float)

#ifndef PHONON_LOG10OVER20
#define PHONON_LOG10OVER20
static const double log10over20 = 0.1151292546497022842; // ln(10) / 20
#endif // PHONON_LOG10OVER20

double VolumeFaderEffect::volumeDecibel() const
{
    return -log(volume()) / log10over20;
}

void VolumeFaderEffect::setVolumeDecibel(double newVolumeDecibel)
{
    setVolume(exp(-newVolumeDecibel * log10over20));
}

PHONON_GETTER(Phonon::VolumeFaderEffect::FadeCurve, fadeCurve, d->fadeCurve)
PHONON_SETTER(setFadeCurve, fadeCurve, Phonon::VolumeFaderEffect::FadeCurve)

void VolumeFaderEffect::fadeIn(int fadeTime)
{
    fadeTo(1.0, fadeTime);
}

void VolumeFaderEffect::fadeOut(int fadeTime)
{
    fadeTo(0.0, fadeTime);
}

void VolumeFaderEffect::fadeTo(float volume, int fadeTime)
{
    K_D(VolumeFaderEffect);
    if (k_ptr->backendObject())
        BACKEND_CALL2("fadeTo", float, volume, int, fadeTime);
    else
        d->currentVolume = volume;
}

bool VolumeFaderEffectPrivate::aboutToDeleteBackendObject()
{
    if (m_backendObject)
    {
        pBACKEND_GET(float, currentVolume, "volume");
        pBACKEND_GET(Phonon::VolumeFaderEffect::FadeCurve, fadeCurve, "fadeCurve");
    }
    return true;
}

void VolumeFaderEffectPrivate::setupBackendObject()
{
    Q_ASSERT(m_backendObject);

    // set up attributes
    pBACKEND_CALL1("setVolume", float, currentVolume);
    pBACKEND_CALL1("setFadeCurve", Phonon::VolumeFaderEffect::FadeCurve, fadeCurve);
}
}

#include "volumefadereffect.moc"
#undef PHONON_CLASSNAME
// vim: sw=4 ts=4
