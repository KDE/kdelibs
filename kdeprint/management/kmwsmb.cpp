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

#include "kmwsmb.h"
#include "kmwizard.h"
#include "smbview.h"
#include "kmprinter.h"

#include <klocale.h>
#include <kpushbutton.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>

KMWSmb::KMWSmb(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_title = i18n("SMB Printer Settings");
	m_ID = KMWizard::SMB;
	m_nextpage = KMWizard::Driver;

	m_view = new SmbView(this,"SmbView");
	QPushButton	*m_scan = new KPushButton(KGuiItem(i18n("Scan"), "viewmag"), this);
	QPushButton	*m_abort = new KPushButton(KGuiItem(i18n("Abort"), "stop"), this);
	m_abort->setEnabled(false);
	QLabel		*m_worklabel = new QLabel(i18n("Workgroup:"), this);
	QLabel		*m_serverlabel = new QLabel(i18n("Server:"), this);
	QLabel		*m_printerlabel = new QLabel(i18n("Printer:"), this);
	m_work = new QLineEdit(this);
	m_server = new QLineEdit(this);
	m_printer = new QLineEdit(this);

	QVBoxLayout	*lay0 = new QVBoxLayout(this, 0, 10);
	QGridLayout	*lay1 = new QGridLayout(0, 3, 2, 0, 10);
	QHBoxLayout	*lay3 = new QHBoxLayout(0, 0, 10);
	lay0->addLayout(lay1,0);
	lay0->addWidget(m_view,1);
	lay0->addLayout(lay3,0);
	lay0->addSpacing(10);
	lay1->setColStretch(1,1);
	lay1->addWidget(m_worklabel,0,0);
	lay1->addWidget(m_serverlabel,1,0);
	lay1->addWidget(m_printerlabel,2,0);
	lay1->addWidget(m_work,0,1);
	lay1->addWidget(m_server,1,1);
	lay1->addWidget(m_printer,2,1);
	lay3->addStretch(1);
	lay3->addWidget(m_scan);
	lay3->addWidget(m_abort);

	connect(m_scan,SIGNAL(clicked()),SLOT(slotScan()));
	connect(m_abort,SIGNAL(clicked()),SLOT(slotAbort()));
	connect(m_view,SIGNAL(printerSelected(const QString&,const QString&,const QString&)),SLOT(slotPrinterSelected(const QString&,const QString&,const QString&)));
	connect(m_view,SIGNAL(running(bool)),m_abort,SLOT(setEnabled(bool)));
}

bool KMWSmb::isValid(QString& msg)
{
	if (m_server->text().isEmpty())
		msg = i18n("Empty server name.");
	else if (m_printer->text().isEmpty())
		msg = i18n("Empty printer name.");
	else
		return true;
	return false;
}

void KMWSmb::updatePrinter(KMPrinter *printer)
{
	KURL	url;
	if (m_work->text().isEmpty())
		url = QString::fromLatin1("smb://%1/%2").arg(m_server->text()).arg(m_printer->text());
	else
		url = QString::fromLatin1("smb://%1/%2/%3").arg(m_work->text()).arg(m_server->text()).arg(m_printer->text());
	if (!printer->option("kde-login").isEmpty())
	{
		url.setUser(printer->option("kde-login"));
		if (!printer->option("kde-password").isEmpty())
			url.setPass(printer->option("kde-password"));
	}
	printer->setDevice(url);
}

void KMWSmb::initPrinter(KMPrinter *printer)
{
	if (printer)
	{
		m_view->setLoginInfos(printer->option("kde-login"),printer->option("kde-password"));
	}
}

void KMWSmb::slotScan()
{
	m_view->init();
}

void KMWSmb::slotAbort()
{
	m_view->abort();
}

void KMWSmb::slotPrinterSelected(const QString& work, const QString& server, const QString& printer)
{
	m_work->setText(work);
	m_server->setText(server);
	m_printer->setText(printer);
}
#include "kmwsmb.moc"
