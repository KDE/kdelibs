/*  This file is part of the KDE project
    Copyright (C) 2007 Matthias Kretz <kretz@kde.org>

    Permission to use, copy, modify, and distribute this software
    and its documentation for any purpose and without fee is hereby
    granted, provided that the above copyright notice appear in all
    copies and that both that the copyright notice and this
    permission notice and warranty disclaimer appear in supporting
    documentation, and that the name of the author not be used in
    advertising or publicity pertaining to distribution of the
    software without specific, written prior permission.

    The author disclaim all warranties with regard to this
    software, including all implied warranties of merchantability
    and fitness.  In no event shall the author be liable for any
    special, indirect or consequential damages or any damages
    whatsoever resulting from loss of use, data or profits, whether
    in an action of contract, negligence or other tortious action,
    arising out of or in connection with the use or performance of
    this software.

*/

#include "videoplayback.h"

#include <QVBoxLayout>

#include <phonon/videopath.h>
#include <phonon/audiooutput.h>
#include <phonon/audiopath.h>
#include <phonon/mediaobject.h>
#include <phonon/ui/videowidget.h>
#include <phonon/ui/mediacontrols.h>

using namespace Phonon;

MediaPlayer::MediaPlayer(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_vwidget = new VideoWidget(this);
    layout->addWidget(m_vwidget);
    m_vwidget->hide();

    m_vpath = new VideoPath(this);

        // might need VideoCategory here
        m_aoutput = new AudioOutput(Phonon::MusicCategory, this);
    m_apath = new AudioPath(this);

    m_media = new MediaObject(this);
    m_media->addVideoPath(m_vpath);
    m_vpath->addOutput(m_vwidget);
    m_media->addAudioPath(m_apath);
    m_apath->addOutput(m_aoutput);

    m_controls = new MediaControls(this);
    layout->addWidget(m_controls);
    m_controls->setMediaProducer(m_media);
    m_controls->setAudioOutput(m_aoutput);

        connect(m_media, SIGNAL(hasVideoChanged(bool)), SLOT(hasVideoChanged(bool)));
}

void MediaPlayer::setUrl(const KUrl &url)
{
    m_media->setUrl(url);
    m_vwidget->setVisible(m_media->hasVideo());
}

void MediaPlayer::hasVideoChanged(bool hasVideo)
{
    m_vwidget->setVisible(hasVideo);
}

int main()
{
    return 0;
}

#include "videoplayback.moc"
