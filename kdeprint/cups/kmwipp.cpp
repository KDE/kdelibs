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

#include "kmwipp.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <klocale.h>
#include <qvalidator.h>

#include <cups/http.h>

KMWIpp::KMWIpp(QWidget *parent)
    : KMWInfoBase(2,parent)
{
	m_ID = KMWizard::IPP;
	m_title = i18n("Remote IPP server");
	m_nextpage = KMWizard::IPPSelect;
	lineEdit( 1 )->setValidator( new QIntValidator( this ) );

	setInfo(i18n("<p>Enter the information concerning the remote IPP server "
		     "owning the targeted printer. This wizard will poll the server "
		     "before continuing.</p>"));
	setLabel(0,i18n("Host:"));
	setLabel(1,i18n("Port:"));
	setText( 1, QLatin1String( "631" ) );
}

bool KMWIpp::isValid(QString& msg)
{
	// check informations
	if (text(0).isEmpty())
	{
		msg = i18n("Empty server name.");
		return false;
	}
	bool	ok(false);
	int	p = text(1).toInt(&ok);
	if (!ok)
	{
		msg = i18n("Incorrect port number.");
		return false;
	}

	// check server
	http_t	*HTTP = httpConnect(qPrintable(text(0)),p);
	if (HTTP)
	{
		httpClose(HTTP);
		return true;
	}
	else
	{
		msg = i18n("<nobr>Unable to connect to <b>%1</b> on port <b>%2</b> .</nobr>").arg(text(0)).arg(p);
		return false;
	}
}

void KMWIpp::updatePrinter(KMPrinter *p)
{
	KUrl	url;
	url.setProtocol("ipp");
	url.setHost(text(0));
	url.setPort(text(1).toInt());
	if (!p->option("kde-login").isEmpty()) url.setUser(p->option("kde-login"));
	if (!p->option("kde-password").isEmpty()) url.setPass(p->option("kde-password"));
	p->setDevice(url.url());
}
