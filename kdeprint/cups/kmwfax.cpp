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

#include "kmwfax.h"
#include "kmwizard.h"
#include "kmprinter.h"
#include "ipprequest.h"
#include "cupsinfos.h"

#include <qlabel.h>
#include <qlayout.h>
#include <klistbox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kurl.h>

KMWFax::KMWFax(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::Custom+2;
	m_title = i18n("Fax Serial Device");
	m_nextpage = KMWizard::Driver;

	QLabel	*lab = new QLabel(this);
	lab->setText(i18n("<p>Select the device which your serial Fax/Modem is connected to.</p>"));
	m_list = new KListBox(this);

	QVBoxLayout	*l1 = new QVBoxLayout(this,0,10);
	l1->addWidget(lab,0);
	l1->addWidget(m_list,1);

	// initialize
	IppRequest	req;
	req.setOperation(CUPS_GET_DEVICES);
	QString	uri = QString::fromLatin1("ipp://%1:%2/printers/").arg(CupsInfos::self()->host()).arg(CupsInfos::self()->port());
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	if (req.doRequest("/"))
	{
		ipp_attribute_t	*attr = req.first();
		while (attr)
		{
			if (attr->name && strcmp(attr->name,"device-uri") == 0 && strncmp(attr->values[0].string.text,"fax",3) == 0)
			{
				m_list->insertItem(SmallIcon("blockdevice"),QString::fromLatin1(attr->values[0].string.text));
			}
			attr = attr->next;
		}
	}
}

bool KMWFax::isValid(QString& msg)
{
	if (m_list->currentItem() == -1)
	{
		msg = i18n("You must select a device.");
		return false;
	}
	return true;
}

void KMWFax::updatePrinter(KMPrinter *printer)
{
	QString	uri = m_list->currentText();
	printer->setDevice(KURL(uri));
}
