/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
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
#include <knumvalidator.h>

#include <kapplication.h>
#include <klocale.h>
#include <kextsock.h>
#include <kdebug.h>

#include <unistd.h>

QString localRootIP();

//----------------------------------------------------------------------------------------

SocketConfig::SocketConfig(KMWSocketUtil *util, QWidget *parent, const char *name)
: KDialogBase(parent, name, true, QString::null, Ok|Cancel, Ok, true)
{
	QWidget	*dummy = new QWidget(this);
	setMainWidget(dummy);
        KIntValidator *val = new KIntValidator( this );
	QLabel	*masklabel = new QLabel(i18n("&Subnetwork:"),dummy);
	QLabel	*portlabel = new QLabel(i18n("&Port:"),dummy);
	QLabel	*toutlabel = new QLabel(i18n("&Timeout (ms):"),dummy);
	QLineEdit	*mm = new QLineEdit(dummy);
	mm->setText(QString::fromLatin1(".[0-255]"));
	mm->setReadOnly(true);
	mm->setFixedWidth(fontMetrics().width(mm->text())+10);

	mask_ = new QLineEdit(dummy);
	mask_->setAlignment(Qt::AlignRight);
	port_ = new QComboBox(true,dummy);
        if ( port_->lineEdit() )
            port_->lineEdit()->setValidator( val );
	tout_ = new QLineEdit(dummy);
        tout_->setValidator( val );

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

	QGridLayout	*main_ = new QGridLayout(dummy, 3, 2, 0, 10);
	QHBoxLayout	*lay1 = new QHBoxLayout(0, 0, 5);
	main_->addWidget(masklabel, 0, 0);
	main_->addWidget(portlabel, 1, 0);
	main_->addWidget(toutlabel, 2, 0);
	main_->addLayout(lay1, 0, 1);
	main_->addWidget(port_, 1, 1);
	main_->addWidget(tout_, 2, 1);
	lay1->addWidget(mask_,1);
	lay1->addWidget(mm,0);

	resize(250,130);
	setCaption(i18n("Scan Configuration"));
}

SocketConfig::~SocketConfig()
{
}

void SocketConfig::slotOk()
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

	KDialogBase::slotOk();
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
        delete dlg;
}

//----------------------------------------------------------------------------------------

QString localRootIP()
{
	char	buf[256];
	buf[0] = '\0';
	if (!gethostname(buf, sizeof(buf)))
		buf[sizeof(buf)-1] = '\0';
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
