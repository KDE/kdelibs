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

#include "kmwippprinter.h"
#include "kmwizard.h"
#include "kmprinter.h"
#include "ipprequest.h"
#include "kmcupsmanager.h"
#include "networkscanner.h"

#include <klistview.h>
#include <q3header.h>
#include <kpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <kmessagebox.h>
#include <q3textview.h>
#include <qlayout.h>
#include <qregexp.h>
#include <kseparator.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kurl.h>

KMWIppPrinter::KMWIppPrinter(QWidget *parent)
    : KMWizardPage(parent)
{
	m_title = i18n("IPP Printer Information");
	m_ID = KMWizard::Custom+1;
	m_nextpage = KMWizard::Driver;

	m_list = new KListView(this);
	m_list->addColumn("");
	m_list->header()->hide();
	m_list->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	m_list->setLineWidth(1);

	QLabel	*l1 = new QLabel(i18n("&Printer URI:"),this);

	m_uri = new QLineEdit(this);

	l1->setBuddy(m_uri);

	m_info = new Q3TextView(this);
	m_info->setPaper(colorGroup().background());
	m_info->setMinimumHeight(100);
	m_info->setText(i18n("<p>Either enter the printer URI directly, or use the network scanning facility.</p>"));
	m_ippreport = new KPushButton(KGuiItem(i18n("&IPP Report"), "kdeprint_report"), this);
	m_ippreport->setEnabled(false);

	m_scanner = new NetworkScanner( 631, this );

	KSeparator* sep = new KSeparator(this);
	sep->setFixedHeight(20);

	connect(m_list,SIGNAL(selectionChanged(Q3ListViewItem*)),SLOT(slotPrinterSelected(Q3ListViewItem*)));
	connect( m_scanner, SIGNAL( scanStarted() ), SLOT( slotScanStarted() ) );
	connect( m_scanner, SIGNAL( scanFinished() ), SLOT( slotScanFinished() ) );
	connect( m_scanner, SIGNAL( scanStarted() ), parent, SLOT( disableWizard() ) );
	connect( m_scanner, SIGNAL( scanFinished() ), parent, SLOT( enableWizard() ) );
	connect(m_ippreport, SIGNAL(clicked()), SLOT(slotIppReport()));

	// layout
	QHBoxLayout	*lay3 = new QHBoxLayout(this, 0, 10);
	QVBoxLayout	*lay2 = new QVBoxLayout(0, 0, 0);
	QHBoxLayout	*lay4 = new QHBoxLayout(0, 0, 0);

	lay3->addWidget(m_list,1);
	lay3->addLayout(lay2,1);
	lay2->addWidget(l1);
	lay2->addWidget(m_uri);
	lay2->addSpacing(10);
	lay2->addWidget(m_info, 1);
	lay2->addSpacing(5);
	lay2->addLayout(lay4);
	lay4->addStretch(1);
	lay4->addWidget(m_ippreport);
	lay2->addWidget(sep);
	lay2->addWidget( m_scanner );
}

KMWIppPrinter::~KMWIppPrinter()
{
}

void KMWIppPrinter::updatePrinter(KMPrinter *p)
{
	p->setDevice(m_uri->text());
}

bool KMWIppPrinter::isValid(QString& msg)
{
	if (m_uri->text().isEmpty())
	{
		msg = i18n("You must enter a printer URI.");
		return false;
	}

	KUrl uri( m_uri->text() );
	if (!m_scanner->checkPrinter(uri.host(),uri.port(631)))
	{
		msg = i18n("No printer found at this address/port.");
		return false;
	}
	return true;
}

void KMWIppPrinter::slotScanStarted()
{
	m_list->clear();
}

void KMWIppPrinter::slotScanFinished()
{
	m_ippreport->setEnabled(false);
	const QList<NetworkScanner::SocketInfo*>	*list = m_scanner->printerList();
	QListIterator<NetworkScanner::SocketInfo*>	it(*list);
	while (it.hasNext())
	{
    NetworkScanner::SocketInfo *info(it.next());
		QString	name;
		if (info->Name.isEmpty())
			name = i18n("Unknown host - 1 is the IP", "<Unknown> (%1)").arg(info->IP);
		else
		name = info->Name;
		Q3ListViewItem	*item = new Q3ListViewItem(m_list,name,info->IP,QString::number(info->Port));
		item->setPixmap(0,SmallIcon("kdeprint_printer"));
	}
}

void KMWIppPrinter::slotPrinterSelected(Q3ListViewItem *item)
{
	m_ippreport->setEnabled(item != 0);
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

void KMWIppPrinter::slotIppReport()
{
	IppRequest	req;
	QString	uri("ipp://%1:%2/ipp");
	Q3ListViewItem	*item = m_list->currentItem();

	if (item)
	{
		req.setOperation(IPP_GET_PRINTER_ATTRIBUTES);
		req.setHost(item->text(1));
		req.setPort(item->text(2).toInt());
		uri = uri.arg(item->text(1)).arg(item->text(2));
		req.addURI(IPP_TAG_OPERATION, "printer-uri", uri);
		if (req.doRequest("/ipp/"))
		{
			QString	caption = i18n("IPP Report for %1").arg(item->text(0));
			static_cast<KMCupsManager*>(KMManager::self())->ippReport(req, IPP_TAG_PRINTER, caption);
		}
		else
			KMessageBox::error(this, i18n("Unable to generate report. IPP request failed with message: "
			                              "%1 (0x%2).").arg(ippErrorString((ipp_status_t)req.status())).arg(req.status(),0,16));
	}
}

#include "kmwippprinter.moc"
