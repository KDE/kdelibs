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
#include "videoeffect.h"
#include "videoeffect_p.h"
#include "effectparameter.h"
#include "factory.h"

namespace Phonon
{

VideoEffect::VideoEffect(const VideoEffectDescription &type, QObject *parent)
    : QObject(parent)
    , Base(*new VideoEffectPrivate)
{
    K_D(VideoEffect);
    d->type = type.index();
    d->createBackendObject();
}

VideoEffect::VideoEffect(VideoEffectPrivate &dd, QObject *parent, const VideoEffectDescription &type)
    : QObject(parent)
    , Base(dd)
{
    K_D(VideoEffect);
    d->type = type.index();
}

void VideoEffectPrivate::createBackendObject()
{
    if (m_backendObject)
        return;
    K_Q(VideoEffect);
    m_backendObject = Factory::createVideoEffect(type, q);
    if (m_backendObject) {
        setupBackendObject();
    }
}

VideoEffectDescription VideoEffect::type() const
{
    K_D(const VideoEffect);
    return VideoEffectDescription::fromIndex(d->type);
}

QList<EffectParameter> VideoEffect::parameterList() const
{
    K_D(const VideoEffect);
    QList<EffectParameter> ret;
    // there should be an iface object, but better be safe for those backend
    // switching corner-cases: when the backend switches the new backend might
    // not support this effect -> no iface object
    if (d->m_backendObject)
    {
        BACKEND_GET(QList<EffectParameter>, ret, "parameterList");
        for (int i = 0; i < ret.size(); ++i)
            ret[i].setEffect(const_cast<VideoEffect *>(this));
    }
    return ret;
}

QVariant VideoEffect::value(int parameterId) const
{
    K_D(const VideoEffect);
    if (!d->m_backendObject)
        return d->parameterValues[parameterId];
    QVariant ret;
    BACKEND_GET1(QVariant, ret, "value", int, parameterId);
    return ret;
}

void VideoEffect::setValue(int parameterId, QVariant newValue)
{
    K_D(VideoEffect);
    if (k_ptr->backendObject())
        BACKEND_CALL2("setValue", int, parameterId, QVariant, newValue);
    else
        d->parameterValues[parameterId] = newValue;
}

bool VideoEffectPrivate::aboutToDeleteBackendObject()
{
    if (m_backendObject)
    {
        QList<EffectParameter> plist;
        pBACKEND_GET(QList<EffectParameter>, plist, "parameterList");
        foreach (EffectParameter p, plist)
            parameterValues[p.id()] = p.value();
    }
    return true;
}

void VideoEffectPrivate::setupBackendObject()
{
    Q_ASSERT(m_backendObject);

    // set up attributes
    QList<EffectParameter> plist;
    pBACKEND_GET(QList<EffectParameter>, plist, "parameterList");
    foreach (EffectParameter p, plist) {
        if (parameterValues.contains(p.id())) {
            p.setValue(parameterValues[p.id()]);
        }
    }
}

} //namespace Phonon

#include "videoeffect.moc"

// vim: sw=4 ts=4 tw=80
