// $Id$

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#undef HAVE_SSL

#include <sys/types.h>
#include <sys/uio.h>

#include <netdb.h>
#include <unistd.h>

#ifdef HAVE_SSL
extern "C" {
	#include <openssl/ssl.h>
};
#endif

#include "kio/tcpslavebase.h"
#include "ksock.h"

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
#ifdef HAVE_SSL
	if (m_bIsSSL) {
		return SSL_write(ssl, static_cast<const char *>(data), len);
	} else
#endif
	{
		return ::write(m_iSock, data, len);
	}
}

ssize_t TCPSlaveBase::Read(void *data, ssize_t len)
{
#ifdef HAVE_SSL
	if (m_bIsSSL) {
		return SSL_read(ssl, static_cast<char *>(data), len);
	} else
#endif
	{
		return ::read(m_iSock, data, len);
	}
}

ssize_t TCPSlaveBase::ReadLine(char *data, ssize_t len)
{
	memset(data, 0, len);
#ifdef HAVE_SSL
	if (m_bIsSSL) {
		int c = 0, rc;
		char x;
		for (c = 0; c < len-1; c++) {
			rc = SSL_read(ssl, &x, 1);
			if (rc <= 0)
				return 0;
			data[c] = x;
			if (x == '\n') {
				data[c+1] = 0;
				break;
			}
		}
		if (c != len-1)
			c++;
		if (c <= 0)
			return 0;
		data[c]=0;
	} else
#endif
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
	ksockaddr_in server_name;

	memset(&server_name, 0, sizeof(server_name));

	port = GetPort(_port);

	m_iSock = ::socket(PF_INET, SOCK_STREAM, 0);
	if (m_iSock == -1) return false;

	if (!KSocket::initSockaddr(&server_name, host, port))
        {
		error( ERR_UNKNOWN_HOST, host);
		return false;
        }

	if (::connect(m_iSock, (struct sockaddr*)(&server_name), sizeof(server_name))) {
		error( ERR_COULD_NOT_CONNECT, host);
		return false;
	}

	// Since we want to use stdio on the socket,
	// we must fdopen it to get a file pointer,
	// if it fails, close everything up
	if ((fp = fdopen(m_iSock, "w+")) == 0) {
		CloseDescriptor();
		return false;
	}
#ifdef HAVE_SSL
	if (m_bIsSSL) {
		ssl = SSL_new(ssl_context);
		if (!ssl) {
			error( ERR_COULD_NOT_CONNECT, host);
			CloseDescriptor();
			return false;
		}
		SSL_set_fd(ssl, m_iSock);
		if (SSL_connect(ssl) == -1) {
			error( ERR_COULD_NOT_CONNECT, host);
			CloseDescriptor();
			SSL_shutdown(ssl);
			SSL_free(ssl);
      			return false;
		}
		server_cert = SSL_get_peer_certificate(ssl);
		if (!server_cert) {
			error( ERR_COULD_NOT_CONNECT, host );
			CloseDescriptor();
			SSL_shutdown(ssl);
			SSL_free(ssl);
			return false;
		}
		// we should verify the certificate here
		X509_free(server_cert);
	}
#endif
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
}

bool TCPSlaveBase::InitializeSSL()
{
#ifdef HAVE_SSL
  ssl = 0;
  SSLeay_add_ssl_algorithms();
  meth = SSLv23_client_method();
  SSL_load_error_strings();
  ssl_context = SSL_CTX_new(meth);
  return true;
#else
  return true;
#endif
}

void TCPSlaveBase::CleanSSL()
{
#ifdef HAVE_SSL
	SSL_shutdown(ssl);
	SSL_free(ssl);
	SSL_CTX_free(ssl_context);
	ssl=0;
#endif
}

bool TCPSlaveBase::AtEOF()
{
#ifdef HAVE_SSL
	if (m_bIsSSL) {
		return SSL_pending(ssl);
	}
#endif
	return feof(fp);
}
