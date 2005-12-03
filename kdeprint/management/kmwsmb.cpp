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

#include "kmwsmb.h"
#include "kmwizard.h"
#include "smbview.h"
#include "kmprinter.h"
#include "util.h"

#include <klocale.h>
#include <kpushbutton.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>

KMWSmb::KMWSmb(QWidget *parent)
    : KMWizardPage(parent)
{
	m_title = i18n("SMB Printer Settings");
	m_ID = KMWizard::SMB;
	m_nextpage = KMWizard::Driver;

	m_view = new SmbView(this );
        m_view->setObjectName("SmbView");
	m_loginlabel = new QLabel( this );
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
	lay3->addWidget( m_loginlabel );
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
	QString uri = buildSmbURI( m_work->text(), m_server->text(), m_printer->text(), printer->option( "kde-login" ), printer->option( "kde-password" ) );
	printer->setDevice( uri );
}

void KMWSmb::initPrinter(KMPrinter *printer)
{
	if (printer)
	{
		QString login = printer->option( "kde-login" );
		m_view->setLoginInfos(login,printer->option("kde-password"));
		m_loginlabel->setText( i18n( "Login: %1" ).arg( login.isEmpty() ? i18n( "<anonymous>" ) : login ) );
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
