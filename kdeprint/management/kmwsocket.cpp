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

#include "kmwsocket.h"
#include "kmwsocketutil.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <klistview.h>
#include <qheader.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <kmessagebox.h>
#include <qlayout.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kseparator.h>

KMWSocket::KMWSocket(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_title = i18n("Network printer information");
	m_ID = KMWizard::TCP;
	m_nextpage = KMWizard::Driver;

	m_util = new KMWSocketUtil();

	m_list = new KListView(this);
	m_list->addColumn("");
	m_list->header()->hide();
	m_list->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	m_list->setLineWidth(1);

	QLabel	*l1 = new QLabel(i18n("&Printer address:"),this);
	QLabel	*l2 = new QLabel(i18n("P&ort:"),this);
	QLabel	*l3 = new QLabel(i18n("Network scan:"), this);

	m_printer = new QLineEdit(this);
	m_port = new QLineEdit(this);

   l1->setBuddy(m_printer);
   l2->setBuddy(m_port);

	m_bar = new QProgressBar(this);

	QPushButton	*settings_ = new QPushButton(i18n("&Settings"),this);
	QPushButton	*scan_ = new QPushButton(i18n("S&can"),this);

	KSeparator* sep = new KSeparator( KSeparator::HLine, this);
	sep->setFixedHeight(40);

	connect(m_list,SIGNAL(selectionChanged(QListViewItem*)),SLOT(slotPrinterSelected(QListViewItem*)));
	connect(scan_,SIGNAL(clicked()),SLOT(slotScan()));
	connect(settings_,SIGNAL(clicked()),SLOT(slotSettings()));

	// layout
	QHBoxLayout	*lay3 = new QHBoxLayout(this, 0, 10);
	QVBoxLayout	*lay2 = new QVBoxLayout(0, 0, 0);
	QHBoxLayout	*lay1 = new QHBoxLayout(0, 0, 10);

	lay3->addWidget(m_list,1);
	lay3->addLayout(lay2,1);
	lay2->addWidget(l1);
	lay2->addWidget(m_printer);
	lay2->addSpacing(10);
	lay2->addWidget(l2);
	lay2->addWidget(m_port);
	lay2->addWidget(sep);
	lay2->addWidget(l3);
	lay2->addSpacing(10);
	lay2->addWidget(m_bar);
	lay2->addSpacing(10);
	lay2->addLayout(lay1);
	lay2->addStretch(1);
	lay1->addWidget(settings_);
	lay1->addWidget(scan_);
}

KMWSocket::~KMWSocket()
{
	delete m_util;
}

void KMWSocket::updatePrinter(KMPrinter *p)
{
	QString	dev = QString::fromLatin1("socket://%1:%2").arg(m_printer->text()).arg(m_port->text());
	p->setDevice(KURL(dev));
}

bool KMWSocket::isValid(QString& msg)
{
	if (m_printer->text().isEmpty())
	{
		msg = i18n("You must enter a printer address !");
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
			msg = i18n("Wrong port number !");
			return false;
		}
	}
	if (!m_util->checkPrinter(m_printer->text().utf8(),p))
	{
		msg = i18n("No printer found at this address/port !");
		return false;
	}
	return true;
}

void KMWSocket::slotSettings()
{
	m_util->configureScan(this);
}

void KMWSocket::slotScan()
{
	if (!m_util->scanNetwork(m_bar))
		KMessageBox::error(this,i18n("Network scan failed"),title());
	else
	{
		m_list->clear();
		const QPtrList<SocketInfo>	*list = m_util->printerList();
		QPtrListIterator<SocketInfo>	it(*list);
		for (;it.current();++it)
		{
			QString	name;
			if (it.current()->Name.isEmpty())
				name = i18n("Unknown host - 1 is the IP", "<Unknown> (%1)").arg(it.current()->IP);
			else
				name = it.current()->Name;
			QListViewItem	*item = new QListViewItem(m_list,name,it.current()->IP,QString::number(it.current()->Port));
			item->setPixmap(0,SmallIcon("kdeprint_printer"));
		}
	}
	m_bar->reset();
}

void KMWSocket::slotPrinterSelected(QListViewItem *item)
{
	if (!item) return;
	m_printer->setText(item->text(1));
	m_port->setText(item->text(2));
}
#include "kmwsocket.moc"
