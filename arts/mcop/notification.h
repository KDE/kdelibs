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
