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

#ifndef CACHEDWAV_H
#define CACHEDWAV_H

#include "config.h"
#ifdef HAVE_LIBAUDIOFILE
#include "cache.h"
#include <sys/stat.h>
#include <unistd.h>

/*
 * BC - Status (2000-09-30): CachedWav.
 *
 * At the current point in time, there are NO GUARANTEES, so only use this
 * in apps part of official KDE releases (such as kdemultimedia apps), which
 * know what is happening here. Especially, in SOME kde versions this class
 * might not even exist, as it gets conditionally compiled.
 */

namespace Arts {

class CachedWav : public CachedObject
{
protected:
	struct stat oldstat;
	std::string filename;
	bool initOk;

	CachedWav(Cache *cache, std::string filename);
	~CachedWav();

	typedef unsigned char uchar;
public:
	double samplingRate;
	long bufferSize;
	int channelCount;
	int sampleWidth;
	unsigned char *buffer;

	static CachedWav *load(Cache *cache, std::string filename);
	/**
	 * validity test for the cache - returns false if the object is having
	 * reflecting the correct contents anymore (e.g. if the file on the
	 * disk has changed), and there is no point in keeping it in the cache any
	 * longer
	 */
	bool isValid();
	/**
	 * memory usage for the cache
	 */
	int memoryUsage();
};

};

#endif /* HAVE_LIBAUDIOFILE */
#endif /* CACHEDWAV_H */
