#include "kmedia2.h"
#include "stdsynthmodule.h"
#include <errno.h>

using namespace std;
using namespace Arts;

namespace Arts {

class StdoutWriter_impl : virtual public StdoutWriter_skel,
						  virtual public StdSynthModule
{
public:
	StdoutWriter_impl()
	{
	}
	void process_indata(DataPacket<mcopbyte> *data)
	{
		int result;
		errno = 0;
		do {
			result = write(1, data->contents, data->size);
		} while(errno == EINTR && result <= 0);
		data->processed();
	}
};

REGISTER_IMPLEMENTATION(StdoutWriter_impl);

};
