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

#include "kmwbackend.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <qlayout.h>
#include <qregexp.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include <kcursor.h>
#include <klocale.h>
#include <kseparator.h>

class KRadioButton : public QRadioButton
{
public:
	KRadioButton(const QString& txt, QWidget *parent = 0, const char *name = 0);
};

KRadioButton::KRadioButton(const QString& txt, QWidget *parent, const char *name)
: QRadioButton(txt,parent,name)
{
	setCursor(KCursor::handCursor());
}

//********************************************************************************************************

KMWBackend::KMWBackend(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::Backend;
	m_title = i18n("Backend selection");

	m_buttons = new QButtonGroup(this);
	m_buttons->hide();

	m_layout = new QVBoxLayout(this, 10, 10);
	m_layout->addStretch(1);
	m_count = 0;
}

bool KMWBackend::isValid(QString& msg)
{
	if (!m_buttons->selected())
	{
		msg = i18n("You must select a backend!");
		return false;
	}
	return true;
}

void KMWBackend::initPrinter(KMPrinter *p)
{
	QString	s = p->option("kde-backend");
	int	ID(-1);

	if (!s.isEmpty())
		ID = s.toInt();
	else
	{
		s = p->device().protocol();
		// extract protocol
		if (s == "parallel" || s == "serial" || s == "usb") ID = KMWizard::Local;
		else if (s == "smb") ID = KMWizard::SMB;
		else if (s == "ipp" || s == "http") ID = KMWizard::IPP;
		else if (s == "lpd") ID = KMWizard::LPD;
		else if (s == "socket") ID = KMWizard::TCP;
		else if (s == "file") ID = KMWizard::File;
		else if (p->members().count() > 0) ID = KMWizard::Class;
	}

	if (m_buttons->find(ID))
		m_buttons->setButton(ID);
}

void KMWBackend::updatePrinter(KMPrinter *p)
{
	int	ID = m_buttons->id(m_buttons->selected());
	if (ID == KMWizard::Class) p->setType(KMPrinter::Class);
	else p->setType(KMPrinter::Printer);
	p->setOption("kde-backend",QString::number(ID));
	QString	s = m_buttons->selected()->text();
	s.replace(QRegExp("&(?=\\w)"), QString::fromLatin1(""));
	p->setOption("kde-backend-description",s);
	setNextPage((m_map.contains(ID) ? m_map[ID] : KMWizard::Error));
}

void KMWBackend::addBackend(int ID, const QString& txt, bool on, int nextpage)
{
	if (ID == -1)
	{
		KSeparator* sep = new KSeparator( KSeparator::HLine, this);
		m_layout->insertWidget(m_count, sep);
	}
	else
	{
		KRadioButton	*btn = new KRadioButton(txt, this);
		btn->setEnabled(on);
		m_buttons->insert(btn, ID);
		m_map[ID] = (nextpage == -1 ? ID : nextpage);	// use nextpage if specified, default to ID
		m_layout->insertWidget(m_count, btn);
	}
	m_count++;
}

void KMWBackend::enableBackend(int ID, bool on)
{
	QButton	*btn = m_buttons->find(ID);
	if (btn)
		btn->setEnabled(on);
}
