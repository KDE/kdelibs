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

#include "kmwend.h"
#include "kmprinter.h"
#include "kmwizard.h"
#include "util.h"

#include <qtextview.h>
#include <klocale.h>
#include <qlayout.h>

KMWEnd::KMWEnd(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::End;
	m_title = i18n("Confirmation");
	m_nextpage = KMWizard::Error;

	m_view = new QTextView(this);

	QVBoxLayout	*lay = new QVBoxLayout(this, 0, 0);
	lay->addWidget(m_view,1);
}

void KMWEnd::initPrinter(KMPrinter *p)
{
	QString	txt;
	QString	s(QString::fromLatin1("<li><u>%1</u>: %2</li>"));
	int	ID = p->option("kde-backend").toInt();

	// general information
	txt.append(QString::fromLatin1("<b>%1</b><ul type=circle>").arg(i18n("General")));
	txt.append(s.arg(i18n("Type")).arg(p->option("kde-backend-description")));
	txt.append(s.arg(i18n("Name")).arg(p->name()));
	txt.append(s.arg(i18n("Location")).arg(p->location()));
	txt.append(s.arg(i18n("Description")).arg(p->description()));
	txt.append("</ul><br>");

	if (ID == KMWizard::Class)
	{
		// class members
		txt.append(QString::fromLatin1("<b>%1</b><ul type=circle>").arg(i18n("Members")));
		QStringList	m(p->members());
		QString		s1(QString::fromLatin1("<li>%1</li>"));
		for (QStringList::ConstIterator it=m.begin(); it!=m.end(); ++it)
			txt.append(s1.arg(*it));
		txt.append("</ul><br>");
	}
	else
	{
		// backend information
		txt.append(QString::fromLatin1("<b>%1</b><ul type=circle>").arg(i18n("Backend")));
		KURL url ( p->device() );
		switch (ID)
		{
			case KMWizard::Local:
				txt.append(s.arg(i18n("Device")).arg(url.path()));
				break;
			case KMWizard::TCP:
				txt.append(s.arg(i18n("Printer IP")).arg(url.host()));
				txt.append(s.arg(i18n("Port")).arg(url.port()));
				break;
			case KMWizard::LPD:
				txt.append(s.arg(i18n("Host")).arg(url.host()));
				txt.append(s.arg(i18n("Queue")).arg(url.path().right(url.path().length()-1)));
				break;
			case KMWizard::File:
				txt.append(s.arg(i18n("File")).arg(url.path()));
				break;
			case KMWizard::IPP:
				txt.append(s.arg(i18n("Host")).arg(url.host()));
				txt.append(s.arg(i18n("Port")).arg(url.port()));
				txt.append(s.arg(i18n("Printer")).arg(url.path().right(url.path().length()-1)));
				if (url.hasUser()) txt.append(s.arg(i18n("Account")).arg(url.user()));
				break;
			default:
				// double decoding in case the printer name contains chars like '#' that are
				// not decoded by "prettyURL".
				txt.append(s.arg(i18n("URI")).arg(shadowPassword( p->device() )));
				break;
		}
		txt.append("</ul><br>");

		if (p->option("kde-driver") == "raw" || p->driver())
		{
			// driver information
			txt.append(QString::fromLatin1("<b>%1</b><ul type=circle>").arg(i18n("Driver")));
			if (p->option("kde-driver") == "raw")
				txt.append(s.arg(i18n("Type")).arg(i18n("Raw printer")));
			else
			{
				txt.append(s.arg(i18n("Type")).arg((p->dbEntry() ? i18n("DB driver") : i18n("External driver"))));
				txt.append(s.arg(i18n("Manufacturer")).arg(p->manufacturer()));
				txt.append(s.arg(i18n("Model")).arg(p->model()));
				txt.append(s.arg(i18n("Description")).arg(p->driverInfo()));
			}
			txt.append("</ul><br>");
		}
	}

	m_view->setText(txt);
}
