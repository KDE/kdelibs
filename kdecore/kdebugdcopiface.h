/* This file is part of the KDE libraries
    Copyright (C) 2002 Andreas Beckermann (b_mann@gmx.de)

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef _KDEBUGDCOPIFACE_H_
#define _KDEBUGDCOPIFACE_H_

#include <dcopobject.h>
#include <dcopref.h>

/**
 * @short DCOP interface to @ref KDebug.
 **/
class KDebugDCOPIface : virtual public DCOPObject
{
	K_DCOP
public:
	KDebugDCOPIface();
	~KDebugDCOPIface();

k_dcop:
	/**
	 * The kdebugrc has been changed and should be reparsed now.
	 * This will simply call @ref kdClearDebugConfig
	 **/
	void notifyKDebugConfigChanged();
};

#endif

