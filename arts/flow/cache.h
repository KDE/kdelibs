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

#ifndef _SYNTH_CACHE_H
#define _SYNTH_CACHE_H

#include <time.h>
#include <list>
#include <string>
#include "startupmanager.h"

class Cache;

class CachedObject
{
private:
	string _object_key;
	int _ref_cnt;
	time_t _lastAccess;

protected:
	void setKey(string key);

public:
	string getKey();

	time_t lastAccess();

	void decRef();
	void incRef();
	int refCnt();

	virtual bool isValid();
	virtual int memoryUsage() = 0;

	CachedObject(Cache *cache);
	virtual ~CachedObject();
};

class Cache
{
protected:
	list<CachedObject *> objects;
	Cache();
	~Cache();

	static Cache *_instance;

public:
	static Cache *the();
	static void shutdown();

	static long memused;

	CachedObject *get(string key);
	void add(CachedObject *object);

	// garbage collection; returns amount of memory used (after cleaning)
	long cleanUp(long cacheLimit);
};

class CacheShutdown :public StartupClass
{
public:
	void startup();
	void shutdown();
};

#endif
