/*  This file is part of the KDE project
    Copyright (C) 2004-2007 Matthias Kretz <kretz@kde.org>

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

#include "videoplayer.h"
#include "mediaobject.h"
#include "audiopath.h"
#include "audiooutput.h"
#include "videopath.h"
#include "videowidget.h"
#include <kurl.h>
#include <QVBoxLayout>

namespace Phonon
{

class VideoPlayer::Private
{
    public:
        Private()
            : player(0)
        {
        }

        MediaObject *player;
        AudioPath *apath;
        VideoPath *vpath;
        AudioOutput *aoutput;
        VideoWidget *voutput;

        KUrl url;

        void _k_stateChanged(Phonon::State, Phonon::State);
};

VideoPlayer::VideoPlayer(Phonon::Category category, QWidget *parent)
    : QWidget(parent)
    , d(new Private)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    d->aoutput = new AudioOutput(category, this);
    d->apath = new AudioPath(this);
    d->apath->addOutput(d->aoutput);

    d->voutput = new VideoWidget(this);
    layout->addWidget(d->voutput);
    d->vpath = new VideoPath(this);
    d->vpath->addOutput(d->voutput);

    d->player = new MediaObject(this);
    d->player->addAudioPath(d->apath);
    d->player->addVideoPath(d->vpath);

    connect(d->player, SIGNAL(_k_stateChanged(Phonon::State, Phonon::State)),
            SLOT(_k_stateChanged(Phonon::State, Phonon::State)));
    connect(d->player, SIGNAL(finished()), SIGNAL(finished()));
}

VideoPlayer::~VideoPlayer()
{
}

void VideoPlayer::load(const KUrl &url)
{
    // new URL
    d->player->setCurrentSource(url);
    d->url = url;
}

void VideoPlayer::play(const KUrl &url)
{
    if (url == d->url)
    {
        if (!isPlaying())
            d->player->play();
        return;
    }
    // new URL
    d->player->setCurrentSource(url);
        
    if (ErrorState == d->player->state())
        return;

    d->url = url;

    if (StoppedState == d->player->state())
        d->player->play();
}

void VideoPlayer::play()
{
    play(d->url);
}

void VideoPlayer::pause()
{
    d->player->pause();
}

void VideoPlayer::stop()
{
    d->player->stop();
}

qint64 VideoPlayer::totalTime() const
{
    return d->player->totalTime();
}

qint64 VideoPlayer::currentTime() const
{
    return d->player->currentTime();
}

void VideoPlayer::seek(qint64 ms)
{
    d->player->seek(ms);
}

float VideoPlayer::volume() const
{
    return d->aoutput->volume();
}

void VideoPlayer::setVolume(float v)
{
    d->aoutput->setVolume(v);
}

bool VideoPlayer::isPlaying() const
{
    return (d->player->state() == PlayingState);
}

bool VideoPlayer::isPaused() const
{
    return (d->player->state() == PausedState);
}

void VideoPlayer::Private::_k_stateChanged(State ns, State os)
{
    if (os == LoadingState && ns == StoppedState)
        player->play();
}

} // namespaces

#include "videoplayer.moc"

// vim: sw=4 ts=4
