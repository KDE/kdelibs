// $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/uio.h>

#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#include "kio/tcpslavebase.h"
#include "kextsock.h"
#include <ksocks.h>

using namespace KIO;

TCPSlaveBase::TCPSlaveBase(unsigned short int default_port, const QCString &protocol, const QCString &pool_socket, const QCString &app_socket)
	: SlaveBase (protocol, pool_socket, app_socket), m_iDefaultPort(default_port), m_iSock(-1), m_sServiceName(protocol), fp(0)
{
	InitializeSSL();
}

TCPSlaveBase::~TCPSlaveBase()
{
	CleanSSL();
}

ssize_t TCPSlaveBase::Write(const void *data, ssize_t len)
{
	return KSocks::self()->write(m_iSock, data, len);
}

ssize_t TCPSlaveBase::Read(void *data, ssize_t len)
{
	return KSocks::self()->read(m_iSock, data, len);
}

ssize_t TCPSlaveBase::ReadLine(char *data, ssize_t len)
{
	memset(data, 0, len);
	fgets(data, len, fp);
	return strlen(data);
}

unsigned short int TCPSlaveBase::GetPort(unsigned short int _port)
{
	unsigned short int port;
	if (_port <= 0) {
		struct servent *srv=getservbyname(m_sServiceName, "tcp");
		if (srv) {
			port=ntohs(srv->s_port);
		} else
			port=m_iDefaultPort;
	} else
		port=_port;
	return port;
}

  // This function is simply a wrapper to establish the connection
  // to the server.  It's a bit more complicated than ::connect
  // because we first have to check to see if the user specified
  // a port, and if so use it, otherwise we check to see if there
  // is a port specified in /etc/services, and if so use that
  // otherwise as a last resort use the supplied default port.
bool TCPSlaveBase::ConnectToHost(const QCString &host, unsigned short int _port)
{
	unsigned short int port;
	KExtendedSocket ks;

	port = GetPort(_port);

#if 0
	memset(&server_name, 0, sizeof(server_name));

	m_iSock = ::socket(PF_INET, SOCK_STREAM, 0);
	if (m_iSock == -1) return false;

	if (!KSocket::initSockaddr(&server_name, host, port))
        {
		error( ERR_UNKNOWN_HOST, host);
		CloseDescriptor();
		return false;
        }

	if (::connect(m_iSock, (struct sockaddr*)(&server_name), sizeof(server_name))) {
		error( ERR_COULD_NOT_CONNECT, host);
		CloseDescriptor();
		return false;
	}
#else

	ks.setAddress(host, port);
	if (ks.connect() < 0)
	  {
	    if (ks.status() == IO_LookupError)
	      error( ERR_UNKNOWN_HOST, host);
	    else
	      error( ERR_COULD_NOT_CONNECT, host);
	    return false;
	  }
	m_iSock = ks.fd();
	ks.release();		// KExtendedSocket no longer applicable
#endif
	

	// Since we want to use stdio on the socket,
	// we must fdopen it to get a file pointer,
	// if it fails, close everything up
	if ((fp = fdopen(m_iSock, "w+")) == 0) {
		CloseDescriptor();
		return false;
	}
	m_iPort=port;
	return true;
}

void TCPSlaveBase::CloseDescriptor()
{
	if (fp) {
		fclose(fp);
		fp=0;
		m_iSock=-1;
		CleanSSL();
	}
	if (m_iSock != -1) {
		close(m_iSock);
		m_iSock=-1;
	}
}

bool TCPSlaveBase::InitializeSSL()
{
return false;
}

void TCPSlaveBase::CleanSSL()
{
}

bool TCPSlaveBase::AtEOF()
{
	return feof(fp);
}
