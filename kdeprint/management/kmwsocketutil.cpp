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

#include "kmwsocketutil.h"

#include <qprogressbar.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <kmessagebox.h>
#include <qlayout.h>
#include <qregexp.h>

#include <kapplication.h>
#include <klocale.h>
#include <kextsock.h>
#include <kdebug.h>

#include <unistd.h>

QString localRootIP();

//----------------------------------------------------------------------------------------

SocketConfig::SocketConfig(KMWSocketUtil *util, QWidget *parent, const char *name)
: KDialog(parent,name,true)
{
	QPushButton	*ok = new QPushButton(i18n("Ok"),this);
	QPushButton	*cancel = new QPushButton(i18n("Cancel"),this);
	ok->setDefault(true);

	connect(ok,SIGNAL(clicked()),SLOT(accept()));
	connect(cancel,SIGNAL(clicked()),SLOT(reject()));

	QLabel	*masklabel = new QLabel(i18n("&Subnetwork:"),this);
	QLabel	*portlabel = new QLabel(i18n("&Port:"),this);
	QLabel	*toutlabel = new QLabel(i18n("&Timeout (ms):"),this);
	QLineEdit	*mm = new QLineEdit(this);
	mm->setText(QString::fromLatin1(".[0-255]"));
	mm->setReadOnly(true);
	mm->setFixedWidth(fontMetrics().width(mm->text())+10);

	mask_ = new QLineEdit(this);
	mask_->setAlignment(Qt::AlignRight);
	port_ = new QComboBox(true,this);
	tout_ = new QLineEdit(this);

	masklabel->setBuddy(mask_);
	portlabel->setBuddy(port_);
	toutlabel->setBuddy(tout_);

	mask_->setText(util->root_);
	port_->insertItem("631");
	port_->insertItem("9100");
	port_->insertItem("9101");
	port_->insertItem("9102");
	port_->setEditText(QString::number(util->port_));
	tout_->setText(QString::number(util->timeout_));

	QGridLayout	*main_ = new QGridLayout(this, 4, 2, 10, 10);
	QHBoxLayout	*btn_ = new QHBoxLayout(0, 0, 10), *lay1 = new QHBoxLayout(0, 0, 5);
	main_->addWidget(masklabel, 0, 0);
	main_->addWidget(portlabel, 1, 0);
	main_->addWidget(toutlabel, 2, 0);
	main_->addLayout(lay1, 0, 1);
	main_->addWidget(port_, 1, 1);
	main_->addWidget(tout_, 2, 1);
	main_->addMultiCellLayout(btn_, 4, 4, 0, 1);
	btn_->addStretch(1);
	btn_->addWidget(ok);
	btn_->addWidget(cancel);
	lay1->addWidget(mask_,1);
	lay1->addWidget(mm,0);

	resize(250,130);
	setCaption(i18n("Scan Configuration"));
}

SocketConfig::~SocketConfig()
{
}

void SocketConfig::done(int result)
{
	if (result == Accepted)
	{
		QString	msg;
		QRegExp	re("(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})");
		if (!re.exactMatch(mask_->text()))
			msg = i18n("Wrong subnetwork specification.");
		else
		{
			for (int i=1; i<=3; i++)
				if (re.cap(i).toInt() >= 255)
				{
					msg = i18n("Wrong subnetwork specification.");
					break;
				}
		}

		bool 	ok(false);
		int 	v = tout_->text().toInt(&ok);
		if (!ok || v <= 0)
			msg = i18n("Wrong timeout specification.");
		v = port_->currentText().toInt(&ok);
		if (!ok || v <= 0)
			msg = i18n("Wrong port specification.");
		if (!msg.isEmpty())
		{
			KMessageBox::error(this,msg);
			return;
		}
	}
	KDialog::done(result);
}

//----------------------------------------------------------------------------------------

KMWSocketUtil::KMWSocketUtil()
{
	printerlist_.setAutoDelete(true);
	root_ = localRootIP();
	port_ = 9100;
	timeout_ = 50;
}

bool KMWSocketUtil::checkPrinter(const QString& IPstr, int port, QString* hostname)
{
	KExtendedSocket	sock(IPstr, port, KExtendedSocket::inetSocket|KExtendedSocket::streamSocket);
	bool	result(false);
	sock.setTimeout(0, timeout_ * 1000);
	if (sock.connect() == 0)
	{
		if (hostname)
		{
			QString	portname;
			KExtendedSocket::resolve((KSocketAddress*)(sock.peerAddress()), *hostname, portname);
		}
		result = true;
	}
	sock.close();
	return result;
}

bool KMWSocketUtil::scanNetwork(QProgressBar *bar)
{
	printerlist_.setAutoDelete(true);
	printerlist_.clear();
	int	n(256);
	if (bar)
		bar->setTotalSteps(n);
	for (int i=0; i<n; i++)
	{
		QString	IPstr = root_ + "." + QString::number(i);
		QString	hostname;
		if (checkPrinter(IPstr, port_, &hostname))
		{ // we found a printer at this address, create SocketInfo entry in printer list
			SocketInfo	*info = new SocketInfo;
			info->IP = IPstr;
			info->Port = port_;
			info->Name = hostname;
			printerlist_.append(info);
		}
		if (bar)
		{
			bar->setProgress(i);
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
		root_ = dlg->mask_->text();
		port_ = dlg->port_->currentText().toInt();
		timeout_ = dlg->tout_->text().toInt();
	}
}

//----------------------------------------------------------------------------------------

QString localRootIP()
{
	char	buf[256];
	gethostname(buf, 255);
	QPtrList<KAddressInfo>	infos = KExtendedSocket::lookup(buf, QString::null);
	infos.setAutoDelete(true);
	if (infos.count() > 0)
	{
		QString	IPstr = infos.first()->address()->nodeName();
		int	p = IPstr.findRev('.');
		IPstr.truncate(p);
		return IPstr;
	}
	return QString::null;
}

#include "kmwsocketutil.moc"
