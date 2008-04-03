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

#include "effect.h"
#include "delayaudioeffect.h"

namespace Phonon
{
namespace Fake
{
Effect::Effect(int effectId, QObject *parent)
    : QObject(parent), m_sink(0), m_effect(0)
{
    switch(effectId)
    {
    case 0x7F000001:
        m_effect = new DelayAudioEffect;
        m_parameterList.append(EffectParameter(1, "time", 0,
                    m_effect->value(1), 1.0, 15000.0, QVariantList(),
                    "Set's the delay in milliseconds"));
        m_parameterList.append(EffectParameter(2, "feedback", 0,
                m_effect->value(2), 0.0, 1.0));
        m_parameterList.append(EffectParameter(3, "level", 0,
                m_effect->value(3), 0.0, 1.0));
    }
    qSort(m_parameterList);
}

Effect::~Effect()
{
    delete m_effect;
    m_effect = 0;
}

QList<Phonon::EffectParameter> Effect::parameters() const
{
    return m_parameterList;
}

QVariant Effect::parameterValue(const EffectParameter &p) const
{
    if (m_effect) {
        return m_effect->value(p.id());
    }
    return QVariant();
}

bool Effect::setAudioSink(AudioNode *node)
{
    if (m_sink != 0 && node != 0)
        return false;

    m_sink = node;
    return true;
}


void Effect::setParameterValue(const EffectParameter &p, const QVariant &v)
{
    if (m_effect) {
        return m_effect->setValue(p.id(), v);
    }
}

void Effect::processBuffer(QVector<float> &buffer)
{
    if (m_effect)
        m_effect->processBuffer(buffer);
}

}} //namespace Phonon::Fake

#include "moc_effect.cpp"
// vim: sw=4 ts=4
