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

bool VolumeSlider::isMuteVisible() const
{
    return d_ptr->muteButton.isVisible();
}

void VolumeSlider::setMuteVisible(bool visible)
{
    d_ptr->muteButton.setVisible(visible);
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

void VolumeSlider::setOrientation(Qt::Orientation o)
{
    Q_D(VolumeSlider);
    Qt::Alignment align = (o == Qt::Horizontal ? Qt::AlignVCenter : Qt::AlignHCenter);
    d->layout.setAlignment(&d->muteButton, align);
    d->layout.setAlignment(&d->slider, align);
    d->layout.setDirection(o == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
    d->slider.setOrientation(o);
}

void VolumeSlider::setAudioOutput(AudioOutput *output)
{
    if (!output) {
        return;
    }
    Q_D(VolumeSlider);

    d->output = output;
    d->slider.setValue(qRound(100 * output->volume()));
    d->slider.setEnabled(true);
    connect(output, SIGNAL(destroyed()), SLOT(_k_outputDestroyed()));
    connect(&d->slider, SIGNAL(valueChanged(int)), SLOT(_k_sliderChanged(int)));
    connect(&d->muteButton, SIGNAL(clicked()), SLOT(_k_buttonClicked()));
    connect(output, SIGNAL(volumeChanged(float)), SLOT(_k_volumeChanged(float)));
    connect(output, SIGNAL(mutedChanged(bool)), SLOT(_k_mutedChanged(bool)));
}

void VolumeSliderPrivate::_k_buttonClicked()
{
    if (output) {
        output->setMuted(!output->isMuted());
    }
}

void VolumeSliderPrivate::_k_mutedChanged(bool muted)
{
    if (muted) {
        muteButton.setIcon(mutedIcon);
    } else {
        muteButton.setIcon(volumeIcon);
    }
}

void VolumeSliderPrivate::_k_sliderChanged(int value)
{
    Q_Q(VolumeSlider);
    q->setToolTip(i18n("Volume: %1%", value));
    if (output) {
        ignoreVolumeChange = true;
        output->setVolume((static_cast<float>(value)) * 0.01);
        ignoreVolumeChange = false;
    }
}

void VolumeSliderPrivate::_k_volumeChanged(float value)
{
    if (!ignoreVolumeChange) {
        slider.setValue(qRound(100 * value));
    }
}

void VolumeSliderPrivate::_k_outputDestroyed()
{
    output = 0;
    slider.setValue(100);
    slider.setEnabled(false);
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
