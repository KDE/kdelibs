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

#include "kmwname.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qregexp.h>

KMWName::KMWName(QWidget *parent, const char *name)
: KMWInfoBase(3,parent,name)
{
	m_ID = KMWizard::Name;
	m_title = i18n("General Information");
	m_nextpage = KMWizard::End;

	setInfo(i18n("<p>Enter the information concerning your printer or class. <b>Name</b> is mandatory, "
		     "<b>Location</b> and <b>Description</b> are not (they may even not be used on some systems).</p>"));
	setLabel(0,i18n("Name:"));
	setLabel(1,i18n("Location:"));
	setLabel(2,i18n("Description:"));
}

bool KMWName::isValid(QString& msg)
{
	if (text(0).isEmpty())
	{
		msg = i18n("You must supply at least a name!");
		return false;
	}
	else if (text(0).find(QRegExp("\\s")) != -1)
	{
		QString	conv = text(0);
		conv.replace(QRegExp("\\s"), "");
		int result = KMessageBox::warningYesNoCancel(this,
					i18n("This is usually not a good idea to include spaces "
					     "in printer name. This may prevent your printer from "
					     "working correctly. The wizard can strip all spaces "
					     "from the string you entered, resulting in %1. "
					     "What do you want to do?").arg(conv),
					QString::null,
					i18n("Strip"), i18n("Keep"));
		switch (result)
		{
			case KMessageBox::Yes:
				setText(0, conv);
			case KMessageBox::No:
				return true;
			default:
				return false;
		}
	}
	return true;
}

void KMWName::initPrinter(KMPrinter *p)
{
	setText(0,p->printerName());
	setText(1,p->location());
	setText(2,p->description());
	if (text(2).isEmpty())
		if (p->option("kde-driver") == "raw")
			setText(2,i18n("Raw printer"));
		else
			setText(2,p->manufacturer() + " " + p->model());

	setCurrent(0);
}

void KMWName::updatePrinter(KMPrinter *p)
{
	p->setPrinterName(text(0));
	p->setName(text(0));
	p->setLocation(text(1));
	p->setDescription(text(2));
}
