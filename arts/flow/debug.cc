    /*

    Copyright (C) 1999 Stefan Westerfeld
                       stefan@space.twc.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

#include "debug.h"
#include <stdarg.h>
#include <stdio.h>

static bool _artsdebug = true;

void setartsdebug(bool enable)
{
	_artsdebug = enable;
}

void artsdebug(const char *fmt,...)
{
	if(_artsdebug)
	{
    	va_list ap;
    	va_start(ap, fmt);
    	(void) vfprintf(stdout, fmt, ap);
    	va_end(ap);
	}
}
