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

#include <kssl.h>


class TCPSlaveBase::TcpSlaveBasePrivate {
public:
  KSSL *kssl;
  bool usingTLS;
};


TCPSlaveBase::TCPSlaveBase(unsigned short int default_port, const QCString &protocol, const QCString &pool_socket, const QCString &app_socket)
	: SlaveBase (protocol, pool_socket, app_socket), m_iDefaultPort(default_port), m_iSock(-1), m_sServiceName(protocol), fp(0)
{
// We have to have two constructors, so don't add anything else in here.
// put it in doConstructorStuff() instead.
        doConstructorStuff();
        m_bIsSSL = false;
}

TCPSlaveBase::TCPSlaveBase(unsigned short int default_port, const QCString &protocol, const QCString &pool_socket, const QCString &app_socket, bool useSSL)
	: SlaveBase (protocol, pool_socket, app_socket), m_bIsSSL(useSSL), m_iDefaultPort(default_port), m_iSock(-1), m_sServiceName(protocol), fp(0)
{
        doConstructorStuff();
        if (useSSL)
           m_bIsSSL = InitializeSSL();
}

// The constructor procedures go here now.
void TCPSlaveBase::doConstructorStuff()
{
        d = new TcpSlaveBasePrivate;
        d->usingTLS = false;
}

TCPSlaveBase::~TCPSlaveBase()
{
	CleanSSL();
        if (d->usingTLS) delete d->kssl;
        delete d;
}

ssize_t TCPSlaveBase::Write(const void *data, ssize_t len)
{
        if (m_bIsSSL || d->usingTLS)
           return d->kssl->write(data, len);
	return KSocks::self()->write(m_iSock, data, len);
}

ssize_t TCPSlaveBase::Read(void *data, ssize_t len)
{
        if (m_bIsSSL || d->usingTLS)
           return d->kssl->read(data, len);
	return KSocks::self()->read(m_iSock, data, len);
}

ssize_t TCPSlaveBase::ReadLine(char *data, ssize_t len)
{
        // let's not segfault!
        if (!data) return -1;

        if (m_bIsSSL || d->usingTLS) {
           // ugliness alert!!  calling read() so many times can't be good...
           int clen=0;
           char *buf=data;
           while (clen < len) {
              int rc = d->kssl->read(buf, 1);
              if (rc < 0) return -1;
              clen++;
              if (*buf++ == '\n')
                 break;
           }
           *buf=0; 
           return clen;
        }

        // This should change.  It's O(2n) and can be made O(n).
        *data = 0;
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
        if (m_bIsSSL) {
           d->kssl->reInitialize();
           int rc = d->kssl->connect(m_iSock);
           if (rc < 0) { 
              CloseDescriptor();
              return false;
           }
        }

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
        stopTLS();
	if (fp) {
		fclose(fp);
		fp=0;
		m_iSock=-1;
                if (m_bIsSSL)
                   d->kssl->close();
	}
	if (m_iSock != -1) {
		close(m_iSock);
		m_iSock=-1;
	}
}

bool TCPSlaveBase::InitializeSSL()
{
   if (m_bIsSSL) {
      if (KSSL::doesSSLWork()) {
         d->kssl = new KSSL;
         return true;
      } else return false;
   } else return false;
}

void TCPSlaveBase::CleanSSL()
{
   if (m_bIsSSL) {
      delete d->kssl;
   }
}

bool TCPSlaveBase::AtEOF()
{
	return feof(fp);
}

bool TCPSlaveBase::startTLS()
{
        if (d->usingTLS || m_bIsSSL || !KSSL::doesSSLWork()) return false;

        d->kssl = new KSSL(false);
        if (!d->kssl->TLSInit()) {
           delete d->kssl;
           return false;
        }

        int rc = d->kssl->connect(m_iSock);
        if (rc < 0) {
           delete d->kssl;
           return false;
        }

        d->usingTLS = true;
return d->usingTLS;
}


void TCPSlaveBase::stopTLS()
{
        if (d->usingTLS) {
           delete d->kssl;
           d->usingTLS = false;
        }
}


bool TCPSlaveBase::canUseTLS()
{
KSSLSettings kss;
        if (m_bIsSSL || !KSSL::doesSSLWork()) return false;
        return kss.tlsv1();
}


bool TCPSlaveBase::usingTLS()
{
        return d->usingTLS;
}


