    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

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

/*
 * BC: this is here for binary compatibility only - new applications should
 * be recompiled (including new debug.h) and will then not use this any longer
 */

#ifndef KDE_USE_FINAL

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
#endif
