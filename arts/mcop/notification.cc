    /*

    Copyright (C) 2000-2001 Stefan Westerfeld
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

#include "notification.h"
#include "debug.h"

using namespace Arts;

void NotificationClient::notify(const Notification&)
{
}

NotificationManager::NotificationManager()
{
	arts_assert(!instance);
	instance = this;
}

NotificationManager::~NotificationManager()
{
	arts_assert(instance);
	instance = 0;
}

bool NotificationManager::run()
{
	if(todo.empty()) return false;

	while(!todo.empty())
	{
		Notification wm = todo.front();
		todo.pop();

		/*
		 * we'll copy and remove the notification first, to be sure that
		 * nothing bad happens to it if we rebuild "todo" (for instance in
		 * removeClient) 
		 */
		wm.receiver->notify(wm);
	}
	return true;
}

void NotificationManager::removeClient(NotificationClient *client)
{
	std::queue<Notification> newTodo;

	while(!todo.empty())
	{
		const Notification& n = todo.front();
		if(n.receiver != client)
			newTodo.push(n);
		else
		{
			arts_debug("NotificationManager: removing one notification");
			NotificationDestroyFunction destroy = 
				(NotificationDestroyFunction)n.internal;
			if(destroy) destroy(n);
		}
		todo.pop();
	}
	todo = newTodo;
}

void Notification::setDestroy(NotificationDestroyFunction destroy)
{
	internal = (void*)destroy;
}

NotificationManager *NotificationManager::instance = 0;
