/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QtGui/QWidget>
#include <phonon/mediaobject.h>
#include <phonon/path.h>
#include <phonon/audiooutput.h>
#include <phonon/effect.h>
#include <phonon/videowidget.h>
#include "mediacontrols.h"
#include <phonon/effectwidget.h>
#include <kurl.h>

class MediaPlayer : public QWidget
{
    Q_OBJECT
    public:
        MediaPlayer(QWidget *parent = 0);
        void setUrl(const KUrl &url);

    private Q_SLOTS:
        void openEffectWidget();
        void toggleScaleMode(bool);
        void switchAspectRatio(int x);
        void setBrightness(int b);

    private:
        Phonon::MediaObject *m_media;
        Phonon::Path m_apath;
        Phonon::AudioOutput *m_aoutput;
        Phonon::Path m_vpath;
        Phonon::Effect *m_effect;
        Phonon::VideoWidget *m_vwidget;
        Phonon::MediaControls *m_controls;
        Phonon::EffectWidget *m_effectWidget;
};

// vim: ts=4
#endif // MEDIAPLAYER_H
