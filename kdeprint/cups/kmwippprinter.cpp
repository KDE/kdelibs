/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
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

#include "kmwippprinter.h"
#include "kmwsocketutil.h"
#include "kmwizard.h"
#include "kmprinter.h"
#include "ipprequest.h"

#include <klistview.h>
#include <qheader.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <kmessagebox.h>
#include <qtextview.h>
#include <qlayout.h>
#include <qregexp.h>
#include <kseparator.h>
#include <klocale.h>
#include <kiconloader.h>

KMWIppPrinter::KMWIppPrinter(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_title = i18n("IPP printer information");
	m_ID = KMWizard::Custom+1;
	m_nextpage = KMWizard::Driver;

	m_util = new KMWSocketUtil();
	m_util->setDefaultPort(631);

	m_list = new KListView(this);
	m_list->addColumn("");
	m_list->header()->hide();
	m_list->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	m_list->setLineWidth(1);

	QLabel	*l1 = new QLabel(i18n("Printer URI"),this);
	QLabel	*l3 = new QLabel(i18n("Network scan"), this);

	m_uri = new QLineEdit(this);
	m_bar = new QProgressBar(this);
	m_info = new QTextView(this);
	m_info->setPaper(colorGroup().background());
	m_info->setMinimumHeight(100);
	m_info->setText(i18n("<p>Either enter the printer URI directly, or use the network scanning facility.</p>"));

	QPushButton	*settings_ = new QPushButton(i18n("Settings"),this);
	QPushButton	*scan_ = new QPushButton(i18n("Scan"),this);

	KSeparator* sep = new KSeparator( KSeparator::HLine, this);
	sep->setFixedHeight(20);

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
	lay2->addWidget(m_uri);
	lay2->addSpacing(10);
	lay2->addWidget(m_info, 1);
	lay2->addWidget(sep);
	lay2->addWidget(l3);
	lay2->addSpacing(10);
	lay2->addWidget(m_bar);
	lay2->addSpacing(10);
	lay2->addLayout(lay1);
	//lay2->addStretch(1);
	lay1->addWidget(settings_);
	lay1->addWidget(scan_);
}

KMWIppPrinter::~KMWIppPrinter()
{
	delete m_util;
}

void KMWIppPrinter::updatePrinter(KMPrinter *p)
{
	p->setDevice(KURL(m_uri->text()));
}

bool KMWIppPrinter::isValid(QString& msg)
{
	if (m_uri->text().isEmpty())
	{
		msg = i18n("You must enter a printer URI !");
		return false;
	}
	/*if (!m_util->checkPrinter(m_printer->text().utf8(),p))
	{
		msg = i18n("No printer found at this address/port !");
		return false;
	}*/
	return true;
}

void KMWIppPrinter::slotSettings()
{
	m_util->configureScan(this);
}

void KMWIppPrinter::slotScan()
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

void KMWIppPrinter::slotPrinterSelected(QListViewItem *item)
{
	if (!item) return;

	// trying to get printer attributes
	IppRequest	req;
	QString		uri;
	QStringList	keys;

	req.setOperation(IPP_GET_PRINTER_ATTRIBUTES);
	req.setHost(item->text(1));
	req.setPort(item->text(2).toInt());
	uri = QString::fromLatin1("ipp://%1:%2/ipp").arg(item->text(1)).arg(item->text(2));
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	keys.append("printer-name");
	keys.append("printer-state");
	keys.append("printer-info");
	keys.append("printer-uri-supported");
	keys.append("printer-make-and-model");
	keys.append("printer-location");
	req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",keys);
	if (req.doRequest("/ipp/") && (req.status() == IPP_OK || req.status() == IPP_OK_SUBST || req.status() == IPP_OK_CONFLICT))
	{
		QString	value, txt;
		int 	state;
		if (req.name("printer-name",value)) txt.append(i18n("<b>Name</b>: %1<br>").arg(value));
		if (req.text("printer-location",value) && !value.isEmpty()) txt.append(i18n("<b>Location</b>: %1<br>").arg(value));
		if (req.text("printer-info",value) && !value.isEmpty()) txt.append(i18n("<b>Description</b>: %1<br>").arg(value.replace(QRegExp(";"),"<br>")));
		if (req.uri("printer-uri-supported",value))
		{
			if (value[0] == '/')
				value.prepend(QString::fromLatin1("ipp://%1:%2").arg(item->text(1)).arg(item->text(2)));
			m_uri->setText(value);
		}
		if (req.text("printer-make-and-model",value) && !value.isEmpty()) txt.append(i18n("<b>Model</b>: %1<br>").arg(value));
		if (req.enumvalue("printer-state",state))
		{
			switch (state)
			{
				case IPP_PRINTER_IDLE: value = i18n("Idle"); break;
				case IPP_PRINTER_STOPPED: value = i18n("Stopped"); break;
				case IPP_PRINTER_PROCESSING: value = i18n("Processing..."); break;
				default: value = i18n("Unknown State", "Unknown"); break;
			}
			txt.append(i18n("<b>State</b>: %1<br>").arg(value));
		}
		m_info->setText(txt);
	}
	else
	{
		m_uri->setText(uri);
		m_info->setText(i18n("Unable to retrieve printer info. Printer answered:<br><br>%1").arg(ippErrorString((ipp_status_t)req.status())));
	}
}
#include "kmwippprinter.moc"
