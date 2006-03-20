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

#ifndef PHONON_UI_VOLUMESLIDER_H
#define PHONON_UI_VOLUMESLIDER_H

#include <QWidget>
#include <kdelibs_export.h>

namespace Phonon
{
class AudioOutput;

namespace Ui
{

/**
 * \short Widget providing a slider to control the volume of an AudioOutput.
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
class PHONON_EXPORT VolumeSlider : public QWidget
{
	Q_OBJECT
	Q_PROPERTY( float maximumVolume READ maximumVolume WRITE setMaximumVolume )
	Q_PROPERTY( bool iconVisible READ isIconVisible WRITE setIconVisible )
	public:
		VolumeSlider( QWidget* parent = 0 );
		~VolumeSlider();

		/**
		 * defaults to 1.0
		 */
		float maximumVolume() const;
		bool isIconVisible() const;

	public Q_SLOTS:
		void setMaximumVolume( float );
		void setIconVisible( bool );
		void setOrientation( Qt::Orientation );
		void setAudioOutput( AudioOutput* );

	private Q_SLOTS:
		void outputDestroyed();
		void sliderChanged( int );
		void volumeChanged( float );

	private:
		class Private;
		Private* d;
};

}} // namespace Phonon::Ui

// vim: sw=4 ts=4 tw=80
#endif // PHONON_UI_VOLUMESLIDER_H
