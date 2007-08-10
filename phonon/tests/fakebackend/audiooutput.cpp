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

#include "audiooutput.h"
#include <QtCore/QVector>

//#include <phonon/config-alsa.h>

#ifdef HAVE_SYS_SOUNDCARD_H
#include <sys/soundcard.h>
#endif
//#include <sys/ioctl.h>
#include <iostream>

namespace Phonon
{
namespace Fake
{
AudioOutput::AudioOutput(QObject *parent)
    : AbstractAudioOutput(parent)
    , m_device(10000)
    , m_dsp("/dev/dsp")
{
}

AudioOutput::~AudioOutput()
{
}

qreal AudioOutput::volume() const
{
    return m_volume;
}

int AudioOutput::outputDevice() const
{
    return m_device;
}

void AudioOutput::setVolume(qreal newVolume)
{
    m_volume = newVolume;
    emit volumeChanged(m_volume);
}

bool AudioOutput::setOutputDevice(int newDevice)
{
    Q_ASSERT(newDevice >= 10000);
    Q_ASSERT(newDevice <= 10001);
    m_device = newDevice;
    return true;
}

void AudioOutput::processBuffer(QVector<float> &_buffer)
{
    const QVector<float> &buffer(_buffer);
    // be nice to everybody using KDE with the fake backend and don't play this awful noise :)
    return;

    //static QFile indump("indump");
    //if (!indump.isOpen())
        //indump.open(QIODevice::WriteOnly);
    //static QFile outdump("outdump");
    //if (!outdump.isOpen())
        //outdump.open(QIODevice::WriteOnly);
    openDevice();
    if (!m_dsp.isOpen())
        return;

    // we dump the data in /dev/dsp
    qint16 *pcm = new qint16[2 *buffer.size()]; // 2 *for stereo
    char *towrite = reinterpret_cast<char *>(pcm);
    int converted;
    for (int i = 0; i < buffer.size(); ++i)
    {
        //indump.write(QByteArray::number(buffer[i]) + "\n");
        converted = static_cast<qint16>(m_volume * buffer[i] * static_cast<float>(0x7FFF));
        //outdump.write(QByteArray::number(converted) + "\n");
         *pcm++ = converted;
         *pcm++ = converted;
    }
    int size = sizeof(qint16) * 2 * buffer.size();
    int written;
    while (size > 0)
    {
        written = m_dsp.write(towrite, size);
        // QFSFileEngine loops until errno != EINTR
        if (written < 0)
            break;
        size = size - written;
        if (size > 0)
        {
            towrite += written;
            kWarning() << "only " << written << " bytes written to /dev/dsp";
        }
    }

    pcm -= 2 *buffer.size();
    delete[] pcm;
}

void AudioOutput::openDevice()
{
    if (m_dsp.isOpen())
        return;

#ifdef HAVE_SYS_SOUNDCARD_H
    if (m_dsp.open(QIODevice::WriteOnly))
    {
        int fd = m_dsp.handle();
        int format = AFMT_S16_LE;
        int stereo = 1;
        int samplingRate = 44100;
        ioctl(fd, SNDCTL_DSP_SETFMT, &format);
        ioctl(fd, SNDCTL_DSP_STEREO, &stereo);
        ioctl(fd, SNDCTL_DSP_SPEED, &samplingRate);
    }
#endif
}

void AudioOutput::closeDevice()
{
    m_dsp.close();
}

}} //namespace Phonon::Fake

#include "moc_audiooutput.cpp"
// vim: sw=4 ts=4
