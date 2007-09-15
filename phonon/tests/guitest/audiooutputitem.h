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

#ifndef AUDIOOUTPUTITEM_H
#define AUDIOOUTPUTITEM_H

#include "sinkitem.h"
#include <Phonon/AudioOutput>
#include <Phonon/VolumeSlider>

class QModelIndex;

using Phonon::AudioOutput;
using Phonon::VolumeSlider;

class AudioOutputItem : public SinkItem
{
    Q_OBJECT
    public:
        AudioOutputItem(const QPoint &pos, QGraphicsView *widget);

        //enum { Type = UserType + 4 };
        //int type() const { return Type; }

        virtual MediaNode *mediaNode() { return &m_output; }
        virtual const MediaNode *mediaNode() const { return &m_output; }

    private slots:
        void deviceChange(const QModelIndex &modelIndex);

    private:
        AudioOutput m_output;
        VolumeSlider *m_volslider;
};

#endif // AUDIOOUTPUTITEM_H
