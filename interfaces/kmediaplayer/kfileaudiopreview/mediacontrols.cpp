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
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <QtGui/QBoxLayout>
#include <QtGui/QToolButton>
#include <phonon/seekslider.h>
#include <phonon/volumeslider.h>
#include <kicon.h>

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

bool MediaControls::isLoopControlVisible() const
{
    Q_D(const MediaControls);
    return d->loopButton.isVisible();
}

void MediaControls::setMediaObject(MediaObject *media)
{
    Q_D(MediaControls);
    if (d->media) {
        disconnect(d->media, SIGNAL(destroyed()), this, SLOT(_k_mediaDestroyed()));
        disconnect(d->media, SIGNAL(stateChanged(Phonon::State, Phonon::State)), this,
                SLOT(_k_stateChanged(Phonon::State, Phonon::State)));
        disconnect(d->media, SIGNAL(finished()), this, SLOT(_k_finished()));
        disconnect(&d->playButton, SIGNAL(clicked()), d->media, SLOT(play()));
        disconnect(&d->pauseButton, SIGNAL(clicked()), d->media, SLOT(pause()));
        disconnect(&d->stopButton, SIGNAL(clicked()), d->media, SLOT(stop()));
    }
    d->media = media;
    if (media) {
        connect(media, SIGNAL(destroyed()), SLOT(_k_mediaDestroyed()));
        connect(media, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
                SLOT(_k_stateChanged(Phonon::State, Phonon::State)));
        connect(d->media, SIGNAL(finished()), this, SLOT(_k_finished()));
        connect(&d->playButton, SIGNAL(clicked()), media, SLOT(play()));
        connect(&d->pauseButton, SIGNAL(clicked()), media, SLOT(pause()));
        connect(&d->stopButton, SIGNAL(clicked()), media, SLOT(stop()));
    }

    d->seekSlider.setMediaObject(media);
}

void MediaControls::setAudioOutput(AudioOutput *audioOutput)
{
    Q_D(MediaControls);
    d->volumeSlider.setAudioOutput(audioOutput);
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

void MediaControls::setLoopControlVisible(bool vis)
{
    Q_D(MediaControls);
    d->loopButton.setVisible(vis);
}

void MediaControlsPrivate::_k_stateChanged(State newstate, State)
{
    switch(newstate)
    {
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

void MediaControlsPrivate::_k_finished()
{
    if (loopButton.isChecked()) {
        Q_ASSERT(media->state() == Phonon::StoppedState);
        media->play();
    }
}

} // namespace Phonon

#include "mediacontrols.moc"
// vim: sw=4 ts=4
