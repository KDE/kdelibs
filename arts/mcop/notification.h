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

namespace Arts {

/*
 * BC - Status (2000-09-30): Notification, NotificationClient,
 *   NotificationManager
 *
 * All need to be kept BC, NotificationManager with usual d ptr.
 *
 * Notifications have to be FAST. Currently, they use ID, data & receiver.
 * However, there may be future extensions. For this, one more internal
 * field is provided. Set it to ZERO currently when sending notifications.
 */

struct Notification;
typedef void (*NotificationDestroyFunction)(const Notification& n);

struct Notification {
	class NotificationClient *receiver;
	int ID;
	void *data;
	void *internal;		/* handle with care, equivalent to private d ptr */

	void setDestroy(NotificationDestroyFunction destroy);
};

class NotificationClient {
public:
	virtual void notify(const Notification& wm);
};

class NotificationManagerPrivate;
class NotificationManager {
private:
	NotificationManagerPrivate *d;		// unused
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
	inline bool pending()
	{
		return !todo.empty();
	}
	bool run();
	void removeClient(NotificationClient *client);

};

};

#endif /* NOTIFICATION_H */
