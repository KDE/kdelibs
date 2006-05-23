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

#include "visualization.h"
#include "audiopath.h"
#include "abstractvideooutput.h"
#include <phonon/ifaces/abstractvideooutput.h>

namespace Phonon
{
namespace Fake
{

Visualization::Visualization( QObject* parent )
	: QObject( parent )
{
}

int Visualization::visualization() const
{
	return m_visualization;
}

void Visualization::setVisualization( int newVisualization )
{
	m_visualization = newVisualization;
}

void Visualization::setAudioPath( Ifaces::AudioPath* audioPath )
{
	Q_ASSERT( audioPath );
	AudioPath* ap = qobject_cast<AudioPath*>( audioPath->qobject() );
	Q_ASSERT( ap );
	m_audioPath = ap;
}

void Visualization::setVideoOutput( Ifaces::AbstractVideoOutput* videoOutputIface )
{
	Q_ASSERT( videoOutputIface );
	AbstractVideoOutput* vo = reinterpret_cast<Phonon::Fake::AbstractVideoOutput*>( videoOutputIface->internal1() );
	Q_ASSERT( vo );
	m_videoOutput = vo;
}

}} //namespace Phonon::Fake

#include "visualization.moc"
// vim: sw=4 ts=4 noet
