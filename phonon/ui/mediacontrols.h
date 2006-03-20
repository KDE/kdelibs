/*  This file is part of the KDE project
    Copyright (C) 2006 Matthias Kretz <kretz@kde.org>

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

#ifndef PHONON_UI_MEDIACONTROLS_H
#define PHONON_UI_MEDIACONTROLS_H

#include <QWidget>
#include <kdelibs_export.h>
#include "../phononnamespace.h"

namespace Phonon
{
class AbstractMediaProducer;
class AudioOutput;

namespace Ui
{

/**
 * \short Simple widget showing buttons to control an AbstractMediaProducer
 * object.
 *
 * This widget shows the standard player controls. There's at least the
 * play/pause and stop buttons. If the media is seekable it shows a seek-slider.
 * Optional controls include a volume control and a loop control button.
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
class PHONON_EXPORT MediaControls : public QWidget
{
	Q_OBJECT
	public:
		MediaControls( QWidget* parent = 0 );
		~MediaControls();

		bool isSeekSliderVisible() const;
		bool isVolumeControlVisible() const;
		bool isLoopControlVisible() const;

	public Q_SLOTS:
		void setMediaProducer( AbstractMediaProducer* );
		void setAudioOutput( AudioOutput* audioOutput );
		void setSeekSliderVisible( bool );
		void setVolumeControlVisible( bool );
		void setLoopControlVisible( bool );

	private Q_SLOTS:
		void stateChanged( Phonon::State, Phonon::State );
		void mediaDestroyed();

	private:
		class Private;
		Private* d;
};

}} // namespace Phonon::Ui

// vim: sw=4 ts=4 tw=80
#endif // PHONON_UI_MEDIACONTROLS_H
