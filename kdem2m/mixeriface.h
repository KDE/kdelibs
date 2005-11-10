/*  This file is part of the KDE project
    Copyright (C) 2004-2005 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef MIXERIFACE_H
#define MIXERIFACE_H

#include <dcopobject.h>

class MixerIface : public DCOPObject
{
	K_DCOP
	k_dcop:
		virtual QString name() const = 0;
		virtual float volume() const = 0;
		virtual void setVolume( float volume ) = 0;

	protected:
		MixerIface() : DCOPObject( "MixerIface" ) {}
};

#endif // MIXERIFACE_H
// vim: sw=4 ts=4 noet tw=80
