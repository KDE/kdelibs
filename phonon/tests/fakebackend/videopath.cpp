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

#include "videopath.h"
#include "videoeffect.h"
#include "abstractvideooutput.h"

namespace Phonon
{
namespace Fake
{

VideoPath::VideoPath(QObject *parent)
    : QObject(parent)
{
}

VideoPath::~VideoPath()
{
}

bool VideoPath::addOutput(QObject *videoOutputIface)
{
    Q_ASSERT(videoOutputIface);
    AbstractVideoOutput *vo = qobject_cast<Phonon::Fake::AbstractVideoOutput *>(videoOutputIface);
    Q_ASSERT(vo);
    Q_ASSERT(!m_outputs.contains(vo));
    m_outputs.append(vo);
    return true;
}

bool VideoPath::removeOutput(QObject *videoOutputIface)
{
    Q_ASSERT(videoOutputIface);
    AbstractVideoOutput *vo = qobject_cast<Phonon::Fake::AbstractVideoOutput *>(videoOutputIface);
    // this should be changed to an "if (vo) { ..." for production backends
    Q_ASSERT(vo);
    Q_ASSERT(m_outputs.removeAll(vo) == 1);
    return true;
}

bool VideoPath::insertEffect(QObject *newEffect, QObject *insertBefore)
{
    Q_ASSERT(newEffect);
    VideoEffect *ve = qobject_cast<VideoEffect *>(newEffect);
    Q_ASSERT(ve);
    VideoEffect *before = 0;
    if (insertBefore)
    {
        before = qobject_cast<VideoEffect *>(insertBefore);
        Q_ASSERT(before);
        if (!m_effects.contains(before))
            return false;
        m_effects.insert(m_effects.indexOf(before), ve);
    }
    else
        m_effects.append(ve);

    return true;
}

bool VideoPath::removeEffect(QObject *effect)
{
    Q_ASSERT(effect);
    VideoEffect *ve = qobject_cast<VideoEffect *>(effect);
    Q_ASSERT(ve);
    if (m_effects.removeAll(ve) > 0)
        return true;
    return false;
}

void VideoPath::processFrame(Phonon::Experimental::VideoFrame &frame)
{
    foreach (VideoEffect *effect, m_effects)
        effect->processFrame(frame); //modifies the frame
    foreach (AbstractVideoOutput *output, m_outputs)
        output->processFrame(frame);
}

}}

#include "videopath.moc"
// vim: sw=4 ts=4
