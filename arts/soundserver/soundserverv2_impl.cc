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
#include "artsversion.h"

#include "config.h"

using namespace Arts;
using namespace std;

std::string SoundServerV2_impl:: audioMethod() {
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

std::string SoundServerV2_impl:: audioDevice() {
	return AudioSubSystem::the()->deviceName();
}

long SoundServerV2_impl::fragments() {
	return AudioSubSystem::the()->fragmentCount();
}

long SoundServerV2_impl::fragmentSize() {
	return AudioSubSystem::the()->fragmentSize();
}

long SoundServerV2_impl::autoSuspendSeconds() {
	return autoSuspendTime;
}

void SoundServerV2_impl::autoSuspendSeconds(long int newValue) {
	autoSuspendTime = newValue;
}

std::string SoundServerV2_impl::version() {
	return ARTS_VERSION;
}

long SoundServerV2_impl::bufferSizeMultiplier() {
	return bufferMultiplier;
}

void SoundServerV2_impl::bufferSizeMultiplier(long newValue) {
	bufferMultiplier = newValue;
}

#ifndef __SUNPRO_CC
/* See bottom of simplesoundserver_impl.cc for the reason this is here.  */
REGISTER_IMPLEMENTATION(SoundServerV2_impl);
#endif
