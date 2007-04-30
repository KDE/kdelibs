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

#ifndef VOLUMESLIDER_P_H
#define VOLUMESLIDER_P_H

#include "volumeslider.h"
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QPixmap>
#include <QToolButton>
#include "factory.h"
#include <QIcon>

namespace Phonon
{
class AudioOutput;
class VolumeSliderPrivate
{
    Q_DECLARE_PUBLIC(VolumeSlider)
    protected:
        VolumeSliderPrivate(VolumeSlider *parent)
            : q_ptr(parent),
            layout(QBoxLayout::LeftToRight, parent),
            slider(Qt::Horizontal, parent),
            muteButton(parent),
            volumeIcon(Factory::icon("player-volume")),
            mutedIcon(Factory::icon("player-volume-muted")),
            output(0),
            ignoreVolumeChange(false)
        {
            slider.setRange(0, 100);
            slider.setPageStep(5);
            slider.setSingleStep(1);

            muteButton.setIcon(volumeIcon);
            muteButton.setAutoRaise(true);
            layout.setMargin(0);
            layout.setSpacing(2);
            layout.addWidget(&muteButton, 0, Qt::AlignVCenter);
            layout.addWidget(&slider, 0, Qt::AlignVCenter);
        }

        VolumeSlider *q_ptr;

        void _k_outputDestroyed();
        void _k_sliderChanged(int);
        void _k_volumeChanged(qreal);
        void _k_mutedChanged(bool);
        void _k_buttonClicked();

    private:
        QBoxLayout layout;
        QSlider slider;
        QToolButton muteButton;
        QIcon volumeIcon;
        QIcon mutedIcon;

        AudioOutput *output;
        bool ignoreVolumeChange;
};
} // namespace Phonon

#endif // VOLUMESLIDER_P_H
// vim: sw=4 sts=4 et tw=100
