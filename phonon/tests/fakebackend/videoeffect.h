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
#ifndef Phonon_FAKE_VIDEOEFFECT_H
#define Phonon_FAKE_VIDEOEFFECT_H

#include <QObject>
#include <phonon/experimental/videoframe.h>
#include <phonon/effectparameter.h>

namespace Phonon
{
namespace Fake
{
    class VideoEffect : public QObject
    {
        Q_OBJECT
        public:
            VideoEffect(int effectId, QObject *parent);
            ~VideoEffect();
            Q_INVOKABLE QVariant value(int parameterId) const;
            Q_INVOKABLE void setValue(int parameterId, QVariant newValue);

            Q_INVOKABLE QList<EffectParameter> parameterList() const;

            // Fake specific:
            virtual void processFrame(Phonon::Experimental::VideoFrame &frame);

        private:
            QList<Phonon::EffectParameter> m_parameterList;
    };
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80
#endif // Phonon_FAKE_VIDEOEFFECT_H
