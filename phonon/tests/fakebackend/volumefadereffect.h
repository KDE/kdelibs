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
#ifndef Phonon_FAKE_VOLUMEFADEREFFECT_H
#define Phonon_FAKE_VOLUMEFADEREFFECT_H

#include <QtCore/QDate>
#include "effect.h"
#include <phonon/volumefadereffect.h>
#include <phonon/volumefaderinterface.h>

namespace Phonon
{
namespace Fake
{
    /**
     * \author Matthias Kretz <kretz@kde.org>
     */
    class VolumeFaderEffect : public Effect, public Phonon::VolumeFaderInterface
    {
        Q_OBJECT
        Q_INTERFACES(Phonon::VolumeFaderInterface)

        public:
            VolumeFaderEffect(QObject *parent);
            ~VolumeFaderEffect();

            float volume() const;
            void setVolume(float volume);
            Phonon::VolumeFaderEffect::FadeCurve fadeCurve() const;
            void setFadeCurve(Phonon::VolumeFaderEffect::FadeCurve curve);
            void fadeTo(float volume, int fadeTime);

            void processBuffer(QVector<float> &buffer);

        private:
            float m_volume;
            float m_endvolume;
            int m_fadePosition;
            int m_fadeLength;
            int m_fadeTime;
            QTime m_fadeStart;
            Phonon::VolumeFaderEffect::FadeCurve m_fadeCurve;
    };
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80
#endif // Phonon_FAKE_VOLUMEFADEREFFECT_H
