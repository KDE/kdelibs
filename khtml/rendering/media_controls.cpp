/*
 * Copyright (C) 2009 Michael Howell <mhowell123@gmail.com>.
 * Parts copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "media_controls.h"
#include <QtGui/QHBoxLayout>
#include <phonon/seekslider.h>
#include <KDE/KIcon>

namespace khtml {

MediaControls::MediaControls(Phonon::MediaObject* mediaObject, QWidget* parent) : QWidget(parent)
{
    setLayout(new QHBoxLayout(this));
    m_play = new QPushButton(KIcon("media-playback-start"), i18n("Play"), this);
    connect(m_play, SIGNAL(clicked()), mediaObject, SLOT(play()));
    layout()->addWidget(m_play);
    m_pause = new QPushButton(KIcon("media-playback-pause"), i18n("Pause"), this);
    connect(m_pause, SIGNAL(clicked()), mediaObject, SLOT(pause()));
    layout()->addWidget(m_pause);
    layout()->addWidget(new Phonon::SeekSlider(mediaObject, this));

    slotStateChanged(mediaObject->state());
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State, Phonon::State)), SLOT(slotStateChanged(Phonon::State)));
}

void MediaControls::slotStateChanged(Phonon::State state)
{
    if (state == Phonon::PlayingState) {
        m_play->hide();
	m_pause->show();
    } else {
        m_pause->hide();
	m_play->show();
    }
}

}

