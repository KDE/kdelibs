/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <config.h>
#include "kmwlpd.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <kurl.h>
#include <klocale.h>
#include <qlabel.h>
#include <kdebug.h>
#include <qlineedit.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>

static int openConnection(const char *host);
static bool checkLpdQueue(const char *host, const char *queue);

//********************************************************************************************************

KMWLpd::KMWLpd(QWidget *parent, const char *name)
: KMWInfoBase(2,parent,name)
{
	m_ID = KMWizard::LPD;
	m_title = i18n("LPD queue information");
	m_nextpage = KMWizard::Driver;

	setInfo(i18n("<p>Enter the information concerning the remote LPD queue. "
		     "This wizard will check them before continuing.</p>"));
	setLabel(0,i18n("Host:"));
	setLabel(1,i18n("Queue:"));
}

bool KMWLpd::isValid(QString& msg)
{
	if (text(0).isEmpty() || text(1).isEmpty())
	{
		msg = i18n("Some information is missing !");
		return false;
	}

	// check LPD queue
	if (!checkLpdQueue(text(0).latin1(),text(1).latin1()))
	{
		msg = i18n("<nobr>Can't find queue <b>%1</b> on <b>%2</b> !</nobr>").arg(text(1)).arg(text(0));
		return false;
	}
	return true;
}

void KMWLpd::updatePrinter(KMPrinter *p)
{
	QString	dev = QString::fromLatin1("lpd://%1/%2").arg(text(0)).arg(text(1));
	p->setDevice(KURL(dev));
}

//*******************************************************************************************************

int openConnection(const char *rhost)
{
	int	sock;
	struct hostent	*host;
	struct servent	*serv;
	struct sockaddr_in	sin;

	if (!rhost) return -1;
	host = gethostbyname(rhost);
	if (!host) return -1;
	serv = getservbyname("printer","tcp");
	if (!serv) return -1;
	memset(&sin,0,sizeof(sin));
	if (host->h_length > (int)sizeof(sin.sin_addr))
		host->h_length = sizeof(sin.sin_addr);
	memcpy((caddr_t)&sin.sin_addr,host->h_addr,host->h_length);
	sin.sin_family = host->h_addrtype;
	sin.sin_port = serv->s_port;

	sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock < 0) return -1;
	// host connecting
	if (::connect(sock,(struct sockaddr*)(&sin),sizeof(sin)) < 0) return -1;
	return sock;
}

bool checkLpdQueue(const char *host, const char *queue)
{
	int	sock = openConnection(host);
	if (sock < 0) return false;

	char	res[64] = {0};
	snprintf(res,64,"%c%s\n",(char)4,queue);
kdDebug() << "write: " << res << endl;
	if ((int)::write(sock,res,strlen(res)) != (int)strlen(res))
	{
kdDebug() << "connection closed" << endl;
		close(sock);
		return false;
	}

	char	buf[1024] = {0};
	int	n, tot(1);
kdDebug() << "reading" << endl;
	while ((n=::read(sock,res,63)) > 0)
	{
		res[n] = 0;
		tot += n;
kdDebug() << buf << endl;
		if (tot >= 1024)
			break;
		strncat(buf,res,1023);
	}
	close(sock);
	if (strlen(buf) == 0 || strstr(buf,"unknown printer") != NULL) return false;
	else return true;
}
