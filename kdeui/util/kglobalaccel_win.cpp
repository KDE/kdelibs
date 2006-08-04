/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Ellis Whitehead <ellis@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"

#include <qwindowdefs.h>
#ifdef Q_WS_WIN

#include "kglobalaccel_win.h"
#include "kglobalaccel.h"

#include <kapplication.h>
#include <kdebug.h>

KGlobalAccelImpl::KGlobalAccelImpl(KGlobalAccel* owner)
	: m_owner(owner)
{
}

bool KGlobalAccelImpl::grabKey( int keyQt, bool grab )
{
	if( !keyQt ) {
		kWarning(125) << k_funcinfo << "Tried to grab key with null code." << endl;
		return false;
	}

	// TODO ...

	return false;
}

void KGlobalAccelImpl::enable( )
{
//	kapp->installWinEventFilter( this );
}

void KGlobalAccelImpl::disable( )
{
//	kapp->removeWinEventFilter( this );
}

#include "kglobalaccel_win.moc"

#endif // Q_WS_WIN
