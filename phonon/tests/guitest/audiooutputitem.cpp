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
#include <Phonon/AudioOutputDeviceModel>
#include <Phonon/BackendCapabilities>

using Phonon::AudioOutputDevice;
using Phonon::AudioOutputDeviceModel;

AudioOutputItem::AudioOutputItem(const QPoint &pos, QGraphicsView *widget)
    : SinkItem(pos, widget),
    m_output(Phonon::MusicCategory)
{
    setBrush(QColor(100, 255, 100, 150));

    m_output.setName("GUI-Test");

    QHBoxLayout *hlayout = new QHBoxLayout(m_frame);
    hlayout->setMargin(0);

    QListView *deviceListView = new QListView(m_frame);
    hlayout->addWidget(deviceListView);
    QList<AudioOutputDevice> deviceList = Phonon::BackendCapabilities::availableAudioOutputDevices();
    AudioOutputDeviceModel *model = new AudioOutputDeviceModel(deviceList, deviceListView);
    deviceListView->setModel(model);
    deviceListView->setCurrentIndex(model->index(deviceList.indexOf(m_output.outputDevice()), 0));
    connect(deviceListView, SIGNAL(activated(const QModelIndex &)), SLOT(deviceChange(const QModelIndex &)));

    m_volslider = new VolumeSlider(m_frame);
    m_volslider->setOrientation(Qt::Vertical);
    m_volslider->setAudioOutput(&m_output);
    hlayout->addWidget(m_volslider);
}

void AudioOutputItem::deviceChange(const QModelIndex &modelIndex)
{
    const int row = modelIndex.row();
    QList<AudioOutputDevice> deviceList = Phonon::BackendCapabilities::availableAudioOutputDevices();
    if (row >= 0 && row < deviceList.size()) {
        m_output.setOutputDevice(deviceList[row]);
    }
}

