#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "core.h"
#include "mcoputils.h"

using namespace std;

class TmpGlobalComm_impl : virtual public TmpGlobalComm_skel {
public:
	bool put(const string& variable, const string& value)
	{
		string filename = MCOPUtils::createFilePath(variable);

		int fd = open(filename.c_str(),O_CREAT|O_EXCL|O_WRONLY,S_IRUSR|S_IWUSR);
		if(fd != -1)
		{
			int result = write(fd,value.c_str(),value.size());

			assert(result == (int)value.size());
			close(fd);
			return true;
		}

		return false;
	}

	string get(const string& variable)
	{
		string result = "";
		string filename = MCOPUtils::createFilePath(variable);

		int fd = open(filename.c_str(),O_RDONLY);
		if(fd != -1)
		{
			char buffer[8192];
			int size = read(fd,buffer,8192);
			if(size > 0 && size < 8192) {
				buffer[size] = 0;
				result = buffer;
			}

			close(fd);
		}
		return result;
	}

	void erase(const string& variable)
	{
		string filename = MCOPUtils::createFilePath(variable);
		unlink(filename.c_str());
	}
};

REGISTER_IMPLEMENTATION(TmpGlobalComm_impl);
