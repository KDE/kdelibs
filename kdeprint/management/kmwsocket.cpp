/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
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

#include "kmwsocket.h"
#include "networkscanner.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <klistview.h>
#include <q3header.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <kmessagebox.h>
#include <qlayout.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kseparator.h>

KMWSocket::KMWSocket(QWidget *parent)
    : KMWizardPage(parent)
{
	m_title = i18n("Network Printer Information");
	m_ID = KMWizard::TCP;
	m_nextpage = KMWizard::Driver;

	m_list = new KListView(this);
	m_list->addColumn("");
	m_list->header()->hide();
	m_list->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	m_list->setLineWidth(1);

	QLabel	*l1 = new QLabel(i18n("&Printer address:"),this);
	QLabel	*l2 = new QLabel(i18n("P&ort:"),this);

	m_printer = new QLineEdit(this);
	m_port = new QLineEdit(this);

	l1->setBuddy(m_printer);
	l2->setBuddy(m_port);

	m_scanner = new NetworkScanner( 9100, this );

	KSeparator* sep = new KSeparator( Qt::Horizontal, this);
	sep->setFixedHeight(40);

	connect(m_list,SIGNAL(selectionChanged(Q3ListViewItem*)),SLOT(slotPrinterSelected(Q3ListViewItem*)));
	connect( m_scanner, SIGNAL( scanStarted() ), SLOT( slotScanStarted() ) );
	connect( m_scanner, SIGNAL( scanFinished() ), SLOT( slotScanFinished() ) );
	connect( m_scanner, SIGNAL( scanStarted() ), parent, SLOT( disableWizard() ) );
	connect( m_scanner, SIGNAL( scanFinished() ), parent, SLOT( enableWizard() ) );

	// layout
	QHBoxLayout	*lay3 = new QHBoxLayout(this, 0, 10);
	QVBoxLayout	*lay2 = new QVBoxLayout(0, 0, 0);

	lay3->addWidget(m_list,1);
	lay3->addLayout(lay2,1);
	lay2->addWidget(l1);
	lay2->addWidget(m_printer);
	lay2->addSpacing(10);
	lay2->addWidget(l2);
	lay2->addWidget(m_port);
	lay2->addWidget(sep);
	lay2->addWidget( m_scanner );
	lay2->addStretch(1);
}

KMWSocket::~KMWSocket()
{
}

void KMWSocket::updatePrinter(KMPrinter *p)
{
	QString	dev = QString::fromLatin1("socket://%1:%2").arg(m_printer->text()).arg(m_port->text());
	p->setDevice(dev);
}

bool KMWSocket::isValid(QString& msg)
{
	if (m_printer->text().isEmpty())
	{
		msg = i18n("You must enter a printer address.");
		return false;
	}
	QString	port(m_port->text());
	int	p(9100);
	if (!port.isEmpty())
	{
		bool	ok;
		p = port.toInt(&ok);
		if (!ok)
		{
			msg = i18n("Wrong port number.");
			return false;
		}
	}

	if (!m_scanner->checkPrinter(m_printer->text(),p))
	{
		msg = i18n("No printer found at this address/port.");
		return false;
	}
	return true;
}

void KMWSocket::slotScanStarted()
{
	m_list->clear();
}

void KMWSocket::slotScanFinished()
{
	const QList<NetworkScanner::SocketInfo*>	*list = m_scanner->printerList();
	QListIterator<NetworkScanner::SocketInfo*>	it(*list);
	while (it.hasNext())
	{
		QString	name;
    NetworkScanner::SocketInfo *info(it.next());
		if (info->Name.isEmpty())
			name = i18n("Unknown host - 1 is the IP", "<Unknown> (%1)").arg(info->IP);
		else
			name = info->Name;
		Q3ListViewItem	*item = new Q3ListViewItem(m_list,name,info->IP,QString::number(info->Port));
		item->setPixmap(0,SmallIcon("kdeprint_printer"));
	}
}

void KMWSocket::slotPrinterSelected(Q3ListViewItem *item)
{
	if (!item) return;
	m_printer->setText(item->text(1));
	m_port->setText(item->text(2));
}

#include "kmwsocket.moc"
