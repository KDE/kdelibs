#include "synthschedule.h"
#include "datapacket.h"

class ASyncPort :public Port, public GenericDataChannel {
protected:
	long notifyID;
	std::vector<Notification> subscribers;

	GenericAsyncStream *stream;

	bool pull;
	Notification pullNotification;

public:
	// GenericDataChannel interface
	void processedPacket(GenericDataPacket *packet);
	void sendPacket(GenericDataPacket *packet);
	void setPull(int packets, int capacity);
	void endPull();

	// Port interface
	ASyncPort(std::string name, void *ptr, long flags, StdScheduleNode* parent);

	void connect(Port *port);
	void disconnect(Port *port);
	ASyncPort *asyncPort();
};
