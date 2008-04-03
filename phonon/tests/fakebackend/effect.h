/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

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
#ifndef Phonon_FAKE_AUDIOEFFECT_H
#define Phonon_FAKE_AUDIOEFFECT_H

#include <QtCore/QObject>
#include <phonon/effectparameter.h>
#include <phonon/effectinterface.h>
#include "audionode.h"

namespace Phonon
{
namespace Fake
{
    class EffectInterface;

    class Effect : public QObject, public Phonon::EffectInterface, public AudioNode
    {
        Q_OBJECT
        Q_INTERFACES(Phonon::EffectInterface Phonon::Fake::AudioNode)
        public:
            Effect(int effectId, QObject *parent);
            ~Effect();

            QList<EffectParameter> parameters() const;
            QVariant parameterValue(const EffectParameter &) const;
            void setParameterValue(const EffectParameter &, const QVariant &);

            // Fake specific:
            virtual void processBuffer(QVector<float> &buffer);

            bool setAudioSink(AudioNode *node);

        private:
            AudioNode *m_sink;
            Phonon::Fake::EffectInterface *m_effect;
            QList<Phonon::EffectParameter> m_parameterList;
    };
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80
#endif // Phonon_FAKE_AUDIOEFFECT_H
