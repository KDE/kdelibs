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

#include "kmpropgeneral.h"
#include "kmprinter.h"
#include "kmwizard.h"
#include "kmfactory.h"
#include "kmmanager.h"

#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>

KMPropGeneral::KMPropGeneral(QWidget *parent, const char *name)
: KMPropWidget(parent,name)
{
	m_name = new QLabel("",this);
	m_location = new QLabel("",this);
	m_description = new QLabel("",this);

	QLabel	*l1 = new QLabel(i18n("Printer name:"), this);
	QLabel	*l2 = new QLabel(i18n("Location:"), this);
	QLabel	*l3 = new QLabel(i18n("Description:"), this);

	// layout
	QGridLayout	*main_ = new QGridLayout(this, 4, 2, 10, 7);
	main_->setColStretch(0,0);
	main_->setColStretch(1,1);
	main_->setRowStretch(3,1);
	main_->addWidget(l1,0,0);
	main_->addWidget(l2,1,0);
	main_->addWidget(l3,2,0);
	main_->addWidget(m_name,0,1);
	main_->addWidget(m_location,1,1);
	main_->addWidget(m_description,2,1);

	m_pixmap = "contents";
	m_title = i18n("General");
	m_header = i18n("General Settings");
}

KMPropGeneral::~KMPropGeneral()
{
}

void KMPropGeneral::setPrinter(KMPrinter *p)
{
	if (p)
	{
		m_name->setText(p->name());
		m_location->setText(p->location());
		m_description->setText(p->description());
		emit enableChange(!(p->isSpecial() || p->isRemote() || p->isImplicit()));
	}
	else
	{
		emit enableChange(false);
		m_name->setText("");
		m_location->setText("");
		m_description->setText("");
	}
}

void KMPropGeneral::configureWizard(KMWizard *w)
{
	w->configure(KMWizard::Name,KMWizard::Name,true);
}
