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

#include "videopath.h"
#include "videoeffect.h"

namespace Phonon
{
namespace Fake
{

VideoPath::VideoPath( QObject* parent )
	: QObject( parent )
{
}

VideoPath::~VideoPath()
{
}

bool VideoPath::addOutput( Ifaces::AbstractVideoOutput* videoOutput )
{
	Q_ASSERT( videoOutput );
	Q_ASSERT( !m_outputs.contains( videoOutput ) );
	m_outputs.append( videoOutput );
	return true;
}

bool VideoPath::removeOutput( Ifaces::AbstractVideoOutput* videoOutput )
{
	Q_ASSERT( videoOutput );
	Q_ASSERT( m_outputs.removeAll( videoOutput ) == 1 );
	return true;
}

bool VideoPath::insertEffect( Ifaces::VideoEffect* newEffect, Ifaces::VideoEffect* insertBefore )
{
	Q_ASSERT( newEffect );
	VideoEffect* ve = qobject_cast<VideoEffect*>( newEffect->qobject() );
	Q_ASSERT( ve );
	VideoEffect* before = 0;
	if( insertBefore )
	{
		before = qobject_cast<VideoEffect*>( insertBefore->qobject() );
		Q_ASSERT( before );
		if( !m_effects.contains( before ) )
			return false;
		m_effects.insert( m_effects.indexOf( before ), ve );
	}
	else
		m_effects.append( ve );

	return true;
}

bool VideoPath::removeEffect( Ifaces::VideoEffect* effect )
{
	Q_ASSERT( effect );
	VideoEffect* ve = qobject_cast<VideoEffect*>( effect->qobject() );
	Q_ASSERT( ve );
	if( m_effects.removeAll( ve ) > 0 )
		return true;
	return false;
}
}}

#include "videopath.moc"
// vim: sw=4 ts=4 noet
