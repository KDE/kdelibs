/*    
	kimproxyiface.cpp
	
	IM service library for KDE
	
	DCOP interface to allow us to receive DCOP signals

	Copyright (c) 2004 Will Stephenson   <lists@stevello.free-online.co.uk>

    *************************************************************************
    *                                                                       *
    * This program is free software; you can redistribute it and/or modify  *
    * it under the terms of the GNU General Public License as published by  *
    * the Free Software Foundation; either version 2 of the License, or     *
    * (at your option) any later version.                                   *
    *                                                                       *
    *************************************************************************
*/

#ifndef KIMPROXYIFACE_H
#define KIMPROXYIFACE_H

#include <dcopobject.h>

class KIMProxyIface : virtual public DCOPObject
{
	K_DCOP
	k_dcop:
		virtual void contactStatusChanged( QString uid ) = 0;
};

#endif
