    /*

    Copyright (C) 2001 Jeff Tranter
                       tranter@kde.org

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

#include "artsflow.h"
#include "flowsystem.h"
#include "audiosubsys.h"
#include "debug.h"
#include "soundserverv2_impl.h"

using namespace Arts;

#include "config.h"

SoundServerV2_impl::SoundServerV2_impl() : _autoSuspendSeconds(60) {
}

string SoundServerV2_impl:: audioMethod() {
	return AudioSubSystem::the()->audioIO();
}

long SoundServerV2_impl:: samplingRate() {
	return AudioSubSystem::the()->samplingRate();
}

long SoundServerV2_impl:: channels() {
	return AudioSubSystem::the()->channels();
}

long SoundServerV2_impl:: bits() {
	return AudioSubSystem::the()->bits();
}

bool SoundServerV2_impl:: fullDuplex() {
	return AudioSubSystem::the()->fullDuplex();
}

string SoundServerV2_impl:: audioDevice() {
	return AudioSubSystem::the()->deviceName();
}

long SoundServerV2_impl::fragments() {
	return AudioSubSystem::the()->fragmentCount();
}

long SoundServerV2_impl::fragmentSize() {
	return AudioSubSystem::the()->fragmentSize();
}

long SoundServerV2_impl::autoSuspendSeconds() {
	return _autoSuspendSeconds;
}

void SoundServerV2_impl::autoSuspendSeconds(long int newValue) {
	_autoSuspendSeconds = newValue;
}

/*
 * This is just like the implementation in soundserver_impl.cc
 * except that the suspend time can vary.
 */
long SoundServerV2_impl::secondsUntilSuspend() {
	if (Dispatcher::the()->flowSystem()->suspended())
		return 0;
	if (Dispatcher::the()->flowSystem()->suspendable())
		return (autoSuspendSeconds()*5 - asCount)/5;
	return -1;
}

/*
 * This is just like the implementation in soundserver_impl.cc
 * except that the suspend time can vary.
 */
void SoundServerV2_impl:: notifyTime()
{
	static long lock = 0;
	assert(!lock);          // paranoid reentrancy check (remove me later)
	lock++;
	/*
     * Three times the same game: look if a certain object is still
     * active - if yes, keep, if no, remove
     */

    /* look for jobs which may have terminated by now */
    list<SoundServerJob *>::iterator i;

	i = jobs.begin();
	while(i != jobs.end())
	{
		SoundServerJob *job = *i;

		if(job->done())
		{
			delete job;
			jobs.erase(i);
			arts_debug("job finished");
			i = jobs.begin();
		}
		else i++;
	}

/*
 * AutoSuspend
 */
	if(Dispatcher::the()->flowSystem()->suspendable() &&
	   !Dispatcher::the()->flowSystem()->suspended())
	{
		asCount++;
		if(asCount > autoSuspendSeconds()*5)
		{
			Dispatcher::the()->flowSystem()->suspend();
			arts_info("sound server suspended");
		}
	}
	else
		asCount = 0;
	lock--;
}

#ifndef __SUNPRO_CC
/* See bottom of simplesoundserver_impl.cc for the reason this is here.  */
REGISTER_IMPLEMENTATION(SoundServerV2_impl);
#endif
