#include <stdio.h>
#include <string>
#include <unistd.h>
#include "dispatcher.h"
#include "thread.h"

using namespace Arts;
using namespace std;

class Counter : public Arts::Thread 
{
	string name;
public:
	Counter(const string& name) :name(name) {}

	void run() {
		for(int i = 0;i < 10;i++)
		{
			printf("[%s] %d\n",name.c_str(),i+1);
			sleep(1);
		}
		printf("[%s] terminating.\n", static_cast<Counter *>
			(SystemThreads::the()->getCurrentThread())->name.c_str());
	}
};

int main()
{
	Dispatcher dispatcher;
	Counter c1("counter1"), c2("counter2");

	if(SystemThreads::supported())
		printf("We have a system threads (counters should count parallel).\n");
	else
		printf("No system threads (counters will not count parallel).\n");

	c1.start();
	c2.start();
	c1.waitDone();
	c2.waitDone();
	return 0;
}
