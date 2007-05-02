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

#include "audiopath.h"
#include "audioeffect.h"
#include "abstractaudiooutput.h"
#include <QtCore/QVector>

namespace Phonon
{
namespace Fake
{

AudioPath::AudioPath(QObject *parent)
    : QObject(parent)
{
}

AudioPath::~AudioPath()
{
}

bool AudioPath::addOutput(QObject *audioOutput)
{
    Q_ASSERT(audioOutput);
    AbstractAudioOutput *ao = qobject_cast<AbstractAudioOutput *>(audioOutput);
    Q_ASSERT(ao);
    Q_ASSERT(!m_outputs.contains(ao));
    m_outputs.append(ao);
    return true;
}

bool AudioPath::removeOutput(QObject *audioOutput)
{
    Q_ASSERT(audioOutput);
    AbstractAudioOutput *ao = qobject_cast<AbstractAudioOutput *>(audioOutput);
    Q_ASSERT(ao);
    Q_ASSERT(m_outputs.removeAll(ao) > 0);
    return true;
}

bool AudioPath::insertEffect(QObject *newEffect, QObject *insertBefore)
{
    Q_ASSERT(newEffect);
    AudioEffect *ae = qobject_cast<AudioEffect *>(newEffect);
    Q_ASSERT(ae);
    AudioEffect *before = 0;
    if (insertBefore)
    {
        before = qobject_cast<AudioEffect *>(insertBefore);
        Q_ASSERT(before);
        if (!m_effects.contains(before))
            return false;
        m_effects.insert(m_effects.indexOf(before), ae);
    }
    else
        m_effects.append(ae);

    return true;
}

bool AudioPath::removeEffect(QObject *effect)
{
    Q_ASSERT(effect);
    AudioEffect *ae = qobject_cast<AudioEffect *>(effect);
    Q_ASSERT(ae);
    if (m_effects.removeAll(ae) > 0)
        return true;
    return false;
}

void AudioPath::processBuffer(const QVector<float> &_buffer)
{
    QVector<float> buffer(_buffer);
    foreach (AudioEffect *effect, m_effects)
        effect->processBuffer(buffer); //modifies the buffer
    foreach (AbstractAudioOutput *output, m_outputs)
        output->processBuffer(buffer);
}

}}

#include "audiopath.moc"
// vim: sw=4 ts=4
