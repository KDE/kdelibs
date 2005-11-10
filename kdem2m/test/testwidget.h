/*  This file is part of the KDE project
    Copyright (C) 2004-2005 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <QWidget>
#include <kdem2m/state.h>

class QSlider;
class QLabel;
class QString;
class QPushButton;
namespace KDEM2M
{
	class MediaObject;
	class AudioPath;
	class AudioOutput;
	class VideoPath;
	class VideoOutput;
}

using namespace KDEM2M;

class TestWidget : public QWidget
{
	Q_OBJECT
	public:
		TestWidget();
	private slots:
		void volchanged( int );
		void slotVolumeChanged( float );
		void tick( long );
		void stateChanged( KDEM2M::State );
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
		VideoOutput* vout;
		bool m_ticking;
		bool m_ignoreVolumeChange;
		long m_length;
};

#endif // TESTWIDGET_H
// vim: sw=4 ts=4 noet
