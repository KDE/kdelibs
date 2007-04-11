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

#include "delayaudioeffect.h"
#include <QVector>

namespace Phonon
{
namespace Fake
{

DelayAudioEffect::DelayAudioEffect()
    : m_feedback(0.5f)
    , m_level(0.4f)
{
    for (int i = 0; i < 22127; ++i)
        m_delayBuffer.enqueue(0.0f);
}

DelayAudioEffect::~DelayAudioEffect()
{
}

QVariant DelayAudioEffect::value(int parameterId) const
{
    switch(parameterId)
    {
    case 1:
        return static_cast<double>(m_delayBuffer.size()) / 44.1;
    case 2:
        return m_feedback;
    case 3:
        return m_level;
    }
    return 0.0;
}

void DelayAudioEffect::setValue(int parameterId, QVariant _newValue)
{
    const double newValue = qvariant_cast<double>(_newValue);
    switch(parameterId)
    {
    case 1:
        {
            const int newsize = qRound(44.1 * newValue);
            while (m_delayBuffer.size() < newsize)
                m_delayBuffer.enqueue(0.0f);
            while (m_delayBuffer.size() > newsize)
                m_delayBuffer.dequeue();
        }
        break;
    case 2:
        m_feedback = newValue;
    case 3:
        m_level = newValue;
    }
}

static inline float clamp(const float &min, const float &value, const float &max)
{
    return (min > value) ? min : (max < value) ? max : value;
}

void DelayAudioEffect::processBuffer(QVector<float> &buffer)
{
    //int enqueue;
    for (int i = 0; i < buffer.size(); ++i)
    {
        m_delayBuffer.enqueue(buffer[i] * m_level + m_delayBuffer.head() * m_feedback);
        buffer[i] = clamp(-1.0, buffer[i] + m_delayBuffer.dequeue(), 1.0);
        /*
        buffer[i] = clamp(-1.0, buffer[i] + m_delayBuffer.dequeue(), 1.0);
        enqueue = rand() / (RAND_MAX / 3);
        if (enqueue > 0 || m_delayBuffer.isEmpty())
        {
            m_delayBuffer.enqueue(buffer[i] * 0.11);
            if (enqueue > 1 || m_delayBuffer.size() < 100)
                m_delayBuffer.enqueue(buffer[i] * 0.2);
        }
         */
    }
}

}} //namespace Phonon::Fake

// vim: sw=4 ts=4
