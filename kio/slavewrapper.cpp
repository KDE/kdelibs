#include "kio/slavewrapper.h"
#include "kio/slavebase.h"
#include <ksock.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <kdebug.h>

using namespace KIO;

SlaveWrapper::SlaveWrapper(SlaveBase *s, int socket)
    : slave(s)
{
    parconn.init(socket, socket);
}

SlaveWrapper::~SlaveWrapper()
{
    delete slave;
}

void SlaveWrapper::dispatchLoop()
{
    fd_set rfds;
    struct timeval tv;
    int retval;

    while (true) {
	FD_ZERO(&rfds);

	if (parconn.inited())
	  FD_SET(parconn.fd_from(), &rfds);
	if (appconn.inited())
	  FD_SET(appconn.fd_from(), &rfds);

	/* Wait up to 30 seconds. */
	tv.tv_sec = 30;
	tv.tv_usec = 0;

	retval = select(QMAX(parconn.fd_from(), appconn.fd_from()) + 1, &rfds, NULL, NULL, &tv);
	/* Don't rely on the value of tv now! */

	if (retval > 0) {
	    if (FD_ISSET(parconn.fd_from(), &rfds)) { // dispatch master messages
		int cmd;
		QByteArray data;
                if (parconn.read(&cmd, data) == -1)
                {
                   kDebugInfo(7019, "Connection with master failed.\n");
                   exit(0);
                }
		kDebugInfo(7019, "master said %c", cmd);

		QDataStream stream(data, IO_ReadOnly);
		QString str;

		switch (cmd) {
		case 'C':
		    stream >> str;
		    connectSlave(str);
		    break;
		}
	    }
	    if (FD_ISSET(appconn.fd_from(), &rfds)) { // dispatch application messages
		int cmd;
		QByteArray data;
                if ( appconn.read(&cmd, data) != -1 )
                {
		  kDebugInfo(7019, "app said %c", cmd);
		  slave->dispatch(cmd, data);
                } else // some error occured, perhaps no more application
                {
// When the app exits, should the slave be put back in the pool ?
                  kDebugInfo(7019, "slavewrapper: some error occured, perhaps no more application");
                  exit(0);
                }
	    }
	} else if (retval == -1) // error
        {
          kDebugInfo(7019, "slavewrapper: select returned error %s (%d)", errno==EBADF?"EBADF":errno==EINTR?"EINTR":errno==EINVAL?"EINVAL":errno==ENOMEM?"ENOMEM":"unknown",errno);
          exit(0);
        }
    }
}

void SlaveWrapper::connectSlave(const QString& path)
{
    appconn.init(new KSocket(path));
    slave->setConnection(&appconn);
}

