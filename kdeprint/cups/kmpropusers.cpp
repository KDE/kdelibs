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

#include "kmpropusers.h"
#include "kmprinter.h"
#include "kmwizard.h"

#include <qtextview.h>
#include <qlayout.h>
#include <klocale.h>

KMPropUsers::KMPropUsers(QWidget *parent, const char *name)
: KMPropWidget(parent,name)
{
	m_text = new QTextView(this);
	m_text->setPaper(colorGroup().background());
	m_text->setFrameStyle(QFrame::NoFrame);

	QVBoxLayout	*l0 = new QVBoxLayout(this, 10, 0);
	l0->addWidget(m_text, 1);

	m_title = i18n("Users");
	m_header = i18n("Users Access Settings");
	m_pixmap = "kdeprint_printer_users";
}

KMPropUsers::~KMPropUsers()
{
}

void KMPropUsers::setPrinter(KMPrinter *p)
{
	if (p && p->isPrinter())
	{
		QString	txt("<p>%1:<ul>%1</ul></p>");
		QStringList	users;
		if (!p->option("requesting-user-name-denied").isEmpty())
		{
			txt = txt.arg(i18n("Denied users"));
			users = QStringList::split(",", p->option("requesting-user-name-denied"), false);
			if (users.count() == 1 && users[0] == "none")
				users.clear();
		}
		else if (!p->option("requesting-user-name-allowed").isEmpty())
		{
			txt = txt.arg(i18n("Allowed users"));
			users = QStringList::split(",", p->option("requesting-user-name-allowed"), false);
			if (users.count() == 1 && users[0] == "all")
				users.clear();
		}
		if (users.count() > 0)
		{
			QString	s;
			for (QStringList::ConstIterator it=users.begin(); it!=users.end(); ++it)
				s.append("<li>").append(*it).append("</li>");
			txt = txt.arg(s);
			m_text->setText(txt);
		}
		else
			m_text->setText(i18n("All users allowed"));
		emit enable(true);
		emit enableChange(p->isLocal());
	}
	else
	{
		emit enable(false);
		m_text->setText("");
	}
}

void KMPropUsers::configureWizard(KMWizard *w)
{
	w->configure(KMWizard::Custom+4,KMWizard::Custom+4,true);
}
