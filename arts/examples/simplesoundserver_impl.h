    /*

    Copyright (C) 1999-2000 Stefan Westerfeld
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

    */

#ifndef SIMPLESOUNDSERVER_IMPL_H
#define SIMPLESOUNDSERVER_IMPL_H

#include "soundserver.h"
#include "artsflow.h"
#include <list>

class SimpleSoundServer_impl : virtual public SimpleSoundServer_skel,
										public TimeNotify
{
protected:
	Object_skel *play_obj, *add_left, *add_right;
	list<Synth_PLAY_WAV *> activeWavs;

public:
	SimpleSoundServer_impl();
	~SimpleSoundServer_impl();

	long play(const string& s);
	void notifyTime();
};

#endif /* SIMPLESOUNDSERVER_IMPL_H */
