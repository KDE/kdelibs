/* This file is part of the KDE project
   Copyright (C) 2001 Ian Reinhart Geiser <geiseri@yahoo.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KAPPDCOP_INTERFACE_H
#define KAPPDCOP_INTERFACE_H

#include <dcopobject.h>
#include <qstringlist.h>
#include <qcstring.h>
#include <dcopref.h>

class KApplication;

/**
This is the main interface to the @ref KApplication.  This will provide a consistant
dcop interface to all KDE applications that use it.
@short DCOP interface to @ref KApplication.
@author Ian Reinhart Geiser <geiseri@yahoo.com>
*/
class KAppDCOPInterface : virtual public DCOPObject
{
K_DCOP

public:
	/**
	Construct a new interface object.
	@param KApplication - The parent @ref KApplication object
	that will provide us with the functional interface.
	*/
	KAppDCOPInterface( KApplication * theKapp );
	/**
	Destructor
	Cleans up the dcop connection.
	**/
	~KAppDCOPInterface();
k_dcop:
	/**
	Disable current applications session management
	**/
	void disableSessionManagement();
	
	QCString startupId();
	QCString caption();
	
	void exit();

	
	

private:
	KApplication *m_KApplication;
};

#endif


