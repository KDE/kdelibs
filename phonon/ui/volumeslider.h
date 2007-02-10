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
class VolumeSliderPrivate;

/**
 * \short Widget providing a slider to control the volume of an AudioOutput.
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
class PHONONUI_EXPORT VolumeSlider : public QWidget
{
	Q_OBJECT
    Q_DECLARE_PRIVATE(VolumeSlider)
	/**
	 * This property holds the maximum volume that can be set with this slider.
	 *
	 * By default the maximum value is 1.0 (100%).
	 */
	Q_PROPERTY( float maximumVolume READ maximumVolume WRITE setMaximumVolume )
	/**
	 * This property holds the orientation of the slider.
	 *
	 * The orientation must be Qt::Vertical (the default) or Qt::Horizontal.
	 */
	Q_PROPERTY( Qt::Orientation orientation READ orientation WRITE setOrientation )

    /**
     * This property holds whether slider tracking is enabled.
     *
     * If tracking is enabled (the default), the volume changes
     * while the slider is being dragged. If tracking is
     * disabled, the volume changes only when the user
     * releases the slider.
     */
    Q_PROPERTY( bool tracking READ hasTracking WRITE setTracking )

    /**
     * This property holds the page step.
     *
     * The larger of two natural steps that a slider provides and
     * typically corresponds to the user pressing PageUp or PageDown.
     *
     * Defaults to 5 (5% of the voltage).
     */
    Q_PROPERTY( int pageStep READ pageStep WRITE setPageStep )

    /**
     * This property holds the single step.
     *
     * The smaller of two natural steps that a slider provides and
     * typically corresponds to the user pressing an arrow key.
     *
     * Defaults to 1 (1% of the voltage).
     */
    Q_PROPERTY( int singleStep READ singleStep WRITE setSingleStep )

    /**
     * This property holds whether the mute button/icon next to the slider is visible.
     *
     * By default the mute button/icon is visible.
     */
    Q_PROPERTY(bool muteVisible READ isMuteVisible WRITE setMuteVisible)
	public:
		/**
		 * Constructs a new volume slider with a \p parent.
		 */
		VolumeSlider( QWidget* parent = 0 );
		~VolumeSlider();

        bool hasTracking() const;
        void setTracking( bool tracking );
        int pageStep() const;
        void setPageStep( int milliseconds );
        int singleStep() const;
        void setSingleStep( int milliseconds );
        bool isMuteVisible() const;
		float maximumVolume() const;
		Qt::Orientation orientation() const;

	public Q_SLOTS:
		void setMaximumVolume( float );
		void setOrientation( Qt::Orientation );
        void setMuteVisible(bool);

		/**
		 * Sets the audio output object to be controlled by this slider.
		 */
		void setAudioOutput( AudioOutput* );

    private:
        Q_PRIVATE_SLOT(d_ptr, void _k_outputDestroyed())
        Q_PRIVATE_SLOT(d_ptr, void _k_sliderChanged(int))
        Q_PRIVATE_SLOT(d_ptr, void _k_volumeChanged(float))
        Q_PRIVATE_SLOT(d_ptr, void _k_mutedChanged(bool))
        Q_PRIVATE_SLOT(d_ptr, void _k_buttonClicked())

    protected:
        VolumeSliderPrivate* d_ptr;
};

} // namespace Phonon

// vim: sw=4 ts=4 et
#endif // PHONON_UI_VOLUMESLIDER_H
