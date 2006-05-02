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

#include "videodataoutput.h"
#include <kdebug.h>

namespace Phonon
{
namespace Fake
{
VideoDataOutput::VideoDataOutput( QObject* parent )
	: QObject( parent )
{
}

VideoDataOutput::~VideoDataOutput()
{
}

Phonon::VideoDataOutput::Format VideoDataOutput::format() const
{
	return m_format;
}

int VideoDataOutput::displayLatency() const
{
	return m_latency;
}

int VideoDataOutput::frameRate() const
{
	return 25;
}

void VideoDataOutput::setFormat( Phonon::VideoDataOutput::Format format )
{
	m_format = format;
}

void VideoDataOutput::setDisplayLatency( int milliseconds )
{
	m_latency = milliseconds;
}

void VideoDataOutput::processFrame( Phonon::VideoFrame& frame )
{
	if( frame.format == m_format )
		emit frameReady( frame );
	else
		kError( 604 ) << "format conversion not implemented" << endl;
	// TODO emit endOfMedia
}

}} //namespace Phonon::Fake

#include "videodataoutput.moc"
// vim: sw=4 ts=4 noet
