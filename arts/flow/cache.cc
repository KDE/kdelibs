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

#include "cache.h"
#include "debug.h"
#include "iomanager.h"
#include "dispatcher.h"
#include <iostream>
#include <assert.h>

using namespace std;
using namespace Arts;

bool CachedObject::isValid()
{
	return true;
}

void CachedObject::setKey(string key)
{
	_object_key = key;
}

string CachedObject::getKey()
{
	return _object_key;
}

void CachedObject::decRef()
{
	_ref_cnt--;
	time(&_lastAccess);
}

void CachedObject::incRef()
{
	_ref_cnt++;
}

int CachedObject::refCnt()
{
	return _ref_cnt;
}

time_t CachedObject::lastAccess()
{
	return(_lastAccess);
}

CachedObject::CachedObject(Cache *cache)
{
	_ref_cnt = 1;
	cache->add(this);
}

CachedObject::~CachedObject()
{
	assert(_ref_cnt == 0);
}

//------------------------- Cache implementation ---------------------------
long Cache::memused = 0;

CachedObject *Cache::get(string key)
{
	list<CachedObject *>::iterator i;

	for(i=objects.begin();i != objects.end(); i++)
	{
		if((*i)->getKey() == key && (*i)->isValid())
		{
			(*i)->incRef();
			return(*i);
		}
	}
	return 0;
}

void Cache::add(CachedObject *object)
{
	objects.push_back(object);
}

long Cache::cleanUp(long cacheLimit)
{
	time_t lastAccess;

	list<CachedObject *>::iterator i;
	long memory = 0;

	// delete all invalid unused entries (invalid entries that are still
	// in use, e.g. cached wav files which have changed on disk but are
	// still played can't be deleted!)
	
	for(i=objects.begin();i != objects.end(); i++)
	{
		CachedObject *co = (*i);

		if(co->refCnt() == 0 && !co->isValid())
		{
			objects.remove(co);
			delete co;
			i = objects.begin();
		}
	}

	for(i=objects.begin();i != objects.end(); i++)
	{
		memory += (*i)->memoryUsage();
	}

	bool freeok = true;
	while(memory > cacheLimit && freeok)
	{
		CachedObject *freeme;

		freeok = false;

		// only start freeing objects which have not been accessed
		// in the last 5 seconds

		time(&lastAccess);
		lastAccess -= 5;


		for(i=objects.begin();!freeok && (i != objects.end()); i++)
		{
			CachedObject *co = (*i);

			assert(co->refCnt() >= 0);
			if(co->refCnt() == 0 && (co->lastAccess() < lastAccess))
			{
				lastAccess = co->lastAccess();
				freeme = co;
				freeok = true;
			}
			else
			{
				//artsdebug("%d => %ld\n",co->refCnt(),co->lastAccess());
			}
		}

		if(freeok)
		{
			memory -= freeme->memoryUsage();
			objects.remove(freeme);
			delete(freeme);
		}
		else
		{
			//artsdebug("cache problem: memory overused, but nothing there to free\n");
		}
	}

	memused = memory/1024;
	return(memory);
}

Cache *Cache::_instance = 0;

Cache::Cache()
{
	assert(!_instance);
	_instance = this;
}

Cache::~Cache()
{
	list<CachedObject *>::iterator i;
	for(i=objects.begin(); i != objects.end(); i++)
		delete (*i);
	objects.clear();

	assert(_instance);
	_instance = 0;
}


Cache *Cache::the()
{
	if(!_instance) _instance = new Cache();
	return _instance;
}

void Cache::shutdown()
{
	if(_instance)
	{
		list<CachedObject *>::iterator i;
		long rcnt = 0;
		for(i=_instance->objects.begin(); i != _instance->objects.end(); i++)
			rcnt += (*i)->refCnt();

		if(rcnt == 0)
		{
			delete _instance;
			_instance = 0;
		}
		else
		{
			arts_warning("cache shutdown while still active objects in cache");
		}
	}
}

namespace Arts {			// internal helpers

// periodic cache clean
class CacheClean : public TimeNotify {
public:
	CacheClean();
	void notifyTime();
	virtual ~CacheClean();
};

// cache startup & shutdown
class CacheStartup :public StartupClass
{
public:
	void startup();
	void shutdown();
private:
	CacheClean *cacheClean;
};

};

CacheClean::CacheClean()
{
	Dispatcher::the()->ioManager()->addTimer(5000, this);
}

void CacheClean::notifyTime()
{
	// TODO: make this configurable
	Cache::the()->cleanUp(8192*1024);
}

CacheClean::~CacheClean()
{
	Dispatcher::the()->ioManager()->removeTimer(this);
}

void CacheStartup::startup()
{
	cacheClean = new CacheClean;	
}

void CacheStartup::shutdown()
{
	delete cacheClean;
	Cache::shutdown();
}

static CacheStartup cacheStartup;
