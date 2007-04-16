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

#include "videoeffect.h"
#include "abstractvideooutput.h"

#define PHONON_CLASSNAME VideoPath

namespace Phonon
{
PHONON_OBJECT_IMPL

VideoPath::~VideoPath()
{
    K_D(VideoPath);
    foreach (AbstractVideoOutput *vo, d->outputs)
        d->removeDestructionHandler(vo, d);
    foreach (VideoEffect *ve, d->effects)
        d->removeDestructionHandler(ve, d);
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
            d->addDestructionHandler(videoOutput, d);
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

const QList<AbstractVideoOutput *> &VideoPath::outputs() const
{
    K_D(const VideoPath);
    return d->outputs;
}

bool VideoPath::insertEffect(VideoEffect *newEffect, VideoEffect *insertBefore)
{
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
            d->addDestructionHandler(newEffect, d);
            if (insertBefore)
                d->effects.insert(d->effects.indexOf(insertBefore), newEffect);
            else
                d->effects << newEffect;
            return true;
        }
    }
    return false;
}

bool VideoPath::removeEffect(VideoEffect *effect)
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

const QList<VideoEffect *> &VideoPath::effects() const
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

    QList<VideoEffect *> effectList = effects;
    foreach (VideoEffect *effect, effectList) {
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

void VideoPathPrivate::phononObjectDestroyed(Base *o)
{
    // this method is called from Phonon::Base::~Base(), meaning the VideoEffect
    // dtor has already been called, also virtual functions don't work anymore
    // (therefore qobject_cast can only downcast from Base)
    Q_ASSERT(o);
    AbstractVideoOutput *output = static_cast<AbstractVideoOutput *>(o);
    VideoEffect *videoEffect = static_cast<VideoEffect *>(o);
    if (outputs.contains(output))
    {
        if (m_backendObject && output->k_ptr->backendObject()) {
            pBACKEND_CALL1("removeOutput", QObject *, output->k_ptr->backendObject());
        }
        outputs.removeAll(output);
    }
    else if (effects.contains(videoEffect))
    {
        if (m_backendObject && videoEffect->k_ptr->backendObject()) {
            pBACKEND_CALL1("removeEffect", QObject *, videoEffect->k_ptr->backendObject());
        }
        effects.removeAll(videoEffect);
    }
}

} //namespace Phonon

#include "videopath.moc"

#undef PHONON_CLASSNAME
// vim: sw=4 ts=4 tw=80
