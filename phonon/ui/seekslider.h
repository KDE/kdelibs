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

#ifndef PHONON_UI_SEEKSLIDER_H
#define PHONON_UI_SEEKSLIDER_H

#include <QWidget>
#include <kdelibs_export.h>
#include "../phononnamespace.h"

namespace Phonon
{
class AbstractMediaProducer;

class SeekSliderPrivate;

/**
 * \short Widget providing a slider for seeking in AbstractMediaProducer objects.
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
class PHONONUI_EXPORT SeekSlider : public QWidget
{
	Q_OBJECT
	Q_DECLARE_PRIVATE( SeekSlider )
	/**
	 * This property holds whether the icon next to the slider is visible.
	 *
	 * By default the icon is visible.
	 */
	Q_PROPERTY( bool iconVisible READ isIconVisible WRITE setIconVisible )

	/**
	 * This property holds whether slider tracking is enabled.
	 *
	 * If tracking is enabled (the default), the media seeks
	 * while the slider is being dragged. If tracking is
	 * disabled, the media seeks only when the user
	 * releases the slider.
	 */
	Q_PROPERTY( bool tracking READ hasTracking WRITE setTracking )

	/**
	 * This property holds the page step.
	 *
	 * The larger of two natural steps that a slider provides and
	 * typically corresponds to the user pressing PageUp or PageDown.
	 *
	 * Defaults to 5 seconds.
	 */
	Q_PROPERTY( int pageStep READ pageStep WRITE setPageStep )

	/**
	 * This property holds the single step.
	 *
	 * The smaller of two natural steps that a slider provides and
	 * typically corresponds to the user pressing an arrow key.
	 *
	 * Defaults to 0.5 seconds.
	 */
	Q_PROPERTY( int singleStep READ singleStep WRITE setSingleStep )

	public:
		/**
		 * Constructs a new seek slider with a \p parent.
		 */
		SeekSlider( QWidget* parent = 0 );
		~SeekSlider();

		bool isIconVisible() const;
		bool hasTracking() const;
		void setTracking( bool tracking );
		int pageStep() const;
		void setPageStep( int milliseconds );
		int singleStep() const;
		void setSingleStep( int milliseconds );

	public Q_SLOTS:
		void setIconVisible( bool );

		/**
		 * Sets the media producer object to be controlled by this slider.
		 */
		void setMediaProducer( AbstractMediaProducer* );

	private Q_SLOTS:
		void stateChanged( Phonon::State );
		void mediaDestroyed();
		void seek( int );
		void tick( qint64 );
		void length( qint64 );

	protected:
		SeekSlider( SeekSliderPrivate& d, QWidget* parent );
		SeekSliderPrivate* d_ptr;
};

} // namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // PHONON_UI_SEEKSLIDER_H
