/*  This file is part of the KDE project
    Copyright (C) 2005-2007 Matthias Kretz <kretz@kde.org>

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
#include "effect_p.h"
#include "effectparameter.h"
#include "factory.h"

namespace Phonon
{
Effect::~Effect()
{
    delete k_ptr;
}

Effect::Effect(const AudioEffectDescription &description, QObject *parent)
    : QObject(parent)
    , k_ptr(new EffectPrivate)
{
    K_D(Effect);
    d->q_ptr = this;
    d->type = Effect::AudioEffect;
    d->id = description.index();
    d->createBackendObject();
}

Effect::Effect(const VideoEffectDescription &description, QObject *parent)
    : QObject(parent)
    , k_ptr(new EffectPrivate)
{
    K_D(Effect);
    d->q_ptr = this;
    d->type = Effect::VideoEffect;
    d->id = description.index();
    d->createBackendObject();
}

Effect::Effect(EffectPrivate &dd, QObject *parent)
    : QObject(parent)
    , k_ptr(&dd)
{
    K_D(Effect);
    d->q_ptr = this;
}

void EffectPrivate::createBackendObject()
{
    if (m_backendObject)
        return;
    Q_Q(Effect);
    switch (type) {
    case Effect::AudioEffect:
        m_backendObject = Factory::createAudioEffect(id, q);
        break;
    case Effect::VideoEffect:
        m_backendObject = Factory::createVideoEffect(id, q);
        break;
    }
    if (m_backendObject) {
        setupBackendObject();
    }
}

Effect::Type Effect::type() const
{
    K_D(const Effect);
    return d->type;
}

AudioEffectDescription Effect::audioDescription() const
{
    K_D(const Effect);
    if (d->type != Effect::AudioEffect)
        return AudioEffectDescription();
    return AudioEffectDescription::fromIndex(d->id);
}

VideoEffectDescription Effect::videoDescription() const
{
    K_D(const Effect);
    if (d->type != Effect::VideoEffect)
        return VideoEffectDescription();
    return VideoEffectDescription::fromIndex(d->id);
}

QList<EffectParameter> Effect::allDescriptions() const
{
    K_D(const Effect);
    QList<EffectParameter> ret;
    // there should be an iface object, but better be safe for those backend
    // switching corner-cases: when the backend switches the new backend might
    // not support this effect -> no iface object
    if (d->m_backendObject) {
        BACKEND_GET(QList<EffectParameter>, ret, "allDescriptions");
    }
    return ret;
}

EffectParameter Effect::description(int idx) const
{
    K_D(const Effect);
    EffectParameter ret;
    if (d->m_backendObject) {
        BACKEND_GET1(EffectParameter, ret, "description", int, idx);
    }
    return ret;
}

int Effect::parameterCount() const
{
    K_D(const Effect);
    int ret;
    if (d->m_backendObject) {
        BACKEND_GET(int, ret, "parameterCount");
    }
    return ret;
}

QVariant Effect::parameterValue(int index) const
{
    K_D(const Effect);
    if (!d->m_backendObject) {
        return d->parameterValues[index];
    }
    QVariant ret;
    BACKEND_GET1(QVariant, ret, "parameterValue", int, index);
    return ret;
}

void Effect::setParameterValue(int index, const QVariant &newValue)
{
    K_D(Effect);
    if (k_ptr->backendObject()) {
        BACKEND_CALL2("setParameterValue", int, index, QVariant, newValue);
    } else {
        d->parameterValues[index] = newValue;
    }
}

bool EffectPrivate::aboutToDeleteBackendObject()
{
    if (m_backendObject) {
        int count;
        pBACKEND_GET(int, count, "parameterCount");
        for (int i = 0; i < count; ++i) {
            pBACKEND_GET1(QVariant, parameterValues[i], "parameterValue", int, i);
        }
    }
    return true;
}

void EffectPrivate::setupBackendObject()
{
    Q_ASSERT(m_backendObject);

    // set up attributes
    int count;
    pBACKEND_GET(int, count, "parameterCount");
    for (int i = 0; i < count; ++i) {
        pBACKEND_CALL2("setParameterValue", int, i, QVariant, parameterValues[i]);
    }
}

} //namespace Phonon

#include "effect.moc"

// vim: sw=4 ts=4 tw=80
