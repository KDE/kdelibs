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

#include "videoeffect.h"
#include <QVariant>

namespace Phonon
{
namespace Fake
{
VideoEffect::VideoEffect(int effectId, QObject *parent)
    : QObject(parent)
{
}

VideoEffect::~VideoEffect()
{
}

QList<Phonon::EffectParameter> VideoEffect::allDescriptions() const
{
    return m_parameterList;
}

EffectParameter VideoEffect::description(int i) const
{
    return m_parameterList[i];
}

int VideoEffect::parameterCount() const
{
    return m_parameterList.size();
}

QVariant VideoEffect::parameterValue(int i) const
{
//X     if (m_effect) {
//X         return m_effect->value(m_parameterList[i].id());
//X     }
    return QVariant();
}

void VideoEffect::setParameterValue(int i, const QVariant &v)
{
//X     if (m_effect) {
//X         return m_effect->setValue(m_parameterList[i].id(), v);
//X     }
}

void VideoEffect::processFrame(Phonon::Experimental::VideoFrame &frame)
{
    // do nothing
}

}} //namespace Phonon::Fake

#include "videoeffect.moc"
// vim: sw=4 ts=4
