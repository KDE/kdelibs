/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <goffioul@imec.be>
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

#include "networkscanner.h"

#include <qprogressbar.h>
#include <kpushbutton.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qregexp.h>
#include <qsocket.h>
#include <klocale.h>
#include <kextendedsocket.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <unistd.h>

class NetworkScanner::NetworkScannerPrivate
{
public:
	int port;
	QString prefixaddress;
	int currentaddress;
	int timeout;
	bool scanning;
	QPtrList<NetworkScanner::SocketInfo> printers;

	QProgressBar *bar;
	QPushButton *scan, *settings;
	QTimer *timer;
	QSocket *socket;

	NetworkScannerPrivate( int portvalue ) : port( portvalue )
	{
		prefixaddress = localPrefix();
		currentaddress = 1;
		timeout = 50;
		scanning = false;
		printers.setAutoDelete( true );
	}
	QString localPrefix();
};

QString NetworkScanner::NetworkScannerPrivate::localPrefix()
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

NetworkScanner::NetworkScanner( int port, QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	d = new NetworkScannerPrivate( port );
	d->bar = new QProgressBar( 256, this );
	d->settings = new KPushButton( KGuiItem( i18n( "&Settings" ), "configure" ), this );
	d->scan = new KPushButton( KGuiItem( i18n( "Sc&an" ), "viewmag" ), this );
	d->timer = new QTimer( this );
	d->socket = new QSocket( this );
	QLabel *label = new QLabel( i18n( "Network Scan:" ), this );

	QGridLayout *l0 = new QGridLayout( this, 3, 2, 0, 10 );
	l0->addMultiCellWidget( label, 0, 0, 0, 1 );
	l0->addMultiCellWidget( d->bar, 1, 1, 0, 1 );
	l0->addWidget( d->settings, 2, 0 );
	l0->addWidget( d->scan, 2, 1 );

	connect( d->timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
	connect( d->settings, SIGNAL( clicked() ), SLOT( slotSettingsClicked() ) );
	connect( d->scan, SIGNAL( clicked() ), SLOT( slotScanClicked() ) );
	connect( d->socket, SIGNAL( connected() ), SLOT( slotConnectionSuccess() ) );
	connect( d->socket, SIGNAL( error( int ) ), SLOT( slotConnectionFailed( int ) ) );
}

NetworkScanner::~NetworkScanner()
{
	delete d;
}

void NetworkScanner::start()
{
	if ( d->scanning )
		return;

	d->printers.clear();
	emit scanStarted();
	d->settings->setEnabled( false );
	d->scan->setText( i18n( "&Abort" ) );
	d->currentaddress = -1;
	d->scanning = true;
	next();
}

void NetworkScanner::slotScanClicked()
{
	if ( !d->scanning )
		start();
	else
	{
		d->socket->close();
		finish();
	}
}

void NetworkScanner::finish()
{
	if ( !d->scanning )
		return;

	d->settings->setEnabled( true );
	d->scan->setText( i18n( "Sc&an" ) );
	d->bar->reset();
	d->scanning = false;
	emit scanFinished();
}

void NetworkScanner::slotSettingsClicked()
{
	NetworkScannerConfig dlg( this );
	dlg.exec();
}

void NetworkScanner::slotNext()
{
	if ( !d->scanning )
		return;

	d->timer->stop();
	d->socket->connectToHost( d->prefixaddress + "." + QString::number( d->currentaddress ), d->port );
	//kdDebug() << "Address: " << d->socket->peerName() << ", Port: " << d->socket->peerPort() << endl;
	d->timer->start( d->timeout, true );
}

void NetworkScanner::next()
{
	//kdDebug() << "Next" << endl;
	d->currentaddress++;
	if ( d->currentaddress >= 256 )
		finish();
	else
	{
		d->bar->setProgress( d->currentaddress );
		QTimer::singleShot( 0, this, SLOT( slotNext() ) );
	}
}

void NetworkScanner::slotTimeout()
{
	//kdDebug() << "Timeout" << endl;
	if ( !d->scanning )
		return;

	d->socket->close();
	next();
}

void NetworkScanner::slotConnectionSuccess()
{
	//kdDebug() << "Success" << endl;
	SocketInfo *info = new SocketInfo;
	info->IP = d->socket->peerName();
	info->Port = d->port;
	QString portname;
	KExtendedSocket::resolve( KExtendedSocket::peerAddress( d->socket->socket() ), info->Name, portname );
	d->printers.append( info );
	d->socket->close();
	next();
}

void NetworkScanner::slotConnectionFailed( int )
{
	//kdDebug() << "Failure" << endl;
	next();
}

const QPtrList<NetworkScanner::SocketInfo>* NetworkScanner::printerList()
{
	return &( d->printers );
}

int NetworkScanner::timeout() const
{
	return d->timeout;
}

void NetworkScanner::setTimeout( int to )
{
	d->timeout = to;
}

QString NetworkScanner::subnet() const
{
	return d->prefixaddress;
}

void NetworkScanner::setSubnet( const QString& sn )
{
	d->prefixaddress = sn;
}

int NetworkScanner::port() const
{
	return d->port;
}

void NetworkScanner::setPort( int p )
{
	d->port = p;
}

NetworkScannerConfig::NetworkScannerConfig(NetworkScanner *scanner, const char *name)
	: KDialogBase(scanner, name, true, QString::null, Ok|Cancel, Ok, true)
{
	scanner_ = scanner;
	QWidget	*dummy = new QWidget(this);
	setMainWidget(dummy);

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
	tout_ = new QLineEdit(dummy);

	masklabel->setBuddy(mask_);
	portlabel->setBuddy(port_);
	toutlabel->setBuddy(tout_);

	mask_->setText(scanner_->subnet());
	port_->insertItem("631");
	port_->insertItem("9100");
	port_->insertItem("9101");
	port_->insertItem("9102");
	port_->setEditText(QString::number(scanner_->port()));
	tout_->setText(QString::number(scanner_->timeout()));

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

NetworkScannerConfig::~NetworkScannerConfig()
{
}

void NetworkScannerConfig::slotOk()
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

	scanner_->setTimeout( tout_->text().toInt() );
	scanner_->setSubnet( mask_->text() );
	scanner_->setPort( port_->currentText().toInt() );

	KDialogBase::slotOk();
}

#include "networkscanner.moc"
