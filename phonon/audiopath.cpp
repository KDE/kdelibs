/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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
#include "audiopath_p.h"
#include "factory.h"

#include "effect.h"
#include "effect_p.h"
#include "abstractaudiooutput.h"
#include "abstractaudiooutput_p.h"

#define PHONON_CLASSNAME AudioPath

namespace Phonon
{
PHONON_OBJECT_IMPL

AudioPath::~AudioPath()
{
    K_D(AudioPath);
    foreach (AbstractAudioOutput *ao, d->outputs)
        ao->k_ptr->removeDestructionHandler(d);
    foreach (Effect *ae, d->effects)
        ae->k_ptr->removeDestructionHandler(d);
    delete k_ptr;
}

bool AudioPath::addOutput(AbstractAudioOutput *audioOutput)
{
    K_D(AudioPath);
    if (d->outputs.contains(audioOutput))
        return false;

    if (k_ptr->backendObject() && audioOutput->k_ptr->backendObject()) {
        bool success;
        BACKEND_GET1(bool, success, "addOutput", QObject *, audioOutput->k_ptr->backendObject());
        if (success)
        {
            audioOutput->k_ptr->addDestructionHandler(d);
            d->outputs << audioOutput;
            return true;
        }
    }
    return false;
}

bool AudioPath::removeOutput(AbstractAudioOutput *audioOutput)
{
    K_D(AudioPath);
    if (!d->outputs.contains(audioOutput))
        return false;

    if (d->m_backendObject && audioOutput->k_ptr->backendObject()) {
        bool success;
        BACKEND_GET1(bool, success, "removeOutput", QObject *, audioOutput->k_ptr->backendObject());
        if (success)
        {
            d->outputs.removeAll(audioOutput);
            return true;
        }
    }
    return false;
}

QList<AbstractAudioOutput *> AudioPath::outputs() const
{
    K_D(const AudioPath);
    return d->outputs;
}

bool AudioPath::insertEffect(Effect *newEffect, Effect *insertBefore)
{
    if (newEffect->type() != Effect::AudioEffect || (insertBefore && insertBefore->type() != Effect::AudioEffect)) {
        qWarning("AudioPath::insertEffect: Inserting a video effect into an audio path does not work!");
        return false;
    }
    // effects may be added multiple times, but that means the objects are
    // different (the class is still the same)
    K_D(AudioPath);
    if (d->effects.contains(newEffect))
        return false;

    if (k_ptr->backendObject() && newEffect->k_ptr->backendObject()) {
        bool success;
        BACKEND_GET2(bool, success, "insertEffect", QObject *, newEffect->k_ptr->backendObject(), QObject *, insertBefore ? insertBefore->k_ptr->backendObject() : 0);
        if (success)
        {
            newEffect->k_ptr->addDestructionHandler(d);
            if (insertBefore)
                d->effects.insert(d->effects.indexOf(insertBefore), newEffect);
            else
                d->effects << newEffect;
            return true;
        }
    }
    return false;
}

bool AudioPath::removeEffect(Effect *effect)
{
    K_D(AudioPath);
    if (!d->effects.contains(effect))
        return false;

    if (d->m_backendObject)
    {
        bool success;
        BACKEND_GET1(bool, success, "removeEffect", QObject *, effect->k_ptr->backendObject());
        if (success)
        {
            d->effects.removeAll(effect);
            return true;
        }
    }
    return false;
}

QList<Effect *> AudioPath::effects() const
{
    K_D(const AudioPath);
    return d->effects;
}

bool AudioPathPrivate::aboutToDeleteBackendObject()
{
    return true;
}

void AudioPathPrivate::setupBackendObject()
{
    Q_ASSERT(m_backendObject);

    // set up attributes
    bool success;
    QList<AbstractAudioOutput *> outputList = outputs;
    foreach (AbstractAudioOutput *output, outputList) {
        if (output->k_ptr->backendObject()) {
            pBACKEND_GET1(bool, success, "addOutput", QObject *, output->k_ptr->backendObject());
        } else {
            success = false;
        }
        if (!success) {
            outputs.removeAll(output);
        }
    }

    QList<Effect *> effectList = effects;
    foreach (Effect *effect, effectList) {
        if (effect->k_ptr->backendObject()) {
            pBACKEND_GET1(bool, success, "insertEffect", QObject *, effect->k_ptr->backendObject());
        } else {
            success = false;
        }
        if (!success) {
            effects.removeAll(effect);
        }
    }
}

void AudioPathPrivate::phononObjectDestroyed(BasePrivate *bp)
{
    // this method is called from Phonon::BasePrivate::~BasePrivate(), meaning the Effect
    // dtor has already been called and the private class is down to BasePrivate
    Q_ASSERT(bp);
    foreach (AbstractAudioOutput *ao, outputs) {
        if (ao->k_ptr == bp) {
            if (m_backendObject && bp->backendObject()) {
                pBACKEND_CALL1("removeOutput", QObject *, bp->backendObject());
            }
            outputs.removeAll(ao);
            return;
        }
    }
    foreach (Effect *ae, effects) {
        if (ae->k_ptr == bp) {
            if (m_backendObject && bp->backendObject()) {
                pBACKEND_CALL1("removeEffect", QObject *, bp->backendObject());
            }
            effects.removeAll(ae);
            return;
        }
    }
}

} //namespace Phonon

#include "audiopath.moc"

#undef PHONON_CLASSNAME
// vim: sw=4 ts=4 tw=80
