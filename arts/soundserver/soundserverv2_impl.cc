    /*

    Copyright (C) 2001 Jeff Tranter
                       tranter@kde.org
				  2001 Stefan Westerfeld
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

#include "artsflow.h"
#include "flowsystem.h"
#include "audiosubsys.h"
#include "connect.h"
#include "debug.h"
#include "soundserverv2_impl.h"
#include "artsversion.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "config.h"

using namespace Arts;
using namespace std;

SoundServerV2_impl::SoundServerV2_impl()
	: _sampleStorage(SampleStorage(
		MCOPUtils::createFilePath("artsd-samples"),true))
{
	checkNewObjects();
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

PlayObject SoundServerV2_impl::createPlayObjectForStream(InputStream instream, const std::string& mimetype, bool createBUS)
{
	arts_debug("search streamplayobject, mimetype = %s", mimetype.c_str());

	TraderQuery query;
	query.supports("Interface","Arts::StreamPlayObject");
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
	else arts_warning("mimetype %s unsupported for streaming", mimetype.c_str());

	return PlayObject::null();
}

static void clearDirectory(const string& directory)
{
	DIR *dir = opendir(directory.c_str());
	if(!dir) return;

	struct dirent *de;
	while((de = readdir(dir)) != 0)
	{
		string currentEntry = directory + "/" + de->d_name;

		if(de->d_name[0] != '.')
		{
			unlink(currentEntry.c_str());
		}
	}
	closedir(dir);
}

static void doTypeIndex(string prefix, ModuleDef& module) /* copied from mcopidl */
{
	FILE *typeIndex = fopen((prefix+".mcopclass").c_str(),"w");

	vector<string> supportedTypes;

	vector<InterfaceDef>::iterator ii;
	for(ii = module.interfaces.begin(); ii != module.interfaces.end(); ii++)
		supportedTypes.push_back(ii->name);

	vector<TypeDef>::iterator ti;
	for(ti = module.types.begin(); ti != module.types.end(); ti++)
		supportedTypes.push_back(ti->name);

	string supportedTypesList;
	vector<string>::iterator si;
	bool first = true;
	for(si = supportedTypes.begin(); si != supportedTypes.end(); si++)
	{
		if(!first) supportedTypesList += ",";

		supportedTypesList += (*si);
		first = false;
	}
	fprintf(typeIndex, "# this file was generated by artsd - do not edit\n");
	fprintf(typeIndex,"Type=%s\n",supportedTypesList.c_str());
	fprintf(typeIndex,"TypeFile=%s.mcoptype\n",prefix.c_str());
	fclose(typeIndex);
}

void SoundServerV2_impl::checkNewObjects()
{
	const char *home = getenv("HOME");
	arts_return_if_fail(home != 0);

	string dir = home + string("/.mcop/trader-cache");

	mkdir(home,0755);
	mkdir((home+string("/.mcop")).c_str(),0755);
	mkdir(dir.c_str(),0755);

	int chdirOk = chdir(dir.c_str());
	arts_return_if_fail(chdirOk == 0);

	clearDirectory(dir);

	TraderQuery query;
	query.supports("Interface", "Arts::Loader");
	vector<TraderOffer> *offers = query.query();
	vector<TraderOffer>::iterator i;

	for(i = offers->begin(); i != offers->end(); i++)
	{
		// TODO: error checking?
		Arts::Loader loader = SubClass(i->interfaceName());

		/* put trader-information in ~/.mcop/trader-cache */
		vector<TraderEntry> *entries = loader.traderEntries();
		vector<TraderEntry>::iterator ei;
		for(ei = entries->begin(); ei != entries->end(); ei++)
		{
			const TraderEntry& entry = *ei;

			FILE *traderFile = fopen((dir+"/"+entry.interfaceName+".mcopclass").c_str(),"w");
			fprintf(traderFile, "# this file was generated by artsd - do not edit\n");
			vector<string>::const_iterator li;
			for(li = entry.lines.begin(); li != entry.lines.end(); li++)
				fprintf(traderFile,"%s\n", li->c_str());

			fclose(traderFile);
		}
		delete entries;

		/* put type-information in ~/.mcop/trader-cache */
		vector<ModuleDef> *modules = loader.modules();
		vector<ModuleDef>::iterator mi;
		for(mi = modules->begin(); mi != modules->end(); mi++)
		{
			Arts::ModuleDef& module = *mi;

			Buffer b;
			module.writeType(b);

			FILE *typeFile = fopen((module.moduleName+".arts.mcoptype").c_str(),"w");
			unsigned long towrite = b.size();
			fwrite(b.read(towrite),1,towrite,typeFile);
			fclose(typeFile);

			doTypeIndex(module.moduleName+".arts",module);
		}
		delete modules;
	}
	delete offers;

	Dispatcher::the()->reloadTraderData();
}

#ifndef __SUNPRO_CC
/* See bottom of simplesoundserver_impl.cc for the reason this is here.  */
REGISTER_IMPLEMENTATION(SoundServerV2_impl);
#endif
