	/*

	Copyright (C) 1998 Stefan Westerfeld
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

#include "utils.h"
#include <sys/time.h>
#include <config.h>

int get_byteorder(void)
{
#ifdef WORDS_BIGENDIAN
	return ORDER_BIGENDIAN;
#else
	return ORDER_LITTLEENDIAN;
#endif
}

float benchmark(int what)
{
	static struct timeval start,stop;

	if(what == BENCH_BEGIN)
	{
		gettimeofday(&start,NULL);
		return(0);
	}
	if(what == BENCH_END)
	{
		float diff;
		gettimeofday(&stop,NULL);
		diff = (stop.tv_sec-start.tv_sec);
		diff += (float)(stop.tv_usec-start.tv_usec)/1000000;
		return(diff);
	}

	return(0.0); /* should not happen? */
}
