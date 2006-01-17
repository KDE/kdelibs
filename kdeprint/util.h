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

#ifndef UTIL_H
#define UTIL_H

#include "kprinter.h"
#include <qstring.h>
#include <kurl.h>
#include <klocale.h>

KUrl smbToUrl(const QString& work, const QString& server, const QString& printer);
void urlToSmb(const KUrl& url, QString& work, QString& server, QString& printer);
// those 2 are only workarounds when the login/password may contain
// strange characters ('@', '/'). In this case, those chars are not encoded
// as other SMB tools (smbspool) doesn't seem to support encoding. This
// utilities allow to continue working KUrl class (and encoding) within
// KDEPrint, but without encoding outside KDEPrint (shoudl fix bug #38733)
KUrl smbToUrl(const QString& s);
QString urlToSmb(const KUrl& url);
KDEPRINT_EXPORT QString buildSmbURI( const QString& work, const QString& server, const QString& printer, const QString& user, const QString& passwd );
KDEPRINT_EXPORT bool splitSmbURI( const QString& uri, QString& work, QString& server, QString& printer, QString& user, QString& passwd );
KDEPRINT_EXPORT QString shadowPassword( const QString& uri );

static const struct pagesizestruct
{
	const char*	text;
	int 	ID;
} page_sizes[] =
{
	{ "A0", KPrinter::A0 },
	{ "A1", KPrinter::A1 },
	{ "A2", KPrinter::A2 },
	{ "A3", KPrinter::A3 },
	{ "A4", KPrinter::A4 },
	{ "A5", KPrinter::A5 },
	{ "A6", KPrinter::A6 },
	{ "A7", KPrinter::A7 },
	{ "A8", KPrinter::A8 },
	{ "A9", KPrinter::A9 },
	{ "B1", KPrinter::B1 },
	{ "B10", KPrinter::B10 },
	{ "B2", KPrinter::B2 },
	{ "B3", KPrinter::B3 },
	{ "B4", KPrinter::B4 },
	{ "B5", KPrinter::B5 },
	{ "B6", KPrinter::B6 },
	{ "B7", KPrinter::B7 },
	{ "B8", KPrinter::B8 },
	{ "B9", KPrinter::B9 },
	{ I18N_NOOP("Envelope C5"), KPrinter::C5E },
	{ I18N_NOOP("Envelope DL"), KPrinter::DLE },
	{ I18N_NOOP("Envelope US #10"), KPrinter::Comm10E },
	{ I18N_NOOP("Executive"), KPrinter::Executive },
	{ I18N_NOOP("Folio"), KPrinter::Folio },
	{ I18N_NOOP("Ledger"), KPrinter::Ledger },
	{ I18N_NOOP("Tabloid"), KPrinter::Tabloid },
	{ I18N_NOOP("US Legal"), KPrinter::Legal },
	{ I18N_NOOP("US Letter"), KPrinter::Letter }
};
int findIndex(int ID);

#endif
