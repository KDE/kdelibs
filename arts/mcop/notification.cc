#include "notification.h"
#include <assert.h>

void NotificationClient::notify(const Notification&)
{
}

NotificationManager::NotificationManager()
{
	assert(!instance);
	instance = this;
}

NotificationManager::~NotificationManager()
{
	assert(instance);
	instance = 0;
}

NotificationManager *NotificationManager::instance = 0;
