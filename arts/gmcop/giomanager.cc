    /*

    Copyright (C) 2001 Stefan Westerfeld
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


#include "giomanager.h"
#include "notification.h"
#include "debug.h"
#include <glib.h>

using namespace Arts;
using namespace std;

namespace Arts {
	struct GIOManagerSource
	{
		GSource source;
		GIOManager *ioManager;
	};
	struct GIOWatch
	{
		GPollFD gpollfd;	 /* <- must be the first data member */

	/* GPollFD =
  		gint          fd;
  		gushort       events;
  		gushort       revents;
  	*/
		GIOWatch(GIOManagerSource *source, int fd, int types, IONotify *notify)
			: types(types), registered(false), notify(notify), source(source)
		{
			gpollfd.fd = fd;
			setTypes(types);
		}
		void setTypes(int types)
		{
			this->types = types;
			gpollfd.events = gpollfd.revents = 0;

			if(types & IOType::read)
				gpollfd.events |= G_IO_IN | G_IO_HUP;
			if(types & IOType::write)
				gpollfd.events |= G_IO_OUT;
			if(types & IOType::except)
				gpollfd.events |= G_IO_ERR;

		}
		/* prepares source for running in event loop level "level"
		 * removes source unconditionally if level == -1
		 */
		void prepare(int level)
		{
			gpollfd.revents = 0;

			bool shouldRegister = true;
			if(level != 1 && (types & IOType::reentrant) == 0)
				shouldRegister = false;
			if(level == -1)
				shouldRegister = false;

			if(shouldRegister == registered)
				return;

			if(shouldRegister)
			{
				g_source_add_poll(&source->source, &this->gpollfd);
			}
			else
			{
				g_source_remove_poll(&source->source, &this->gpollfd);
			}

			registered = shouldRegister;
		}
		int check()
		{
			int result = 0;

			if(gpollfd.revents & (G_IO_IN | G_IO_HUP))
				result |= IOType::read;
			if(gpollfd.revents & G_IO_OUT)
				result |= IOType::write;
			if(gpollfd.revents & G_IO_ERR)
				result |= IOType::except;

			return result;
		}
		void destroy()
		{
			/* TODO: if active do this, else do that */
			delete this;
		}
		~GIOWatch()
		{
			prepare(-1);
		}

		int types;
		bool registered;
		IONotify *notify;
		GIOManagerSource *source;
	};

class GIOTimeWatch {
	int milliseconds;
	TimeNotify *_notify;
	timeval nextNotify;
	bool active, destroyed;
public:
	GIOTimeWatch(int milliseconds, TimeNotify *notify)
		: milliseconds(milliseconds), _notify(notify),
		  active(false),destroyed(false)
	{
		gettimeofday(&nextNotify,0);

		nextNotify.tv_usec += (milliseconds%1000)*1000;
		nextNotify.tv_sec += (milliseconds/1000)+(nextNotify.tv_usec/1000000);
		nextNotify.tv_usec %= 1000000;
	}
	int msUntilNextNotify(const timeval& now)
	{
		int result = (nextNotify.tv_sec - now.tv_sec)*1000
				   + (nextNotify.tv_usec - now.tv_usec)/1000;

		if(result < 0) result = 0;
		return result;
	}
	void advance(const timeval& currentTime)
	{
		active = true;
		while(msUntilNextNotify(currentTime) == 0)
		{
			nextNotify.tv_usec += (milliseconds%1000)*1000;
			nextNotify.tv_sec += (milliseconds/1000)
			                  +(nextNotify.tv_usec/1000000);
			nextNotify.tv_usec %= 1000000;

			_notify->notifyTime();

			if(destroyed)
			{
				delete this;
				return;
			}
		}
		active = false;
	}
	void destroy()
	{
		if(active)
		{
			destroyed = true;
		}
		else
		{
			delete this;
		}
	}
	TimeNotify *notify()
	{
		return _notify;
	}
};

static gboolean GIOManager_prepare(GSource *source, gint *timeout)
{
	return((GIOManagerSource *)source)->ioManager->prepare(timeout);
}

static gboolean GIOManager_check(GSource *source)
{
	return((GIOManagerSource *)source)->ioManager->check();
}

static gboolean GIOManager_dispatch(GSource *source, GSourceFunc callback,
											gpointer user_data)
{
	return((GIOManagerSource *)source)->ioManager->dispatch(callback,user_data);
}


};

GIOManager::GIOManager(GMainContext *context)
	: level(0), context(context)
{
	static GSourceFuncs funcs =
	{
		GIOManager_prepare,
		GIOManager_check,
		GIOManager_dispatch,
		0
	};

	source = (GIOManagerSource *)g_source_new(&funcs, sizeof(GIOManagerSource));
	source->ioManager = this;
	g_source_set_can_recurse(&source->source, true);
	g_source_attach(&source->source, context);
}

GIOManager::~GIOManager()
{
	g_source_unref(&source->source);
}

void GIOManager::processOneEvent(bool blocking)
{
	g_main_context_iteration(context, blocking);
}

