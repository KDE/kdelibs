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
	/**
	 * This property holds the maximum volume that can be set with this slider.
	 *
	 * By default the maximum value is 1.0 (100%).
	 */
	Q_PROPERTY( float maximumVolume READ maximumVolume WRITE setMaximumVolume )
	/**
	 * This property holds whether the icon next to the slider is visible.
	 *
	 * By default the icon is visible.
	 */
	Q_PROPERTY( bool iconVisible READ isIconVisible WRITE setIconVisible )
	/**
	 * This property holds the orientation of the slider.
	 *
	 * The orientation must be Qt::Vertical (the default) or Qt::Horizontal.
	 */
	Q_PROPERTY( Qt::Orientation orientation READ orientation WRITE setOrientation )
	public:
		/**
		 * Constructs a new volume slider with a \p parent.
		 */
		VolumeSlider( QWidget* parent = 0 );
		~VolumeSlider();

		float maximumVolume() const;
		bool isIconVisible() const;
		Qt::Orientation orientation() const;

	public Q_SLOTS:
		void setMaximumVolume( float );
		void setIconVisible( bool );
		void setOrientation( Qt::Orientation );

		/**
		 * Sets the audio output object to be controlled by this slider.
		 */
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
