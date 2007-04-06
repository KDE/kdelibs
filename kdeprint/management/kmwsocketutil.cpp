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

#include <QtGui/QProgressBar>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtGui/QComboBox>
#include <QtGui/QPushButton>
#include <kmessagebox.h>
#include <QtGui/QLayout>
#include <QtCore/QRegExp>
#include <QtNetwork/QHostInfo>
#include <knumvalidator.h>

#include <QtGui/QApplication>
#include <QtNetwork/QTcpSocket>
#include <klocale.h>
#include <kdebug.h>
#include <ksocketfactory.h>

#include <unistd.h>

static QString localRootIP();

//----------------------------------------------------------------------------------------

SocketConfig::SocketConfig(KMWSocketUtil *util, QWidget *parent, const char *name)
: KDialog( parent )
{
  setObjectName( name );
  setModal( true );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );

	QWidget	*dummy = new QWidget(this);
	setMainWidget(dummy);
        KIntValidator *val = new KIntValidator( this );
	QLabel	*masklabel = new QLabel(i18n("&Subnetwork:"),dummy);
	QLabel	*portlabel = new QLabel(i18n("&Port:"),dummy);
	QLabel	*toutlabel = new QLabel(i18n("&Timeout (ms):"),dummy);
	QLineEdit	*mm = new QLineEdit(dummy);
	mm->setText(QLatin1String(".[0-255]"));
	mm->setReadOnly(true);
	mm->setFixedWidth(fontMetrics().width(mm->text())+10);

	mask_ = new QLineEdit(dummy);
	mask_->setAlignment(Qt::AlignRight);
	port_ = new QComboBox(dummy);
	port_->setEditable(true);
        if ( port_->lineEdit() )
            port_->lineEdit()->setValidator( val );
	tout_ = new QLineEdit(dummy);
        tout_->setValidator( val );

	masklabel->setBuddy(mask_);
	portlabel->setBuddy(port_);
	toutlabel->setBuddy(tout_);

	mask_->setText(util->root_);
	port_->addItem("631");
	port_->addItem("9100");
	port_->addItem("9101");
	port_->addItem("9102");
	port_->setEditText(QString::number(util->port_));
	tout_->setText(QString::number(util->timeout_));

	QGridLayout	*main_ = new QGridLayout(dummy);
  main_->setMargin(0);
  main_->setSpacing(10);
	QHBoxLayout	*lay1 = new QHBoxLayout();
	main_->addWidget(masklabel, 0, 0);
	main_->addWidget(portlabel, 1, 0);
	main_->addWidget(toutlabel, 2, 0);
	main_->addLayout(lay1, 0, 1);
	main_->addWidget(port_, 1, 1);
	main_->addWidget(tout_, 2, 1);
  lay1->setMargin(0);
  lay1->setSpacing(5);
	lay1->addWidget(mask_,1);
	lay1->addWidget(mm,0);

	resize(250,130);
	setCaption(i18n("Scan Configuration"));
	connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
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

	KDialog::accept();
}

//----------------------------------------------------------------------------------------

KMWSocketUtil::KMWSocketUtil()
{
	root_ = localRootIP();
	port_ = 9100;
	timeout_ = 50;
}

KMWSocketUtil::~KMWSocketUtil()
{
	qDeleteAll(printerlist_);
}

bool KMWSocketUtil::checkPrinter(const QString& IPstr, int port, QString* hostname)
{
	QTcpSocket *sock = KSocketFactory::synchronousConnectToHost("ipp", IPstr, port, timeout_);
	if (sock->isOpen())
	{
		if (hostname)
		{
			QHostInfo info = QHostInfo::fromName(sock->peerAddress().toString());
			*hostname = info.hostName();
		}
		return true;
	}
	else
		return false;
}

bool KMWSocketUtil::scanNetwork(QProgressBar *bar)
{
	qDeleteAll(printerlist_);
	printerlist_.clear();
	int	n(256);
	if (bar)
		bar->setRange(0, n);
	for (int i=0; i<n; i++)
	{
		QString	IPstr = root_ + '.' + QString::number(i);
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
			bar->setValue(i);
			qApp->flush();
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

// ### Use QNetworkInterface?
// This function is just plain *wrong* anyways for any netmask that isn't /24
static QString localRootIP()
{
	QHostInfo infos = QHostInfo::fromName(QHostInfo::localHostName());
	if (!infos.error() && !infos.addresses().isEmpty())
	{
		QString	IPstr = infos.addresses().at(0).toString();
		int	p = IPstr.lastIndexOf('.');
		IPstr.truncate(p);
		return IPstr;
	}
	return QString();
}

#include "kmwsocketutil.moc"
