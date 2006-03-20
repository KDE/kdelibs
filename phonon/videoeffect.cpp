/*  This file is part of the KDE project
    Copyright (C) 2005 Matthias Kretz <kretz@kde.org>

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
#include "videoeffect.h"
#include "videoeffect_p.h"
#include "ifaces/videoeffect.h"
#include "factory.h"

namespace Phonon
{
PHONON_OBJECT_IMPL( VideoEffect )

QString VideoEffect::type() const
{
	K_D( const VideoEffect );
	return d->iface() ? d->iface()->type() : d->type;
}

void VideoEffect::setType( const QString& type )
{
	K_D( VideoEffect );
	if( d->iface() )
		d->iface()->setType( type );
	else
		d->type = type;
}

bool VideoEffectPrivate::aboutToDeleteIface()
{
	if( iface() )
	{
		type = iface()->type();
	}
	return true;
}

void VideoEffect::setupIface()
{
	K_D( VideoEffect );
	Q_ASSERT( d->iface() );

	// set up attributes
	d->iface()->setType( d->type );
}

} //namespace Phonon

#include "videoeffect.moc"

// vim: sw=4 ts=4 tw=80 noet
