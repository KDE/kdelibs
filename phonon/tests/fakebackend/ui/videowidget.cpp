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

#include "videowidget.h"
#include <QPalette>
#include <QImage>
#include <QPainter>
#include <kdebug.h>

namespace Phonon
{
namespace Fake
{

VideoWidget::VideoWidget( QWidget* parent )
	: QWidget( parent )
{
	QPalette p = palette();
	p.setColor( QPalette::Window, Qt::blue );
	setPalette( p );
	setBackgroundRole( QPalette::Window );
	setAutoFillBackground( true );
	setMinimumSize( 100, 100 );
}

Phonon::VideoWidget::AspectRatio VideoWidget::aspectRatio() const
{
	return m_aspectRatio;
}

void VideoWidget::setAspectRatio( Phonon::VideoWidget::AspectRatio aspectRatio )
{
	kDebug( 604 ) << k_funcinfo << aspectRatio << endl;
	m_aspectRatio = aspectRatio;
	m_videoSize = size();
	const int w = width();
	const int h = height();
	switch( m_aspectRatio )
	{
		case Phonon::VideoWidget::AspectRatioWidget:
			break;
		case Phonon::VideoWidget::AspectRatioAuto:
		case Phonon::VideoWidget::AspectRatio4_3:
			if( h * 4 / 3 < w )
				m_videoSize.setWidth( h * 4 / 3 );
			else
				m_videoSize.setHeight( w * 3 / 4 );
			break;
		case Phonon::VideoWidget::AspectRatioSquare:
			if( h < w )
				m_videoSize.setWidth( h );
			else
				m_videoSize.setHeight( w );
			break;
		case Phonon::VideoWidget::AspectRatioAnamorphic:
			if( h * 16 / 9 < w )
				m_videoSize.setWidth( h * 16 / 9 );
			else
				m_videoSize.setHeight( w * 9 / 16 );
			break;
		case Phonon::VideoWidget::AspectRatioDvb:
			if( h * 211 / 100 < w )
				m_videoSize.setWidth( h * 211 / 100 );
			else
				m_videoSize.setHeight( w * 100 / 211 );
			break;
		default:
			Q_ASSERT( false ); // this can't happen
	}
}

void VideoWidget::processFrame( Phonon::VideoFrame& frame )
{
	switch( frame.fourcc )
	{
		case 0x00000000:
			{
				QImage image( reinterpret_cast<uchar*>( frame.data.data() ), frame.width, frame.height, QImage::Format_RGB32 );
				image = image.scaled( m_videoSize, Qt::IgnoreAspectRatio, Qt::FastTransformation );
				m_pixmap = QPixmap::fromImage( image );
				repaint();
			}
			break;
		default:
			kError( 604 ) << "video frame format not implemented" << endl;
	}
}

void VideoWidget::paintEvent( QPaintEvent* ev )
{
	QPainter p( this );
	p.drawPixmap( 0, 0, m_pixmap );
}

void VideoWidget::resizeEvent( QResizeEvent* ev )
{
	setAspectRatio( m_aspectRatio );
	QWidget::resizeEvent( ev );
}

}} //namespace Phonon::Fake

#include "videowidget.moc"
// vim: sw=4 ts=4 noet
