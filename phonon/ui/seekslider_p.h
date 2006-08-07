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

#ifndef SEEKSLIDER_P_H
#define SEEKSLIDER_P_H

#include "seekslider.h"
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>
#include <kicontheme.h>
#include <kiconloader.h>
#include <QPixmap>

namespace Phonon
{
class AbstractMediaProducer;
class SeekSliderPrivate
{
	Q_DECLARE_PUBLIC( SeekSlider )
	protected:
		SeekSliderPrivate( SeekSlider* parent )
			: layout( parent )
			, slider( Qt::Horizontal, parent )
			, icon( parent )
			, media( 0 )
			, ticking( false )
			, iconPixmap( SmallIcon( "player_time", 16, K3Icon::DefaultState ) )
			, disabledIconPixmap( SmallIcon( "player_time", 16, K3Icon::DisabledState ) )
		{
		}

		SeekSlider* q_ptr;
		
	private:
		void setEnabled( bool );

		QHBoxLayout layout;
		QSlider slider;
		QLabel icon;
		AbstractMediaProducer* media;
		bool ticking;
		QPixmap iconPixmap;
		QPixmap disabledIconPixmap;
};
} // namespace Phonon

#endif // SEEKSLIDER_P_H
// vim: sw=4 ts=4 tw=80
