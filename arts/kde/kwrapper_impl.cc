	/*

	Copyright (C) 2001 Nikolas Zimmermann <wildfox@kde.org>
	                   Stefan Westerfeld <stefan@space.twc.de>

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

#include "debug.h"
#include "artsflow.h"
#include "flowsystem.h"
#include "connect.h"
#include "kwrapper_impl.h"

using namespace Arts;
using namespace std;

KWrapperFactory_impl::KWrapperFactory_impl()
{
}

KWrapperFactory_impl::~KWrapperFactory_impl()
{
}

PlayObject KWrapperFactory_impl::createPlayObject(const string &url, const string &mimetype, bool createBUS)
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
				Arts::connect(result,"left",uplink,"left");
				Arts::connect(result,"right",uplink,"right");
				uplink.start();
				result._node()->start();
				result._addChild(uplink,"uplink");
				return result;
			}
			else return result;
		}
		else arts_warning("couldn't load file %s", url.c_str());
	}
	else arts_warning("mimetype %s unsupported", mimetype.c_str());

	return PlayObject::null();
}
