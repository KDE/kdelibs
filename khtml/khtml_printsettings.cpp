/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2002 Michael Goffioul <goffioul@imec.be>
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

#include "khtml_printsettings.h"

#include <klocale.h>
#include <qcheckbox.h>
#include <qlayout.h>

KHTMLPrintSettings::KHTMLPrintSettings(QWidget *parent, const char *name)
: KPrintDialogPage(parent, name)
{
	setTitle(i18n("HTML Settings"));

	m_printfriendly = new QCheckBox(i18n("Printer friendly mode (black text, no background)"), this);
	m_printfriendly->setChecked(true);
	m_printimages = new QCheckBox(i18n("Print images"), this);
	m_printimages->setChecked(true);
	m_printheader = new QCheckBox(i18n("Print header"), this);
	m_printheader->setChecked(true);

	QVBoxLayout	*l0 = new QVBoxLayout(this, 0, 10);
	l0->addWidget(m_printfriendly);
	l0->addWidget(m_printimages);
	l0->addWidget(m_printheader);
	l0->addStretch(1);
}

KHTMLPrintSettings::~KHTMLPrintSettings()
{
}

void KHTMLPrintSettings::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	opts["app-khtml-printfriendly"] = (m_printfriendly->isChecked() ? "true" : "false");
	opts["app-khtml-printimages"] = (m_printimages->isChecked() ? "true" : "false");
	opts["app-khtml-printheader"] = (m_printheader->isChecked() ? "true" : "false");
}

void KHTMLPrintSettings::setOptions(const QMap<QString,QString>& opts)
{
	m_printfriendly->setChecked(opts["app-khtml-printfriendly"] != "false");
	m_printimages->setChecked(opts["app-khtml-printimages"] != "false");
	m_printheader->setChecked(opts["app-khtml-printheader"] != "false");
}

#include "khtml_printsettings.moc"