void GIOManager::run()
{
	arts_warning("GIOManager::run not implemented yet");
}

void GIOManager::terminate()
{
	arts_warning("GIOManager::terminate not implemented yet");
}

void GIOManager::watchFD(int fd, int types, IONotify * notify)
{
	fdList.push_back(new GIOWatch(source, fd, types, notify));
}

void GIOManager::remove(IONotify *notify, int types)
{
	list<GIOWatch *>::iterator i;

	i = fdList.begin();
	while(i != fdList.end())
	{
		GIOWatch *w = *i;

		if(w->notify == notify)
		{
			int newTypes = w->types & (~types);

			if(newTypes)
			{
				w->setTypes(newTypes);
			}
			else
			{
				w->destroy();
				fdList.erase(i);
				i = fdList.begin();
			}
		}
		else i++;
	}
}

void GIOManager::addTimer(int milliseconds, TimeNotify *notify)
{
	timeList.push_back(new GIOTimeWatch(milliseconds,notify));
}

void GIOManager::removeTimer(TimeNotify *notify)
{
	list<GIOTimeWatch *>::iterator i;

	i = timeList.begin();
	while(i != timeList.end())
	{
		GIOTimeWatch *w = *i;

		if(w->notify() == notify)
		{
			w->destroy();
			timeList.erase(i);
			i = timeList.begin();
		}
		else i++;
	}
}

gboolean GIOManager::prepare(gint *timeout)
{
	*timeout = 10000;

	level++;

	/* handle timers - only at level 1 */
	if(level == 1 && timeList.size())
	{
		struct timeval currenttime;
		gettimeofday(&currenttime,0);

		list<GIOTimeWatch *>::iterator ti;

		ti = timeList.begin();
		while(ti != timeList.end())
		{
			GIOTimeWatch *w = *ti++;
			int ms = w->msUntilNextNotify(currenttime);

			if(ms < *timeout) *timeout = ms;
		}
	}

	list<GIOWatch *>::iterator i;
	for(i = fdList.begin(); i != fdList.end(); i++)
	{
		GIOWatch *w = *i;
		w->prepare(level);
	}

	if(level == 1 && NotificationManager::the()->pending())
		*timeout = 0;

	level--;

	return (*timeout == 0);
}

gboolean GIOManager::check()
{
	gboolean result = false;
	level++;

	/*
	 * handle timers - only at level 1
	 */
	if(level == 1 && timeList.size())
	{
		struct timeval currenttime;
		gettimeofday(&currenttime,0);

		list<GIOTimeWatch *>::iterator ti;

		ti = timeList.begin();
		while(ti != timeList.end() && !result)
		{
			GIOTimeWatch *w = *ti++;
			if(w->msUntilNextNotify(currenttime) == 0)
				result = true;
		}
	}

	/*
	 * handle filedescriptors
	 */
	list<GIOWatch *>::iterator i;
	for(i = fdList.begin(); i != fdList.end(); i++) {
		GIOWatch *w = *i;
		int match = w->check();

		if((w->types & IOType::reentrant) == 0 && level != 1)
		{
			arts_assert(match == 0);
		}
		if(match)
		{
			result = true;
		}
	}

	/*
	 * check for notifications
	 */
	if(level == 1 && NotificationManager::the()->pending())
		result = true;

	level--;

	return result;
}

gboolean GIOManager::dispatch(GSourceFunc /* func */, gpointer /* user_data */)
{
	level++;

	// notifications not carried out reentrant
	if(level == 1)
		NotificationManager::the()->run();
		
	/* handle filedescriptor things */

	/*
	 * the problem is, that objects that are being notified may change
	 * the watch list, add fds, remove fds, remove objects and whatever
	 * else
	 *
	 * so we can' notify them from the loop - but we can make a stack
	 * of "notifications to do" and send them as soon as we looked up
	 * in the list what to send
	 */
	long tonotify = 0;

	list<GIOWatch *>::iterator i;
	for(i = fdList.begin(); i != fdList.end(); i++) {
		GIOWatch *w = *i;
		int match = w->check();

		if((w->types & IOType::reentrant) == 0 && level != 1)
		{
			arts_assert(match == 0);
		}
		if(match)
		{
			tonotify++;
			notifyStack.push(make_pair(w,match));
		}
	}
	
	while(tonotify != 0)	/* FIXME: what happens if an iowatch gets deleted */
	{
		GIOWatch *w = notifyStack.top().first;
		int match = notifyStack.top().second;

		w->notify->notifyIO(w->gpollfd.fd,match);

		notifyStack.pop();
		tonotify--;
	}

	/* handle timers - only at level 1 */
	if(level == 1 && timeList.size())
	{
		struct timeval currenttime;
		gettimeofday(&currenttime,0);

		list<GIOTimeWatch *>::iterator ti;

		ti = timeList.begin();
		while(ti != timeList.end())
		{
			GIOTimeWatch *w = *ti++;
			w->advance(currenttime);
		}
	}

	// notifications not carried out reentrant
	if(level == 1)
		NotificationManager::the()->run();
	level--;

	return true;
}
