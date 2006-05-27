/*  This file is part of the KDE project
    Copyright (C) 2004-2006 Matthias Kretz <kretz@kde.org>

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

#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <QWidget>
#include <phonon/phononnamespace.h>

class QSlider;
class QLabel;
class QString;
class QPushButton;
namespace Phonon
{
	class MediaObject;
	class AudioPath;
	class AudioOutput;
	class VideoPath;
	class VideoOutput;
	class VideoWidget;
}

using namespace Phonon;

class TestWidget : public QWidget
{
	Q_OBJECT
	public:
		TestWidget();
	private Q_SLOTS:
		void volchanged( int );
		void slotVolumeChanged( float );
		void tick( long );
		void stateChanged( Phonon::State );
		void seek( int );
		void length( long );
		void loadFile( const QString& );

	private:
		QSlider *m_volslider, *m_seekslider;
		QLabel *m_statelabel, *m_volumelabel, *m_totaltime, *m_currenttime, *m_remainingtime;
		QPushButton *m_pause, *m_play, *m_stop;
		MediaObject* m_media;
		AudioPath* path;
		AudioOutput* m_output;
		VideoPath* vpath;
		VideoWidget* vout;
		bool m_ticking;
		bool m_ignoreVolumeChange;
		long m_length;
};

#endif // TESTWIDGET_H
// vim: sw=4 ts=4 noet
