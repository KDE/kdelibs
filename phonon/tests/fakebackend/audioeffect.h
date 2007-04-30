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

#include <QObject>
#include <phonon/effectparameter.h>

namespace Phonon
{
namespace Fake
{
    class EffectInterface;

    class AudioEffect : public QObject
    {
        Q_OBJECT
        public:
            AudioEffect(int effectId, QObject *parent);
            ~AudioEffect();

            Q_INVOKABLE QList<EffectParameter> allDescriptions() const;
            Q_INVOKABLE EffectParameter description(int) const;
            Q_INVOKABLE int parameterCount() const;
            Q_INVOKABLE QVariant parameterValue(int) const;
            Q_INVOKABLE void setParameterValue(int, const QVariant &);

            // Fake specific:
            virtual void processBuffer(QVector<float> &buffer);

        private:
            EffectInterface *m_effect;
            QList<Phonon::EffectParameter> m_parameterList;
    };
}} //namespace Phonon::Fake

// vim: sw=4 ts=4 tw=80
#endif // Phonon_FAKE_AUDIOEFFECT_H
