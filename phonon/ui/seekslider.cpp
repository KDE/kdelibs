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

SeekSlider::SeekSlider(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new SeekSliderPrivate(this))
{
}

/*SeekSlider::SeekSlider(SeekSliderPrivate &_d, QWidget *parent)
    : QWidget(parent)
    , d_ptr(&_d)
{
} */

SeekSlider::~SeekSlider()
{
    delete d_ptr;
}

void SeekSlider::setMediaProducer(AbstractMediaProducer *media)
{
    if (!media)
        return;

    Q_D(SeekSlider);
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
    ticking = true;
    slider.setRange(0, msec);
    ticking = false;
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
            ticking = true;
            slider.setValue(0);
            ticking = false;
            break;
        }
    }
}

void SeekSliderPrivate::setEnabled(bool x)
{
    slider.setEnabled(x);
    iconLabel.setPixmap(icon.pixmap(iconSize, x ? QIcon::Normal : QIcon::Disabled));
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
        ticking = true;
        slider.setValue(0);
        ticking = false;
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

void SeekSlider::setTracking(bool tracking)
{
    d_ptr->slider.setTracking(tracking);
}

int SeekSlider::pageStep() const
{
    return d_ptr->slider.pageStep();
}

void SeekSlider::setPageStep(int milliseconds)
{
    d_ptr->slider.setPageStep(milliseconds);
}

int SeekSlider::singleStep() const
{
    return d_ptr->slider.singleStep();
}

void SeekSlider::setSingleStep(int milliseconds)
{
    d_ptr->slider.setSingleStep(milliseconds);
}

bool SeekSlider::isIconVisible() const
{
    Q_D(const SeekSlider);
    return d->iconLabel.isVisible();
}

void SeekSlider::setIconVisible(bool vis)
{
    Q_D(SeekSlider);
    d->iconLabel.setVisible(vis);
}

Qt::Orientation SeekSlider::orientation() const
{
    return d_ptr->slider.orientation();
}

void SeekSlider::setOrientation(Qt::Orientation o)
{
    Q_D(SeekSlider);
    Qt::Alignment align = (o == Qt::Horizontal ? Qt::AlignVCenter : Qt::AlignHCenter);
    d->layout.setAlignment(&d->iconLabel, align);
    d->layout.setAlignment(&d->slider, align);
    d->layout.setDirection(o == Qt::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
    d->slider.setOrientation(o);
}

QSize SeekSlider::iconSize() const
{
    return d_ptr->iconSize;
}

void SeekSlider::setIconSize(const QSize &iconSize)
{
    Q_D(SeekSlider);
    d->iconSize = iconSize;
    d->iconLabel.setPixmap(d->icon.pixmap(d->iconSize, d->slider.isEnabled() ? QIcon::Normal : QIcon::Disabled));
}

} // namespace Phonon

#include "seekslider.moc"
// vim: sw=4 ts=4
