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

#include "kmwdrivertest.h"
#include "kmprinter.h"
#include "kmwizard.h"
#include "driver.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "kmdriverdialog.h"

#include <qlabel.h>
#include <kpushbutton.h>
#include <qlayout.h>
#include <klocale.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kguiitem.h>

KMWDriverTest::KMWDriverTest(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::DriverTest;
	m_title = i18n("Printer Test");
	m_nextpage = KMWizard::Name;
	m_driver = 0;
	m_printer = 0;

	m_manufacturer = new QLabel(this);
	m_model = new QLabel(this);
	m_driverinfo = new QLabel(this);
	m_driverinfo->setTextFormat(Qt::RichText);
	QLabel	*l1 = new QLabel(i18n("<b>Manufacturer:</b>"), this);
	QLabel	*l2 = new QLabel(i18n("<b>Model:</b>"), this);
	QLabel	*l3 = new QLabel(i18n("<b>Description:</b>"), this);

	m_test = new KPushButton(KGuiItem(i18n("&Test"), "kdeprint_testprinter"), this);
	m_settings = new KPushButton(KGuiItem(i18n("&Settings..."), "configure"), this);

	QLabel	*l0 = new QLabel(this);
	l0->setText(i18n("<p>Now you can test the printer before finishing installation. "
			 "Use the <b>Settings</b> button to configure the printer driver and "
			 "the <b>Test</b> button to test your configuration. Use the <b>Back</b> "
			 "button to change the driver (your current configuration will be discarded).</p>"));

	QVBoxLayout	*lay1 = new QVBoxLayout(this, 0, 15);
	QGridLayout	*lay2 = new QGridLayout(0, 3, 3, 0, 0);
	QHBoxLayout	*lay3 = new QHBoxLayout(0, 0, 10);
	lay1->addWidget(l0,0);
	lay1->addLayout(lay2,0);
	lay1->addLayout(lay3,0);
	lay1->addStretch(1);
	lay2->setColStretch(2,1);
	lay2->addColSpacing(1,10);
	lay2->addWidget(l1,0,0);
	lay2->addWidget(l2,1,0);
	lay2->addWidget(l3,2,0,Qt::AlignLeft|Qt::AlignTop);
	lay2->addWidget(m_manufacturer,0,2);
	lay2->addWidget(m_model,1,2);
	lay2->addWidget(m_driverinfo,2,2);
	lay3->addWidget(m_test,0);
	lay3->addWidget(m_settings,0);
	lay3->addStretch(1);

	connect(m_test,SIGNAL(clicked()),SLOT(slotTest()));
	connect(m_settings,SIGNAL(clicked()),SLOT(slotSettings()));
}

KMWDriverTest::~KMWDriverTest()
{
	delete m_driver;
}

void KMWDriverTest::initPrinter(KMPrinter *p)
{
	m_manufacturer->setText(p->manufacturer());
	m_model->setText(p->model());
	m_driverinfo->setText(p->driverInfo());
	m_printer = p;

	delete m_driver;
	m_driver = 0;

	QString	drfile = p->option("kde-driver");
	bool	checkDriver(true);
	if (!drfile.isEmpty() && drfile != "raw")
		m_driver = KMFactory::self()->manager()->loadFileDriver(drfile);
	else if (p->dbEntry() != NULL)
		m_driver = KMFactory::self()->manager()->loadDbDriver(p->dbEntry());
	else
		checkDriver = false;

	if (checkDriver && !m_driver)
	{
		KMessageBox::error(this, i18n("<qt>Unable to load the requested driver:<p>%1</p></qt>").arg(KMManager::self()->errorMsg()));
		KMManager::self()->setErrorMsg(QString::null);
	}
	m_settings->setEnabled((m_driver != 0));
}

void KMWDriverTest::updatePrinter(KMPrinter *p)
{
	// Give the DrMain structure to the driver and don't care about it anymore.
	// It will be destroyed either when giving another structure, or when the
	// printer object will be destroyed.
	p->setDriver(m_driver);
	m_driver = 0;
}

void KMWDriverTest::slotTest()
{
	if (!m_printer) return;

	QString	name = "tmpprinter_"+KApplication::randomString(8);
	// save printer name (can be non empty when modifying a printer)
	QString	oldname = m_printer->name();

	m_printer->setName(name);
	m_printer->setPrinterName(name);
	m_printer->setDriver(m_driver);
	if (KMFactory::self()->manager()->createPrinter(m_printer))
	{
		if (KMFactory::self()->manager()->testPrinter(m_printer))
			KMessageBox::information(this,i18n("Test page successfully sent to printer. Wait until printing is complete, then click the OK button."));
		else
			KMessageBox::error(this,"<qt>"+i18n("Unable to test printer: ")+KMFactory::self()->manager()->errorMsg()+"</qt>");
		if (!KMFactory::self()->manager()->removePrinter(m_printer))
			KMessageBox::error(this,i18n("Unable to remove temporary printer."));
	}
	else
		KMessageBox::error(this,i18n("Unable to create temporary printer."));

	// restoring old name
	m_printer->setName(oldname);
	m_printer->setPrinterName(oldname);

	m_driver = m_printer->takeDriver();
}

void KMWDriverTest::slotSettings()
{
	if (m_driver)
	{
		KMDriverDialog	dlg(this);
		dlg.setDriver(m_driver);
		dlg.showButtonCancel(false);	// only OK button
		dlg.exec();
	}
}
#include "kmwdrivertest.moc"
