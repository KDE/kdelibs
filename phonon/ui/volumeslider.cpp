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

#include "volumeslider.h"
#include "volumeslider_p.h"
#include "../audiooutput.h"
#include <klocale.h>

namespace Phonon
{


VolumeSlider::VolumeSlider( QWidget* parent )
    : QWidget( parent ),
    d_ptr( new VolumeSliderPrivate( this ) )
{
	setToolTip( i18n( "Volume: %1%", 100 ) );
	setWhatsThis( i18n( "Use this slider to adjust the volume. The leftmost position is 0%, the rightmost is %1%", 100 ) );
}

VolumeSlider::~VolumeSlider()
{
    delete d_ptr;
}

float VolumeSlider::maximumVolume() const
{
    return d_ptr->slider.maximum() * 0.01;
}

void VolumeSlider::setMaximumVolume( float volume )
{
	int max = static_cast<int>( volume * 100 );
    d_ptr->slider.setMaximum(max);
	setWhatsThis( i18n( "Use this slider to adjust the volume. The leftmost position is 0%, the rightmost is %1%" ,  max ) );
}

Qt::Orientation VolumeSlider::orientation() const
{
    return d_ptr->slider.orientation();
}

void VolumeSlider::setOrientation( Qt::Orientation o )
{
    d_ptr->slider.setOrientation(o);
}

void VolumeSlider::setAudioOutput( AudioOutput* output )
{
	if( !output )
		return;
    Q_D(VolumeSlider);

	d->output = output;
	d->slider.setValue( qRound( 100 * output->volume() ) );
	d->slider.setEnabled( true );
	connect( output, SIGNAL( destroyed() ), SLOT( outputDestroyed() ) );
	connect( &d->slider, SIGNAL( valueChanged( int ) ), SLOT( sliderChanged( int ) ) );
	connect( output, SIGNAL( volumeChanged( float ) ), SLOT( volumeChanged( float ) ) );
}

void VolumeSlider::sliderChanged( int value )
{
    Q_D(VolumeSlider);
	setToolTip( i18n( "Volume: %1%" ,  value ) );
	if( d->output )
	{
		d->ignoreVolumeChange = true;
		d->output->setVolume( ( static_cast<float>( value ) ) * 0.01 );
		d->ignoreVolumeChange = false;
	}
}

void VolumeSlider::volumeChanged( float value )
{
    Q_D(VolumeSlider);
	if( !d->ignoreVolumeChange )
		d->slider.setValue( qRound( 100 * value ) );
}

void VolumeSlider::outputDestroyed()
{
    Q_D(VolumeSlider);
	d->output = 0;
	d->slider.setValue( 100 );
	d->slider.setEnabled( false );
}

bool VolumeSlider::isIconVisible() const
{
    return d_ptr->icon.isVisible();
}

void VolumeSlider::setIconVisible( bool vis )
{
    d_ptr->icon.setVisible(vis);
}

bool VolumeSlider::hasTracking() const
{
    return d_ptr->slider.hasTracking();
}

void VolumeSlider::setTracking(bool tracking)
{
    d_ptr->slider.setTracking(tracking);
}

int VolumeSlider::pageStep() const
{
    return d_ptr->slider.pageStep();
}

void VolumeSlider::setPageStep(int milliseconds)
{
    d_ptr->slider.setPageStep(milliseconds);
}

int VolumeSlider::singleStep() const
{
    return d_ptr->slider.singleStep();
}

void VolumeSlider::setSingleStep(int milliseconds)
{
    d_ptr->slider.setSingleStep(milliseconds);
}

} // namespace Phonon

#include "volumeslider.moc"
// vim: sw=4 et
