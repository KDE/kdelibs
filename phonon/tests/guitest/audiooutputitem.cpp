/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

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

#include "audiooutputitem.h"
#include <QtCore/QModelIndex>
#include <QtGui/QHBoxLayout>
#include <QtGui/QListView>

#include <Phonon/AudioOutputDevice>
#include <Phonon/BackendCapabilities>
#include <kdebug.h>

using Phonon::AudioOutputDevice;

AudioOutputItem::AudioOutputItem(const QPoint &pos, QGraphicsView *widget)
    : SinkItem(pos, widget),
    m_output(Phonon::MusicCategory)
{
    setBrush(QColor(100, 255, 100, 150));
    setTitle("Audio Output");

    m_output.setName("GUI-Test");

    QHBoxLayout *hlayout = new QHBoxLayout(m_frame);
    hlayout->setMargin(0);

    m_deviceListView = new QListView(m_frame);
    hlayout->addWidget(m_deviceListView);
    QList<AudioOutputDevice> deviceList = Phonon::BackendCapabilities::availableAudioOutputDevices();
    m_model = new AudioOutputDeviceModel(deviceList, m_deviceListView);
    m_deviceListView->setModel(m_model);
    m_deviceListView->setCurrentIndex(m_model->index(deviceList.indexOf(m_output.outputDevice()), 0));
    connect(m_deviceListView, SIGNAL(activated(const QModelIndex &)), SLOT(deviceChange(const QModelIndex &)));

    m_volslider = new VolumeSlider(m_frame);
    m_volslider->setOrientation(Qt::Vertical);
    m_volslider->setAudioOutput(&m_output);
    hlayout->addWidget(m_volslider);

    connect(Phonon::BackendCapabilities::notifier(), SIGNAL(availableAudioOutputDevicesChanged()),
            SLOT(availableDevicesChanged()));
}

void AudioOutputItem::availableDevicesChanged()
{
    QList<AudioOutputDevice> deviceList = Phonon::BackendCapabilities::availableAudioOutputDevices();
    Q_ASSERT(m_model);
    m_model->setModelData(deviceList);
    m_deviceListView->setCurrentIndex(m_model->index(deviceList.indexOf(m_output.outputDevice()), 0));
}

void AudioOutputItem::deviceChange(const QModelIndex &modelIndex)
{
    Q_ASSERT(m_model);
    AudioOutputDevice device = m_model->modelData(modelIndex);
    m_output.setOutputDevice(device);
}
