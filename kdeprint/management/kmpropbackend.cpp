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

#include "kmpropbackend.h"
#include "kmprinter.h"
#include "kmwizard.h"

#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>

KMPropBackend::KMPropBackend(QWidget *parent, const char *name)
: KMPropWidget(parent,name)
{
	m_uri = new QLabel("",this);
	m_type = new QLabel("",this);

	QLabel	*l1 = new QLabel(i18n("Printer type:"), this);
	QLabel	*l2 = new QLabel(i18n("URI:"), this);

	// layout
	QGridLayout	*main_ = new QGridLayout(this, 3, 2, 10, 7);
	main_->setColStretch(0,0);
	main_->setColStretch(1,1);
	main_->setRowStretch(2,1);
	main_->addWidget(l1,0,0);
	main_->addWidget(l2,1,0);
	main_->addWidget(m_type,0,1);
	main_->addWidget(m_uri,1,1);

	m_pixmap = "connect_established";
	m_title = i18n("Interface");
	m_header = i18n("Interface Settings");
}

KMPropBackend::~KMPropBackend()
{
}

void KMPropBackend::setPrinter(KMPrinter *p)
{
	if (p && p->isPrinter())
	{
		m_uri->setText(p->device().prettyURL());
		QString	prot = p->device().protocol().lower();
		if (prot == "ipp" || prot == "http") m_type->setText(i18n("IPP Printer"));
		else if (prot == "usb") m_type->setText(i18n("Local USB Printer"));
		else if (prot == "parallel") m_type->setText(i18n("Local Parallel Printer"));
		else if (prot == "serial") m_type->setText(i18n("Local Serial Printer"));
		else if (prot == "socket") m_type->setText(i18n("Network Printer (socket)"));
		else if (prot == "smb") m_type->setText(i18n("SMB printers (Windows)"));
		else if (prot == "lpd") m_type->setText(i18n("Remote LPD queue"));
		else if (prot == "file") m_type->setText(i18n("File printer"));
		else if (prot == "fax") m_type->setText(i18n("Serial Fax/Modem printer"));
		else m_type->setText(i18n("Unknown Protocol", "Unknown"));
		emit enable(true);
		emit enableChange(p->isLocal());
	}
	else
	{
		emit enable(false);
		m_type->setText("");
		m_uri->setText("");
	}
}

void KMPropBackend::configureWizard(KMWizard *w)
{
	w->configure(KMWizard::Backend,KMWizard::Driver,false);
}
