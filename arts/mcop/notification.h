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

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <queue>

struct Notification {
	class NotificationClient *receiver;
	int ID;
	void *data;
};

class NotificationClient {
public:
	virtual void notify(const Notification& wm);
};

class NotificationManager {
protected:
	std::queue<Notification> todo;
	static NotificationManager *instance;

public:
	NotificationManager();
	~NotificationManager();

	static inline NotificationManager *the()
	{
		return instance;
	}
	inline void send(Notification wm)
	{
		todo.push(wm);
	}
	inline bool run()
	{
		if(todo.empty()) return false;

		while(!todo.empty())
		{
			Notification& wm = todo.front();
			todo.pop();
			wm.receiver->notify(wm);
		}
		return true;
	}
};

#endif /* NOTIFICATION_H */
