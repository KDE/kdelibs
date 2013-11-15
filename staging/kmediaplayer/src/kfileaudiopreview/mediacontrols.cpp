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

#include "mediacontrols.h"
#include "mediacontrols_p.h"

#include <phonon/mediaobject.h>
#include <phonon/audiooutput.h>
#include <QBoxLayout>
#include <QToolButton>
#include <phonon/seekslider.h>
#include <phonon/volumeslider.h>

namespace Phonon
{

MediaControls::MediaControls(QWidget *parent)
    : QWidget(parent),
      d_ptr(new MediaControlsPrivate(this))
{
    setMaximumHeight(32);
}

MediaControls::~MediaControls()
{
    delete d_ptr;
}

bool MediaControls::isSeekSliderVisible() const
{
    Q_D(const MediaControls);
    return d->seekSlider.isVisible();
}

bool MediaControls::isVolumeControlVisible() const
{
    Q_D(const MediaControls);
    return d->volumeSlider.isVisible();
}

void MediaControls::setMediaObject(MediaObject *media)
{
    Q_D(MediaControls);
    if (d->media) {
        disconnect(d->media, SIGNAL(destroyed()), this, SLOT(_k_mediaDestroyed()));
        disconnect(d->media, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this,
                   SLOT(_k_stateChanged(Phonon::State, Phonon::State)));
        disconnect(&d->playButton, SIGNAL(clicked()), d->media, SLOT(play()));
        disconnect(&d->pauseButton, SIGNAL(clicked()), d->media, SLOT(pause()));
    }
    d->media = media;
    if (media) {
        connect(media, SIGNAL(destroyed()), SLOT(_k_mediaDestroyed()));
        connect(media, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
                SLOT(_k_stateChanged(Phonon::State, Phonon::State)));
        connect(&d->playButton, SIGNAL(clicked()), media, SLOT(play()));
        connect(&d->pauseButton, SIGNAL(clicked()), media, SLOT(pause()));
    }

    d->seekSlider.setMediaObject(media);
}

void MediaControls::setAudioOutput(AudioOutput *audioOutput)
{
    Q_D(MediaControls);
    d->volumeSlider.setAudioOutput(audioOutput);
    d->updateVolumeSliderVisibility();
    d->volumeSlider.setVisible(audioOutput != 0);
}

void MediaControls::setSeekSliderVisible(bool vis)
{
    Q_D(MediaControls);
    d->seekSlider.setVisible(vis);
}

void MediaControls::setVolumeControlVisible(bool vis)
{
    Q_D(MediaControls);
    d->volumeSlider.setVisible(vis);
}

void MediaControls::resizeEvent(QResizeEvent*)
{
    Q_D(MediaControls);
    d->updateVolumeSliderVisibility();
}

void MediaControlsPrivate::updateVolumeSliderVisibility()
{
    bool isWide = q_ptr->width() > playButton.sizeHint().width() + seekSlider.sizeHint().width() + volumeSlider.sizeHint().width();
    bool hasAudio = volumeSlider.audioOutput() != 0;
    volumeSlider.setVisible(isWide && hasAudio);
}

void MediaControlsPrivate::_k_stateChanged(State newstate, State)
{
    switch (newstate) {
    case Phonon::LoadingState:
    case Phonon::PausedState:
    case Phonon::StoppedState:
        playButton.show();
        pauseButton.hide();
        break;
    case Phonon::BufferingState:
    case Phonon::PlayingState:
        playButton.hide();
        pauseButton.show();
        break;
    case Phonon::ErrorState:
        return;
    }
}

void MediaControlsPrivate::_k_mediaDestroyed()
{
    media = 0;
}

} // namespace Phonon

#include "moc_mediacontrols.cpp"
// vim: sw=4 ts=4
