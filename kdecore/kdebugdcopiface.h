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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEBUGDCOPIFACE_H
#define KDEBUGDCOPIFACE_H

#include <dcopobject.h>
#include <kdelibs_export.h>

/**
 * @short DCOP interface to KDebug.
 **/
class KDECORE_EXPORT KDebugDCOPIface : virtual public DCOPObject
{
	K_DCOP
public:
	KDebugDCOPIface();
	~KDebugDCOPIface();

k_dcop:
	/**
	 * The kdebugrc has been changed and should be reparsed now.
	 * This will simply call kdClearDebugConfig
	 **/
	void notifyKDebugConfigChanged();

	/**
	 * Print out a kdBacktrace. Useful when trying to understand why
	 * a dialog is popping up, without having to launch gdb
	 */
	void printBacktrace();
};

#endif
