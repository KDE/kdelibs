/*  This file is part of the KDE project
    Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>

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

#include "videowidget.h"
#include "videowidget_p.h"
#include "factory.h"

#include <phonon/ifaces/ui/videowidget.h>

namespace Phonon
{
namespace Ui
{

VideoWidget::VideoWidget( QWidget* parent )
	: QWidget( parent )
	, Phonon::AbstractVideoOutput( *new VideoWidgetPrivate( this ) )
{
	K_D( VideoWidget );
	d->createIface();
}

VideoWidget::VideoWidget( VideoWidgetPrivate& d, QWidget* parent )
	: QWidget( parent )
	, Phonon::AbstractVideoOutput( d )
{
}

Ifaces::VideoWidget* VideoWidget::iface()
{
	K_D( VideoWidget );
	if( !d->iface() )
		d->createIface();
	return d->iface();
}

void VideoWidgetPrivate::createIface()
{
	if( iface_ptr )
		return;
	K_Q( VideoWidget );
	setIface( Factory::self()->createVideoWidget( q ) );
	q->setupIface();
}

bool VideoWidget::isFullscreen() const
{
	K_D( const VideoWidget );
	return d->iface() ? d->iface()->isFullscreen() : d->fullscreen;
}

void VideoWidget::setFullscreen( bool newFullscreen )
{
	K_D( VideoWidget );
	if( iface() )
		d->iface()->setFullscreen( newFullscreen );
	else
		d->fullscreen = newFullscreen;
}

bool VideoWidgetPrivate::aboutToDeleteIface()
{
	if( iface() )
		fullscreen = iface()->isFullscreen();
	return AbstractVideoOutputPrivate::aboutToDeleteIface();
}

void VideoWidget::setupIface()
{
	K_D( VideoWidget );
	Q_ASSERT( d->iface() );
	AbstractVideoOutput::setupIface();

	QWidget* w = qobject_cast<QWidget*>( d->iface()->qobject() );
	d->layout.addWidget( w );
	d->iface()->setFullscreen( d->fullscreen );
	if( w )
		setSizePolicy( w->sizePolicy() );
}

/*
QSize VideoWidget::sizeHint()
{
	if( iface() )
	{
		QWidget* w = qobject_cast<QWidget*>( d->iface()->qobject() );
		if( w )
			return w->sizeHint();
	}
	return QSize( 0, 0 );
}

QSize VideoWidget::minimumSizeHint()
{
	if( iface() )
	{
		QWidget* w = qobject_cast<QWidget*>( d->iface()->qobject() );
		if( w )
			return w->minimumSizeHint();
	}
	return QSize( 0, 0 );
}*/

}} //namespace Phonon::Ui

#include "videowidget.moc"

// vim: sw=4 ts=4 tw=80 noet
