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

#include <QWidget>
#include <phonon/mediaobject.h>
#include <phonon/videopath.h>
#include <phonon/audiooutput.h>
#include <phonon/audiopath.h>
#include <phonon/audioeffect.h>
#include <phonon/brightnesscontrol.h>
#include <phonon/deinterlacefilter.h>
#include <phonon/ui/videowidget.h>
#include <phonon/ui/mediacontrols.h>
#include <phonon/ui/effectwidget.h>
#include <kurl.h>

class MediaPlayer : public QWidget
{
	Q_OBJECT
	public:
		MediaPlayer( QWidget* parent = 0 );
		void setUrl( const KUrl& url );

	private Q_SLOTS:
		void openEffectWidget();
        void toggleDeinterlacing(bool);
        void toggleScaleMode(bool);
        void switchAspectRatio(int x);

	private:
		Phonon::MediaObject* m_media;
		Phonon::AudioPath* m_apath;
		Phonon::AudioOutput* m_aoutput;
		Phonon::VideoPath* m_vpath;
		Phonon::AudioEffect* m_effect;
		Phonon::BrightnessControl* m_brightness;
		Phonon::VideoWidget* m_vwidget;
		Phonon::MediaControls* m_controls;
		Phonon::EffectWidget* m_effectWidget;
        Phonon::DeinterlaceFilter *m_deinterlaceFilter;
};

// vim: ts=4
#endif // MEDIAPLAYER_H
