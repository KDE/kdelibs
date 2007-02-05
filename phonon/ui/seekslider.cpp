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

#include "seekslider.h"
#include "seekslider_p.h"
#include "../abstractmediaproducer.h"
#include <kdebug.h>

namespace Phonon
{

SeekSlider::SeekSlider( QWidget* parent )
	: QWidget( parent )
	, d_ptr( new SeekSliderPrivate( this ) )
{
	Q_D( SeekSlider );
	d->layout.setMargin( 0 );
	d->layout.setSpacing( 2 );
	d->setEnabled( false );
	d->layout.addWidget( &d->icon );
	d->layout.addWidget( &d->slider );
}

SeekSlider::SeekSlider( SeekSliderPrivate& _d, QWidget* parent )
	: QWidget( parent )
	, d_ptr( &_d )
{
	Q_D( SeekSlider );
	d->layout.setMargin( 0 );
	d->layout.setSpacing( 2 );
	d->setEnabled( false );
	d->layout.addWidget( &d->icon );
	d->layout.addWidget( &d->slider );
}

SeekSlider::~SeekSlider()
{
	delete d_ptr;
}

void SeekSlider::setMediaProducer( AbstractMediaProducer* media )
{
	if( !media )
		return;

	Q_D( SeekSlider );
	d->media = media;
    connect(media, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
            SLOT(_k_stateChanged(Phonon::State)));
    connect(&d->slider, SIGNAL(valueChanged(int)), SLOT(_k_seek(int)));
    connect(media, SIGNAL(length(qint64)), SLOT(_k_length(qint64)));
    connect(media, SIGNAL(tick(qint64)), SLOT(_k_tick(qint64)));
    connect(media, SIGNAL(seekableChanged(bool)), SLOT(_k_seekableChanged(bool)));
    d->_k_stateChanged(media->state());
}

void SeekSliderPrivate::_k_seek(int msec)
{
    if (!ticking && media) {
        media->seek(msec);
    }
}

void SeekSliderPrivate::_k_tick(qint64 msec)
{
    ticking = true;
    slider.setValue(msec);
    ticking = false;
}

void SeekSliderPrivate::_k_length(qint64 msec)
{
    slider.setRange(0, msec);
}

void SeekSliderPrivate::_k_seekableChanged(bool isSeekable)
{
    if (!isSeekable) {
        setEnabled(false);
    } else {
        switch (media->state()) {
            case Phonon::PlayingState:
                if (media->tickInterval() == 0) {
                    // if the tick signal is not enabled the slider is useless
                    // set the tickInterval to some common value
                    media->setTickInterval(350);
                }
            case Phonon::BufferingState:
            case Phonon::PausedState:
                setEnabled(true);
                break;
            case Phonon::StoppedState:
            case Phonon::LoadingState:
            case Phonon::ErrorState:
                setEnabled(false);
                slider.setValue(0);
                break;
        }
    }
}

void SeekSliderPrivate::setEnabled( bool x )
{
	slider.setEnabled( x );
	icon.setPixmap( x ? iconPixmap : disabledIconPixmap );
}

void SeekSliderPrivate::_k_stateChanged(State newstate)
{
	if (!media->isSeekable()) {
        setEnabled(false);
        return;
    }
    switch (newstate) {
		case Phonon::PlayingState:
            if (media->tickInterval() == 0) {
				// if the tick signal is not enabled the slider is useless
				// set the tickInterval to some common value
                media->setTickInterval(350);
			}
		case Phonon::BufferingState:
		case Phonon::PausedState:
            setEnabled(true);
			break;
		case Phonon::StoppedState:
		case Phonon::LoadingState:
		case Phonon::ErrorState:
            setEnabled(false);
            slider.setValue(0);
			break;
	}
}

void SeekSliderPrivate::_k_mediaDestroyed()
{
    media = 0;
    setEnabled(false);
}

bool SeekSlider::hasTracking() const
{
	return d_ptr->slider.hasTracking();
}

void SeekSlider::setTracking( bool tracking )
{
	d_ptr->slider.setTracking( tracking );
}

int SeekSlider::pageStep() const
{
	return d_ptr->slider.pageStep();
}

void SeekSlider::setPageStep( int milliseconds )
{
	d_ptr->slider.setPageStep( milliseconds );
}

int SeekSlider::singleStep() const
{
	return d_ptr->slider.singleStep();
}

void SeekSlider::setSingleStep( int milliseconds )
{
	d_ptr->slider.setSingleStep( milliseconds );
}

bool SeekSlider::isIconVisible() const
{
	Q_D( const SeekSlider );
	return d->icon.isVisible();
}

void SeekSlider::setIconVisible( bool vis )
{
	Q_D( SeekSlider );
	d->icon.setVisible( vis );
}

} // namespace Phonon

#include "seekslider.moc"
// vim: sw=4 ts=4
