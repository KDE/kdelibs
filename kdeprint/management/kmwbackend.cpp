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

#include "kvisiblebuttongroup.h"
#include "kmwbackend.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <qlayout.h>
#include <qregexp.h>

#include <klocale.h>
#include <kseparator.h>
#include <kdialog.h>
#include <kdebug.h>

//********************************************************************************************************

KMWBackend::KMWBackend(QWidget *parent)
    : KMWizardPage(parent)
{
	m_ID = KMWizard::Backend;
	m_title = i18n("Backend Selection");

	m_buttons = new KVisibleButtonGroup(this);
	m_buttons->setFlat(true);
}

bool KMWBackend::isValid(QString& msg)
{
	if (!m_buttons->checkedButton())
	{
		msg = i18n("You must select a backend.");
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
		s = p->deviceProtocol();
		// extract protocol
		if (s == "parallel" || s == "serial" || s == "usb") ID = KMWizard::Local;
		else if (s == "smb") ID = KMWizard::SMB;
		else if (s == "ipp" || s == "http") ID = KMWizard::IPP;
		else if (s == "lpd") ID = KMWizard::LPD;
		else if (s == "socket") ID = KMWizard::TCP;
		else if (s == "file") ID = KMWizard::File;
		else if (p->members().count() > 0) ID = KMWizard::Class;
	}

	QAbstractButton *btn = m_buttons->button(ID);
	if (btn)
		btn->setChecked(true);
}

void KMWBackend::updatePrinter(KMPrinter *p)
{
	int	ID = m_buttons->checkedId();
	if (ID == KMWizard::Class)
		p->setType(KMPrinter::Class);
	else
		p->setType(KMPrinter::Printer);
	p->setOption("kde-backend",QString::number(ID));
	QString	s = m_buttons->checkedButton()->text();
	s.replace(QRegExp("&(?=\\w)"), QLatin1String(""));
	p->setOption("kde-backend-description",s);
	setNextPage((m_map.contains(ID) ? m_map[ID] : KMWizard::Error));
}

void KMWBackend::addBackend( int ID, bool on, int nextpage )
{
	switch ( ID )
	{
		case KMWizard::Local:
			addBackend( ID, i18n("&Local printer (parallel, serial, USB)"), on,
					i18n( "<qt><p>Locally-connected printer</p>"
						  "<p>Use this for a printer connected "
						  "to the computer via a parallel, serial or USB port.</p></qt>" ),
					nextpage );
			break;
		case KMWizard::SMB:
			addBackend( ID, i18n("&SMB shared printer (Windows)"), on,
					i18n( "<qt><p>Shared Windows printer</p>"
						  "<p>Use this for a printer installed "
						  "on a Windows server and shared on the network using the SMB "
						  "protocol (samba).</p></qt>" ),
					nextpage );
			break;
		case KMWizard::LPD:
			addBackend( ID, i18n("&Remote LPD queue"), on,
					i18n( "<qt><p>Print queue on a remote LPD server</p>"
						  "<p>Use this for a print queue "
						  "existing on a remote machine running a LPD print server.</p></qt>" ),
					nextpage );
			break;
		case KMWizard::TCP:
			addBackend( ID, i18n("Ne&twork printer (TCP)"), on,
					i18n( "<qt><p>Network TCP printer</p>"
						  "<p>Use this for a network-enabled printer "
						  "using TCP (usually on port 9100) as communication protocol. Most "
						  "network printers can use this mode.</p></qt>" ),
					nextpage );
			break;
		case -1:
			addBackend( ID, QString(), on, QString(), nextpage );
			break;
		default:
			kError( 500 ) << "Non standard wizard page ID: " << ID << endl;
	}
}

void KMWBackend::addBackend(int ID, const QString& txt, bool on, const QString& whatsThis, int nextpage)
{
	KRadioButtonWithHandOver *btn = NULL;

	if (ID != -1)
	{
		btn = new KRadioButtonWithHandOver(txt, this);
		btn->setEnabled(on);
		if ( !whatsThis.isEmpty() )
			btn->setWhatsThis(whatsThis );
		m_map[ID] = (nextpage == -1 ? ID : nextpage);	// use nextpage if specified, default to ID
	}
	m_buttons->addButton(btn, ID);
}

void KMWBackend::enableBackend(int ID, bool on)
{
	QAbstractButton *btn = m_buttons->button(ID);
	if (btn)
		btn->setEnabled(on);
}
