#include "simplesoundserver_impl.h"
#include "mcoputils.h"
#include <signal.h>

void stopServer(int)
{
	Dispatcher::the()->terminate();
}

void initSignals()
{
    signal(SIGPIPE,stopServer);
    signal(SIGHUP ,stopServer);
    signal(SIGQUIT,stopServer);
    signal(SIGINT ,stopServer);
    signal(SIGTERM,stopServer);                                                 
}

int main()
{
	initSignals();

	Dispatcher dispatcher;
	SimpleSoundServer_impl server;

	bool result = ObjectManager::the()
				->addGlobalReference(&server,"Arts_SimpleSoundServer");

	if(!result)
	{
		cerr <<
"Error: Can't add object reference (perhaps it is already running?)." << endl <<
"       If you are sure it is not already running, remove the relevant file:"
              << endl << endl <<
"       "<< MCOPUtils::createFilePath("Arts_SimpleSoundServer") << endl << endl;
		return 1;
	}
	dispatcher.run();

	return 0;
}
