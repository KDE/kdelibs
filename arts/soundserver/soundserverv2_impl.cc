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
#include "connect.h"
#include "debug.h"
#include "soundserverv2_impl.h"
#include "artsversion.h"

#include "config.h"

using namespace Arts;
using namespace std;

SoundServerV2_impl::SoundServerV2_impl()
	: _sampleStorage(SampleStorage(
		MCOPUtils::createFilePath("artsd-samples"),true))
{
}

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

StereoVolumeControl SoundServerV2_impl::outVolume() {
	return _outVolume;
}

SampleStorage SoundServerV2_impl::sampleStorage() {
	return _sampleStorage;
}

PlayObject SoundServerV2_impl::createPlayObjectForURL(const std::string& url, const std::string& mimetype, bool createBUS)
{
    arts_debug("search playobject, mimetype = %s", mimetype.c_str());

    TraderQuery query;
    query.supports("Interface","Arts::PlayObject");
    query.supports("MimeType", mimetype);

    string objectType;

    vector<TraderOffer> *offers = query.query();
    if(!offers->empty())
	objectType = offers->front().interfaceName();	// first offer
    
    delete offers;

    /*
     * create a PlayObject and connect it
     */
    if(objectType != "")
    {
	arts_debug("creating %s to play file", objectType.c_str());

	PlayObject result = SubClass(objectType);
	if(result.loadMedia(url))
	{
	    if(createBUS)
	    {
		// TODO: check for existence of left & right streams
		Synth_BUS_UPLINK uplink;
		uplink.busname("out_soundcard");
		connect(result,"left",uplink,"left");
		connect(result,"right",uplink,"right");
		uplink.start();
		result._node()->start();
		result._addChild(uplink,"uplink");
		return result;
	    }
	    else
	        return result;
	}
	else arts_warning("couldn't load file %s", url.c_str());
    }
    else arts_warning("mimetype %s unsupported", mimetype.c_str());

    return PlayObject::null();
}

PlayObject SoundServerV2_impl::createPlayObjectForStream(InputStream instream, bool createBUS)
{
    // Hardcoding audio/x-mp3 ?
    std::string mimetype = "audio/x-mp3";
    arts_debug("search playobject, mimetype = %s", mimetype.c_str());

    TraderQuery query;
    query.supports("Interface","Arts::PlayObject");
    query.supports("MimeType", mimetype);

    string objectType;

    vector<TraderOffer> *offers = query.query();
    if(!offers->empty())
	objectType = offers->front().interfaceName();	// first offer
    
    delete offers;

    /*
     * create a PlayObject and connect it
     */
    if(objectType != "")
    {
	arts_debug("creating %s to play file", objectType.c_str());

	StreamPlayObject result = SubClass(objectType);
	result.streamMedia(instream);

	if(createBUS)
	{
	    // TODO: check for existence of left & right streams
	    Synth_BUS_UPLINK uplink;
	    uplink.busname("out_soundcard");
	    connect(result,"left",uplink,"left");
	    connect(result,"right",uplink,"right");
	    uplink.start();
	    result._node()->start();
	    result._addChild(uplink,"uplink");
	    return result;
	}
	else
	    return result;
    }
    else arts_warning("mimetype %s unsupported", mimetype.c_str());

    return PlayObject::null();
}

#ifndef __SUNPRO_CC
/* See bottom of simplesoundserver_impl.cc for the reason this is here.  */
REGISTER_IMPLEMENTATION(SoundServerV2_impl);
#endif
