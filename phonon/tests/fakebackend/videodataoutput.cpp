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

#include "videodataoutput.h"

namespace Phonon
{
namespace Fake
{
VideoDataOutput::VideoDataOutput(QObject *parent)
    : QObject(parent)
{
}

VideoDataOutput::~VideoDataOutput()
{
}

//X int VideoDataOutput::displayLatency() const
//X {
//X     return m_latency;
//X }

int VideoDataOutput::frameRate() const
{
    return m_frameRate;
}

void VideoDataOutput::setFrameRate(int frameRate)
{
    m_frameRate = frameRate;
}

QSize VideoDataOutput::naturalFrameSize() const
{
    return QSize(320, 240);
}

QSize VideoDataOutput::frameSize() const
{
    return m_frameSize;
}

void VideoDataOutput::setFrameSize(const QSize &frameSize)
{
    m_frameSize = frameSize;
}

//X void VideoDataOutput::setDisplayLatency(int milliseconds)
//X {
//X     m_latency = milliseconds;
//X }

void VideoDataOutput::processFrame(Phonon::Experimental::VideoFrame &frame)
{
    emit frameReady(frame);
    // TODO emit endOfMedia
}

}} //namespace Phonon::Fake

#include "moc_videodataoutput.cpp"
// vim: sw=4 ts=4
