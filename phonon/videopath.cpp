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
#include "videopath.h"
#include "videopath_p.h"
#include "factory.h"

#include "effect.h"
#include "effect_p.h"
#include "abstractvideooutput.h"
#include "abstractvideooutput_p.h"

#define PHONON_CLASSNAME VideoPath

namespace Phonon
{
PHONON_OBJECT_IMPL

VideoPath::~VideoPath()
{
    K_D(VideoPath);
    foreach (AbstractVideoOutput *vo, d->outputs)
        vo->k_ptr->removeDestructionHandler(d);
    foreach (Effect *ve, d->effects)
        ve->k_ptr->removeDestructionHandler(d);
    delete k_ptr;
}

bool VideoPath::addOutput(AbstractVideoOutput *videoOutput)
{
    K_D(VideoPath);
    if (d->outputs.contains(videoOutput))
        return false;

    if (k_ptr->backendObject() && videoOutput->k_ptr->backendObject()) {
        bool success;
        BACKEND_GET1(bool, success, "addOutput", QObject *, videoOutput->k_ptr->backendObject());
        if (success)
        {
            videoOutput->k_ptr->addDestructionHandler(d);
            d->outputs << videoOutput;
            return true;
        }
    }
    return false;
}

bool VideoPath::removeOutput(AbstractVideoOutput *videoOutput)
{
    K_D(VideoPath);
    if (!d->outputs.contains(videoOutput))
        return false;

    if (d->m_backendObject && videoOutput->k_ptr->backendObject()) {
        bool success;
        BACKEND_GET1(bool, success, "removeOutput", QObject *, videoOutput->k_ptr->backendObject());
        if (success)
        {
            d->outputs.removeAll(videoOutput);
            return true;
        }
    }
    return false;
}

QList<AbstractVideoOutput *> VideoPath::outputs() const
{
    K_D(const VideoPath);
    return d->outputs;
}

bool VideoPath::insertEffect(Effect *newEffect, Effect *insertBefore)
{
    if (newEffect->type() != Effect::VideoEffect || (insertBefore && insertBefore->type() != Effect::VideoEffect)) {
        qWarning("VideoPath::insertEffect: Inserting an audio effect into a video path does not work!");
        return false;
    }
    // effects may be added multiple times, but that means the objects are
    // different (the class is still the same)
    K_D(VideoPath);
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

bool VideoPath::removeEffect(Effect *effect)
{
    K_D(VideoPath);
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

QList<Effect *> VideoPath::effects() const
{
    K_D(const VideoPath);
    return d->effects;
}

bool VideoPathPrivate::aboutToDeleteBackendObject()
{
    return true;
}

void VideoPathPrivate::setupBackendObject()
{
    Q_ASSERT(m_backendObject);

    // set up attributes
    bool success;
    QList<AbstractVideoOutput *> outputList = outputs;
    foreach (AbstractVideoOutput *output, outputList) {
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

void VideoPathPrivate::phononObjectDestroyed(BasePrivate *bp)
{
    // this method is called from Phonon::BasePrivate::~BasePrivate(), meaning the Effect
    // dtor has already been called and the private class is down to BasePrivate
    Q_ASSERT(bp);
    foreach (AbstractVideoOutput *ao, outputs) {
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

#include "videopath.moc"

#undef PHONON_CLASSNAME
// vim: sw=4 ts=4 tw=80
