#include "synthschedule.h"
#include "datapacket.h"

class ASyncPort :public Port, public GenericDataChannel {
protected:
	long notifyID;
	std::vector<Notification> subscribers;

public:
	ASyncPort(std::string name, void *ptr, long flags, StdScheduleNode* parent);

	void processedPacket(GenericDataPacket *packet);
	void sendPacket(GenericDataPacket *packet);
	void connect(Port *port);
	void disconnect(Port *port);
	ASyncPort *asyncPort();
};
