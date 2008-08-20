/*
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>

    Win32 port:
    Copyright (C) 2004 Jarosław Staniek <staniek@kde.org>

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

#ifndef _KKEYSERVER_H
#define _KKEYSERVER_H

#include <kdeui_export.h>

#if defined Q_WS_X11 /*or defined Q_WS_WIN*/
#include "kkeyserver_x11.h"
#elif defined Q_WS_MACX
#include "kkeyserver_mac.h"
#elif defined Q_WS_WIN
#include "kkeyserver_win.h"
#endif

/**
 * A collection of functions for the conversion of key presses and
 * their modifiers from the window system specific format
 * to the generic format and vice-versa.
 */
namespace KKeyServer {
    /**
     * Converts the mask of ORed KKey::ModFlag modifiers to a
	 * user-readable string.
	 * @param mod the mask of ORed KKey::ModFlag modifiers
	 * @return the user-readable string
	 */
	KDEUI_EXPORT QString modToStringUser( uint mod );
    
	/**
     * Converts the modifier given as user-readable string
     * to KKey::ModFlag modifier, or 0.
     * @internal
	 */
	KDEUI_EXPORT uint stringUserToMod( const QString& mod );
} // namespace KKeyServer

#endif // !_KKEYSERVER_H
