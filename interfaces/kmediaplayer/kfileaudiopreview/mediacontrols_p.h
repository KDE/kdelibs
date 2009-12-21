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

#ifndef PHONON_MEDIACONTROLS_P_H
#define PHONON_MEDIACONTROLS_P_H

#include "mediacontrols.h"
#include <kicon.h>
#include <klocale.h>
#include <phonon/volumeslider.h>
#include <phonon/seekslider.h>
#include <QtGui/QToolButton>
#include <QtGui/QBoxLayout>

namespace Phonon
{
class MediaControlsPrivate
{
    Q_DECLARE_PUBLIC(MediaControls)
    protected:
        MediaControlsPrivate(MediaControls *parent)
            : q_ptr(parent),
            layout(parent),
            playButton(parent),
            pauseButton(parent),
            stopButton(parent),
            loopButton(parent),
            seekSlider(parent),
            volumeSlider(parent),
            media(0)
        {
            playButton.setIconSize(QSize(32, 32));
            playButton.setIcon(KIcon("media-playback-start"));
            playButton.setToolTip(i18n("start playback"));
            playButton.setAutoRaise(true);

            pauseButton.setIconSize(QSize(32, 32));
            pauseButton.setIcon(KIcon("media-playback-pause"));
            pauseButton.setToolTip(i18n("pause playback"));
            pauseButton.hide();
            pauseButton.setAutoRaise(true);

            stopButton.setIconSize(QSize(32, 32));
            stopButton.setIcon(KIcon("media-playback-stop"));
            stopButton.setToolTip(i18n("stop playback"));
            stopButton.setAutoRaise(true);

            loopButton.setIconSize(QSize(32, 32));
            loopButton.setIcon(KIcon("view-refresh"));
            loopButton.setToolTip(i18n("loop: restarts playback at end"));
            loopButton.setAutoRaise(true);
            loopButton.setCheckable(true);

            volumeSlider.setOrientation(Qt::Horizontal);
            volumeSlider.setFixedWidth(80);
            volumeSlider.hide();

            layout.setMargin(0);
            layout.setSpacing(0);
            layout.addWidget(&playButton);
            layout.addWidget(&pauseButton);
            layout.addWidget(&stopButton);
            layout.addWidget(&loopButton);
            layout.addSpacing(8);
            layout.addWidget(&seekSlider, 1);
            layout.addSpacing(4);
            layout.addWidget(&volumeSlider);
            layout.addStretch();
        }

        MediaControls *q_ptr;
        QHBoxLayout layout;
        QToolButton playButton;
        QToolButton pauseButton;
        QToolButton stopButton;
        QToolButton loopButton;
        SeekSlider seekSlider;
        VolumeSlider volumeSlider;
        MediaObject *media;

    private:
        void _k_stateChanged(Phonon::State, Phonon::State);
        void _k_mediaDestroyed();
        void _k_finished();
};
} // namespace Phonon

#endif // PHONON_MEDIACONTROLS_P_H
// vim: sw=4 sts=4 et tw=100
