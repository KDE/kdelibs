    /*

    Copyright (C) 2000 Hans Meine
                       hans@meine.de

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

#include "soundserver_impl.h"
#include "artsflow.h"
#include "flowsystem.h"
#include "audiosubsys.h"
#include "connect.h"
#include "debug.h"
#include <stdlib.h>
#include <iostream>
#include <algorithm>

using namespace Arts;

#include "config.h"

#ifdef HAVE_REALTIME_SCHED
#include <sched.h>
RealtimeStatus SoundServer_impl::realtimeStatus() {
	int sched = sched_getscheduler(0);
	if (sched == SCHED_FIFO || sched == SCHED_RR) return rtRealtime;
	if (!getenv("STARTED_THROUGH_ARTSWRAPPER")) return rtNoWrapper;
	return rtNoRealtime;
}
#else
RealtimeStatus SoundServer_impl::realtimeStatus() {
	return rtNoSupport;
}
#endif

long SoundServer_impl::secondsUntilSuspend() {
	if (Dispatcher::the()->flowSystem()->suspended())
		return 0;
	if (Dispatcher::the()->flowSystem()->suspendable() && autoSuspendTime == 0)
		return -2;
	if (Dispatcher::the()->flowSystem()->suspendable())
		return (autoSuspendTime*5 - asCount)/5;
	return -1;
};

bool SoundServer_impl::suspend() {
	if(Dispatcher::the()->flowSystem()->suspendable() &&
	   !Dispatcher::the()->flowSystem()->suspended())
		{
			Dispatcher::the()->flowSystem()->suspend();
			arts_info("sound server suspended by client");
			return true;
		}
	return false;
}

bool SoundServer_impl::terminate() {
	Dispatcher::the()->terminate();
	return true;
}


#ifndef __SUNPRO_CC
/* See bottom of simplesoundserver_impl.cc for the reason this is here.  */
REGISTER_IMPLEMENTATION(SoundServer_impl);
#endif
