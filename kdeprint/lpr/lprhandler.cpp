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

#include "lprhandler.h"
#include "kmprinter.h"
#include "printcapentry.h"

LprHandler::LprHandler(const QString& name)
: m_name(name)
{
}

bool LprHandler::validate(PrintcapEntry*)
{
	return true;
}

KMPrinter* LprHandler::createPrinter(PrintcapEntry *entry)
{
	KMPrinter	*prt = new KMPrinter;
	prt->setPrinterName(entry->name);
	prt->setName(entry->name);
	prt->setType(KMPrinter::Printer|KMPrinter::Invalid);
	return prt;
}

bool LprHandler::completePrinter(KMPrinter*, PrintcapEntry*, bool)
{
	return false;
}
