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

#ifndef SOUNDSERVER_IMPL_H
#define SOUNDSERVER_IMPL_H

#include "soundserver.h"
#include "simplesoundserver_impl.h"
#include "soundserver.h"

namespace Arts {

	class SoundServer_impl : virtual public SoundServer_skel,
							 public SimpleSoundServer_impl
	{
		RealtimeStatus realtimeStatus();
		long secondsUntilSuspend();
		bool suspend();
		bool terminate();
	};

};

#endif /* VERBOSESOUNDSERVER_IMPL_H */
