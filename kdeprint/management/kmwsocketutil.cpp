/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id:  $
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

#include "kmwsocketutil.h"

#include <qprogressbar.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <kmessagebox.h>
#include <qlayout.h>

#include <kapp.h>
#include <klocale.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <config.h>


#define	LONGTOIN(x)	(*(struct in_addr*)(&(x)))
#define	INTOLONG(x)	((unsigned long)((x).s_addr))

unsigned long getIP();
unsigned long getIPStart(unsigned long IP);
unsigned long getIPStop(unsigned long IP);
unsigned long getIPStep();

//----------------------------------------------------------------------------------------

class SocketConfig : public QDialog
{
	friend class KMWSocketUtil;

public:
	SocketConfig(KMWSocketUtil *util, QWidget *parent = 0, const char *name = 0);
	~SocketConfig();

private:
	QLineEdit	*start_, *stop_, *tout_;
	QComboBox	*port_;
	QLabel		*startlabel, *stoplabel, *portlabel, *toutlabel;
	QPushButton	*ok, *cancel;
};

SocketConfig::SocketConfig(KMWSocketUtil *util, QWidget *parent, const char *name)
: QDialog(parent,name,true)
{
	ok = new QPushButton(i18n("Ok"),this);
	cancel = new QPushButton(i18n("Cancel"),this);
	ok->setDefault(true);

	connect(ok,SIGNAL(clicked()),SLOT(accept()));
	connect(cancel,SIGNAL(clicked()),SLOT(reject()));

	startlabel = new QLabel(i18n("IP Start"),this);
	stoplabel = new QLabel(i18n("IP Stop"),this);
	portlabel = new QLabel(i18n("Port"),this);
	toutlabel = new QLabel(i18n("Timeout (ms)"),this);

	start_ = new QLineEdit(this);
	stop_ = new QLineEdit(this);
	port_ = new QComboBox(true,this);
	tout_ = new QLineEdit(this);

	start_->setText(inet_ntoa(LONGTOIN(util->start_)));
	stop_->setText(inet_ntoa(LONGTOIN(util->stop_)));
	port_->insertItem("9100");
	port_->insertItem("9101");
	port_->insertItem("9102");
	QString	num;
	num.setNum(util->port_);
	port_->setEditText(num);
	num.setNum(util->timeout_);
	tout_->setText(num);

	QGridLayout	*main_ = new QGridLayout(this, 5, 2, 10, 10);
	QHBoxLayout	*btn_ = new QHBoxLayout(0, 0, 10);
	main_->addWidget(startlabel, 0, 0);
	main_->addWidget(stoplabel, 1, 0);
	main_->addWidget(portlabel, 2, 0);
	main_->addWidget(toutlabel, 3, 0);
	main_->addWidget(start_, 0, 1);
	main_->addWidget(stop_, 1, 1);
	main_->addWidget(port_, 2, 1);
	main_->addWidget(tout_, 3, 1);
	main_->addMultiCellLayout(btn_, 4, 4, 0, 1);
	btn_->addStretch(1);
	btn_->addWidget(ok);
	btn_->addWidget(cancel);

	resize(250,150);
	setCaption(i18n("Scan configuration"));
}

SocketConfig::~SocketConfig()
{
}

//----------------------------------------------------------------------------------------

KMWSocketUtil::KMWSocketUtil()
{
	printerlist_.setAutoDelete(true);
	unsigned long	IP = getIP();
	start_ = getIPStart(IP);
	stop_ = getIPStop(IP);
	port_ = 9100;
	timeout_ = 50;
}

bool KMWSocketUtil::checkPrinter(const char *host, int port)
{
	struct hostent	*ent = gethostbyname(host);
	if (ent)
	{
		unsigned long	IP = *(unsigned long*)(ent->h_addr);
		return checkPrinter(IP,port);
	}
	return false;
}

bool KMWSocketUtil::checkPrinter(unsigned long IP, int port)
{
	struct sockaddr_in	sin;
	int	sock;
	bool	val(false);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr = *(struct in_addr*)&IP;
	sock = ::socket(AF_INET,SOCK_STREAM,0);
	if (sock < 0) return false;
	fcntl(sock,F_SETFL,fcntl(sock,F_GETFL)|O_NONBLOCK);
	if (::connect(sock,(struct sockaddr*)&sin,sizeof(sin)) < 0)
	{
		if (errno == EINPROGRESS)
		{
			struct timeval	tout = {0, timeout_*1000};
			fd_set	wfd;
			FD_ZERO(&wfd);
			FD_SET(sock,&wfd);
			if (::select(sock+1,0,&wfd,0,&tout) <= 0)
				goto end;
			int	res;
			ksize_t	len(sizeof(res));
			getsockopt(sock,SOL_SOCKET,SO_ERROR,(char *)&res,&len);
			if (res != 0) goto end;
		}
		else goto end;
	}
	val = true;
end:	::close(sock);
	return val;
}

bool KMWSocketUtil::scanNetwork(QProgressBar *bar)
{
	printerlist_.setAutoDelete(true);
	printerlist_.clear();
	unsigned long	IP = getIP();
	if (start_ == 0) start_ = getIPStart(IP);
	if (stop_ == 0) stop_ = getIPStop(IP);
	unsigned long	step = getIPStep();
	int	n = (stop_-start_)/step+1;
	if (bar) bar->setTotalSteps(n);
	for (unsigned long ip=start_,count=0;ip<=stop_ && count <= 255;ip+=step,count++)
	{
		if (checkPrinter(ip,port_))
		{ // we found a printer at this address, create SocketInfo entry in printer list
			SocketInfo	*info = new SocketInfo;
			info->IP = inet_ntoa(LONGTOIN(ip));
			info->Port = port_;
			// look for printer name
			struct hostent	*print = gethostbyaddr((char*)&ip,sizeof(ip),AF_INET);
			if (print) info->Name = print->h_name;
			// add it to list
			printerlist_.append(info);
		}
		if (bar)
		{
			bar->setProgress(count);
//			kapp->processEvents(10);
			kapp->flushX();
		}
	}
	return true;
}

void KMWSocketUtil::configureScan(QWidget *parent)
{
	SocketConfig	*dlg = new SocketConfig(this,parent);
	if (dlg->exec())
	{
		unsigned long	i1(inet_addr(dlg->start_->text().latin1())), i2(inet_addr(dlg->stop_->text().latin1()));
		bool	ok1, ok2;
		int	p = QString(dlg->port_->currentText()).toInt(&ok1);
		int	t = QString(dlg->tout_->text()).toInt(&ok2);
		if (i2 < i1 || (long)i2 == -1 || (long)i1 == -1 || !ok1 || !ok2)
		{
			KMessageBox::error(parent,i18n("Wrong settings."));
			return;
		}
		start_ = i1;
		stop_ = i2;
		port_ = p;
		timeout_ = t;
	}
}

//----------------------------------------------------------------------------------------

unsigned long getIP()
{
	char	buf[256];
	struct hostent	*host;
	gethostname(buf,255);
	host = gethostbyname(buf);
	if (host) return (*(unsigned long*)(host->h_addr));
	else return 0xFFFFFFFF;
}

unsigned long getIPStart(unsigned long IP)
{
	return ((inet_addr("255.255.255.1") & IP) | inet_addr("0.0.0.1"));
}

unsigned long getIPStop(unsigned long IP)
{
	return (inet_addr("0.0.0.254") | IP);
}

unsigned long getIPStep()
{
	return (inet_addr("0.0.0.1"));
}
