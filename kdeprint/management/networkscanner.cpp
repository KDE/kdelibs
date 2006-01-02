/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <kdeprint@swing.be>
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

#define USE_QSOCKET

#include "networkscanner.h"

#include <qprogressbar.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qregexp.h>

#include <kpushbutton.h>
#include <klocale.h>
#include <kresolver.h>
#include <kreverseresolver.h>
#include <kbufferedsocket.h>
#include <kmessagebox.h>
#include <knumvalidator.h>
#include <kdebug.h>
#include <unistd.h>

using namespace KNetwork;

class NetworkScanner::NetworkScannerPrivate
{
public:
	int port;
	QString prefixaddress;
	int currentaddress;
	int timeout;
	bool scanning;
	Q3PtrList<NetworkScanner::SocketInfo> printers;

	QProgressBar *bar;
	KPushButton *scan, *settings;
	QLabel *subnetlab;
	QTimer *timer;
	KBufferedSocket *socket;

	NetworkScannerPrivate( int portvalue ) : port( portvalue )
	{
		prefixaddress = localPrefix();
		currentaddress = 1;
		timeout = 50;
		scanning = false;
		printers.setAutoDelete( true );
	}
	QString localPrefix();
	QString scanString();
};

QString NetworkScanner::NetworkScannerPrivate::localPrefix()
{
#warning "Upgrade NetworkScanner with KNetworkInterface"
	char	buf[256];
	buf[0] = '\0';
	if (!gethostname(buf, sizeof(buf)))
		buf[sizeof(buf)-1] = '\0';
	KResolverResults infos = KResolver::resolve(buf, "80");

	if (infos.count() > 0)
	{
		QString	IPstr = infos[0].address().nodeName();
		int	p = IPstr.lastIndexOf('.');	// this is wrong!! -thiago
		IPstr.truncate(p);
		return IPstr;
	}
	return QString();
}

QString NetworkScanner::NetworkScannerPrivate::scanString()
{
	QString s = prefixaddress + ".*";
	if ( port != -1 )
		s.append( ":" ).append( QString::number( port ) );
	return s;
}

NetworkScanner::NetworkScanner( int port, QWidget *parent )
	: QWidget( parent )
{
	d = new NetworkScannerPrivate( port );
	d->bar = new QProgressBar( this );
	d->bar->setRange( 0, 256 );
	d->settings = new KPushButton( KGuiItem( i18n( "&Settings" ), "configure" ), this );
	d->scan = new KPushButton( KGuiItem( i18n( "Sc&an" ), "viewmag" ), this );
	d->timer = new QTimer( this );
	d->socket = new KBufferedSocket( QString(), QString(), this );
	QLabel *label = new QLabel( i18n( "Network scan:" ), this );
	d->subnetlab = new QLabel( i18n( "Subnet: %1" ).arg( d->scanString() ), this );

	QGridLayout *l0 = new QGridLayout( this, 4, 2, 0, 10 );
	l0->addMultiCellWidget( label, 0, 0, 0, 1 );
	l0->addMultiCellWidget( d->bar, 1, 1, 0, 1 );
	l0->addMultiCellWidget( d->subnetlab, 2, 2, 0, 1 );
	l0->addWidget( d->settings, 3, 0 );
	l0->addWidget( d->scan, 3, 1 );

	connect( d->timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
	connect( d->settings, SIGNAL( clicked() ), SLOT( slotSettingsClicked() ) );
	connect( d->scan, SIGNAL( clicked() ), SLOT( slotScanClicked() ) );

	connect( d->socket, SIGNAL( connected( const KNetwork::KResolverEntry& ) ), SLOT( slotConnectionSuccess() ) );
	connect( d->socket, SIGNAL( gotError( int ) ), SLOT( slotConnectionFailed( int ) ) );
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
	d->scan->setGuiItem( KGuiItem( i18n( "&Abort" ), "stop" ) );
	d->currentaddress = -1;
	d->scanning = true;
	next();
}

void NetworkScanner::slotScanClicked()
{
	if ( !d->scanning )
	{
		if ( d->localPrefix() == d->prefixaddress ||
				KMessageBox::warningContinueCancel( this->parentWidget(),
					i18n( "You are about to scan a subnet (%1.*) that does not "
						  "correspond to the current subnet of this computer (%2.*). Do you want "
						  "to scan the specified subnet anyway?" ).arg( d->prefixaddress ).arg( d->localPrefix() ),
					QString(), KGuiItem( i18n( "&Scan" ), "viewmag" ), "askForScan" ) == KMessageBox::Continue )
			start();
	}
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
	d->scan->setGuiItem( KGuiItem( i18n( "Sc&an" ), "viewmag" ) );
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
	d->socket->connect( d->prefixaddress + "." + QString::number( d->currentaddress ), QString::number(d->port) );
	kdDebug() << "Address: " << d->socket->peerAddress().toString() << endl;
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
		d->bar->setValue( d->currentaddress );
		QTimer::singleShot( 0, this, SLOT( slotNext() ) );
	}
}

void NetworkScanner::slotTimeout()
{
	kdDebug() << "Timeout" << endl;
	if ( !d->scanning )
		return;

	d->socket->close();
	next();
}

void NetworkScanner::slotConnectionSuccess( const KResolverEntry& target )
{
	kdDebug() << "Success" << endl;
	kdDebug() << "Connection success: " << target.address().toString() << endl;
	//kdDebug() << "Socket: " << d->socket->socket() << endl;

	KInetSocketAddress addr = target.address().asInet();
	if ( addr.ipVersion() )
	{
		SocketInfo *info = new SocketInfo;
		info->IP = addr.ipAddress().toString();
		info->Port = d->port;

		QString portname;
		KReverseResolver::resolve( addr, info->Name, portname );
		d->printers.append( info );
		d->socket->close();
	}
	else
		kdDebug() << "Connected to something odd!" << endl;
	next();
}

void NetworkScanner::slotConnectionFailed( int )
{
	kdDebug() << "Failure" << endl;
	next();
}

const Q3PtrList<NetworkScanner::SocketInfo>* NetworkScanner::printerList()
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
	d->subnetlab->setText( i18n( "Subnet: %1" ).arg( d->scanString() ) );
}

int NetworkScanner::port() const
{
	return d->port;
}

void NetworkScanner::setPort( int p )
{
	d->port = p;
	d->subnetlab->setText( i18n( "Subnet: %1" ).arg( d->scanString() ) );
}

bool NetworkScanner::checkPrinter( const QString& host, int port )
{
	// try first to find it in the SocketInfo list
	Q3PtrListIterator<NetworkScanner::SocketInfo> it( d->printers );
	for ( ; it.current(); ++it )
	{
		if ( port == it.current()->Port && ( host == it.current()->IP ||
					host == it.current()->Name ) )
			return true;
	}

	// not found in SocketInfo list, try to establish connection
	KStreamSocket sock( host, QString::number(port) );
	sock.setTimeout( d->timeout );
	return sock.connect();
}

NetworkScannerConfig::NetworkScannerConfig(NetworkScanner *scanner, const char *name)
	: KDialogBase(scanner, name, true, QString(), Ok|Cancel, Ok, true)
{
	scanner_ = scanner;
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
	port_ = new QComboBox(true,dummy);
        if ( port_->lineEdit() )
            port_->lineEdit()->setValidator( val );
	tout_ = new QLineEdit(dummy);
        tout_->setValidator( val );

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
