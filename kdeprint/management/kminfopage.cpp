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

#include "kminfopage.h"
#include "kmprinter.h"

#include <qpixmap.h>
#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kseparator.h>

KMInfoPage::KMInfoPage(QWidget *parent)
    : QWidget(parent)
{
	m_title = new QLabel(this);
	m_titlepixmap = new QLabel(this);
	m_titlepixmap->setAlignment(Qt::AlignCenter);
        KSeparator* sep = new KSeparator( Qt::Horizontal, this);
	m_type = new QLabel(this);
	m_description = new QLabel(this);
	m_state = new QLabel(this);
	m_location = new QLabel(this);
	m_model = new QLabel(this);
	m_uri = new QLabel(this);
	m_device = new QLabel(this);
	QLabel	*m_loclabel = new QLabel(i18n("Physical Location", "Location:"), this);
	m_loclabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel	*m_desclabel = new QLabel(i18n("Description:"), this);
	m_desclabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel	*m_typelabel = new QLabel(i18n("Type:"), this);
	m_typelabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel	*m_statelabel = new QLabel(i18n("Status", "State:"), this);
	m_statelabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel	*m_urilabel = new QLabel(i18n("URI:"), this);
	m_urilabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_devlabel = new QLabel(i18n("Device:"), this);
	m_devlabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel	*m_modellabel = new QLabel(i18n("Model:"), this);
	m_modellabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	QGridLayout	*lay0 = new QGridLayout(this, 11, 2, 0, 5);
	QHBoxLayout	*lay1 = new QHBoxLayout(0, 0, 10);
	lay0->addRowSpacing(7,20);
	lay0->setRowStretch(7,0);
	lay0->setRowStretch(10,1);
	lay0->setColStretch(1,1);
	lay0->addMultiCellLayout(lay1, 0, 0, 0, 1);
	lay0->addMultiCellWidget(sep, 1, 1, 0, 1);
	lay0->addWidget(m_typelabel, 2, 0);
	lay0->addWidget(m_statelabel, 3, 0);
	lay0->addWidget(m_loclabel, 4, 0);
	lay0->addWidget(m_desclabel, 5, 0);
	lay0->addWidget(m_urilabel, 6, 0);
	lay0->addWidget(m_devlabel, 8, 0);
	lay0->addWidget(m_modellabel, 9, 0);
	lay0->addWidget(m_type, 2, 1);
	lay0->addWidget(m_state, 3, 1);
	lay0->addWidget(m_location, 4, 1);
	lay0->addWidget(m_description, 5, 1);
	lay0->addWidget(m_uri, 6, 1);
	lay0->addWidget(m_device, 8, 1);
	lay0->addWidget(m_model, 9, 1);
	lay1->addWidget(m_title, 1);
	lay1->addWidget(m_titlepixmap, 0);
}

KMInfoPage::~KMInfoPage()
{
}

void KMInfoPage::setPrinter(KMPrinter *p)
{
	// change "Device" label if it's a real class (not true for implicit classes)
	if (p && p->isClass(false)) m_devlabel->setText(i18n("Members:"));
	else m_devlabel->setText(i18n("Device:"));

	// clear everything if p == 0
	if (!p)
	{
		m_title->setText(QString());
		m_titlepixmap->setPixmap(QPixmap());
		m_type->setText(QString());
		m_state->setText(QString());
		m_location->setText(QString());
		m_description->setText(QString());
		m_uri->setText(QString());
		m_device->setText(QString());
		m_model->setText(QString());
	}
	else
	{
		m_title->setText(QString::fromLatin1("<b>%1</b>").arg(p->name()));
		m_titlepixmap->setPixmap(SmallIcon(p->pixmap()));
		if (p->isImplicit()) m_type->setText(i18n("Implicit class"));
		else if (p->isClass(false))
			if (p->isRemote()) m_type->setText(i18n("Remote class"));
			else m_type->setText(i18n("Local class"));
		else if (p->isPrinter())
			if (p->isRemote()) m_type->setText(i18n("Remote printer"));
			else m_type->setText(i18n("Local printer"));
		else if (p->isSpecial())
			m_type->setText(i18n("Special (pseudo) printer"));
		else m_type->setText(i18n("Unknown class of printer", "Unknown"));
		m_state->setText(p->stateString());
		m_location->setText(p->location());
		m_description->setText(p->description());
		m_uri->setText(p->uri().prettyURL());
		if (p->isClass(false))
		{
			QString	s;
			for (QStringList::ConstIterator it=p->members().begin(); it!=p->members().end(); ++it)
				s.append((*it) + ", ");
			s.truncate(s.length()-2);
			m_device->setText(s);
		}
		else
		{
			// double decoding in case printer name contains
			// annoying chars like '#'
			m_device->setText(KUrl::decode_string(p->device()));
		}
		m_model->setText(p->driverInfo());
	}
}
