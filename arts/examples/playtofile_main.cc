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

#include "playtofile.h"
#include "kmedia2.h"
#include "debug.h"
#include "dynamicrequest.h"
#include "connect.h"
#include "flowsystem.h"
#include <ctype.h>
#include <iostream>

using namespace Arts;
using namespace std;

/* copypasted from simplesoundserver_impl.cc */
PlayObject createPlayObject(const string& filename)
{
	string objectType = "";

	/*
	 * figure out extension (as lowercased letters)
	 */
	string extension = "";
	bool extensionok = false;
	string::const_reverse_iterator i;
	for(i = filename.rbegin(); i != filename.rend() && !extensionok; i++)
	{
		if(*i == '.')
			extensionok = true;
		else
			extension = (char)tolower(*i) + extension;
	}

	/*
	 * query trader for PlayObjects which support this
	 */
	if(extensionok)
	{
		arts_debug("search playobject, extension = %s",extension.c_str());

		TraderQuery query;
		query.supports("Interface","Arts::PlayObject");
		query.supports("Extension",extension);

		vector<TraderOffer> *offers = query.query();
		if(!offers->empty())
			objectType = offers->front().interfaceName();	// first offer

		delete offers;
	}

	/*
	 * create a PlayObject and connect it
	 */
	if(objectType != "")
	{
		arts_debug("creating %s to play file", objectType.c_str());

		PlayObject result = SubClass(objectType);
		if(result.loadMedia(filename))
		{
			result._node()->start();
			return result;
		}
		else arts_warning("couldn't load file %s", filename.c_str());
	}
	else arts_warning("file format extension %s unsupported",extension.c_str());

	return PlayObject::null();
}

/*
 * This is an example for dumping things to a file. It demonstates, that
 *
 * a) you can use all aRts objects in a non-realtime environment (i.e.
 *    the time in the PlayObject will pass much faster than the outside
 *    time when dumping things to a file)
 * 
 * b) complayObjectnents will be dynamically loaded as needed
 *
 * c) you need to do small tricks to get the signal flow going when you are
 *    not having audio modules around
 *
 * d) by implementing aRts modules, you can easily grab and insert data from/to
 *    arbitary non-aRts sources - for instance, a wave editor might implement
 *    own modules to get the wave from/to memory
 */
int main(int argc, char **argv)
{
	Dispatcher d;

	if(argc != 2 && argc != 3)
	{
		cerr << "usage: playtofile <inputfile> [ <outputfile> ]" << endl;
		exit(1);
	}
	PlayObject playObject = createPlayObject(argv[1]);
	if(playObject.isNull())
	{
		cerr << "can't read inputfile " << argv[1] << endl;
		exit(1);
	}

	playObject.play();

	PlayToFile playToFile;
	if(argc == 3) playToFile.filename(argv[2]);

	connect(playObject, "left", playToFile,"left");
	connect(playObject, "right", playToFile,"right");

	// <ignore the following lines>
	// special case when using mpeglib, do tell it not to block (internal
	// interface) - we also put an usleep here to ensure that the threads
	// and everything is fully initialized as soon as we start
	usleep(100000);
	if(playObject._base()->_isCompatibleWith("DecoderBaseObject"))
		if(!DynamicRequest(playObject).method("_set_blocking").param(true).invoke())
			cerr << "mpeglib, and blocking attribute can't be changed?" << endl;
	// </ignore>

	playToFile.start();
	while(playObject.state() != posIdle)
		playToFile.goOn();

	return 0;
}
