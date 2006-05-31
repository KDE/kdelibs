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
	public:
		/**
		 * Constructs a new seek slider with a \p parent.
		 */
		SeekSlider( QWidget* parent = 0 );
		~SeekSlider();

		bool isIconVisible() const;

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
		void tick( long );
		void length( long );

	protected:
		SeekSlider( SeekSliderPrivate& d, QWidget* parent );
		SeekSliderPrivate* d_ptr;
};

} // namespace Phonon

// vim: sw=4 ts=4 tw=80
#endif // PHONON_UI_SEEKSLIDER_H
