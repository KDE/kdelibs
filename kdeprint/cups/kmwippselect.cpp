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

#include "kmwippselect.h"
#include "kmwizard.h"
#include "kmprinter.h"
#include "cupsinfos.h"
#include "ipprequest.h"

#include <klistbox.h>
#include <qlayout.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

KMWIppSelect::KMWIppSelect(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::IPPSelect;
	m_title = i18n("Remote IPP printer selection");
	m_nextpage = KMWizard::Driver;

	m_list = new KListBox(this);

	QVBoxLayout	*lay = new QVBoxLayout(this, 0, 0);
	lay->addWidget(m_list);
}

bool KMWIppSelect::isValid(QString& msg)
{
	if (m_list->currentItem() == -1)
	{
		msg = i18n("You must select a printer!");
		return false;
	}
	return true;
}

void KMWIppSelect::initPrinter(KMPrinter *p)
{
	// storage variables
	QString	host, login, password;
	int	port;

	// save config
	host = CupsInfos::self()->host();
	login = CupsInfos::self()->login();
	password = CupsInfos::self()->password();
	port = CupsInfos::self()->port();

	m_list->clear();

	// retrieve printer list
	KURL	url = p->device();
	CupsInfos::self()->setHost(url.host());
	CupsInfos::self()->setLogin(url.user());
	CupsInfos::self()->setPassword(url.pass());
	CupsInfos::self()->setPort(url.port());
	IppRequest	req;
	QString		uri;
	req.setOperation(CUPS_GET_PRINTERS);
	uri = QString::fromLatin1("ipp://%1:%2/printers/").arg(url.host()).arg(url.port());
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",QString::fromLatin1("printer-name"));
	if (req.doRequest("/printers/"))
	{
		ipp_attribute_t	*attr = req.first();
		while (attr)
		{
			if (attr->name && strcmp(attr->name,"printer-name") == 0)
				m_list->insertItem(SmallIcon("kdeprint_printer"),QString::fromLatin1(attr->values[0].string.text));
			attr = attr->next;
		}
		m_list->sort();
	}

	// restore config
	CupsInfos::self()->setHost(host);
	CupsInfos::self()->setLogin(login);
	CupsInfos::self()->setPassword(password);
	CupsInfos::self()->setPort(port);
}

void KMWIppSelect::updatePrinter(KMPrinter *p)
{
	KURL	url = p->device();
	QString	path = m_list->currentText();
	path.prepend("/printers/");
	url.setPath(path);
	p->setDevice(url);
kdDebug() << url.url() << endl;
}
