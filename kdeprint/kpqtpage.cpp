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

#include "kpqtpage.h"
#include "kprinter.h"
#include "kxmlcommand.h"
#include "driver.h"

#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>

#define ORIENT_PORTRAIT_ID	0
#define ORIENT_LANDSCAPE_ID	1

#define COLORMODE_COLOR_ID	0
#define COLORMODE_GRAYSCALE_ID	1

#define NUP_1		0
#define NUP_2		1
#define NUP_4		2
#define NUP_OTHER	3

void radioCursor(QButtonGroup*);

struct pagesizestruct
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

int findIndex(int ID)
{
	for (int i=0; i<KPrinter::NPageSize-1; i++)
		if (page_sizes[i].ID == ID)
			return i;
	return 4;
}

//*****************************************************************************************************

KPQtPage::KPQtPage(QWidget *parent, const char *name)
: KPrintDialogPage(parent,name)
{
	init();
}

KPQtPage::KPQtPage(DrMain *driver, QWidget *parent, const char *name)
: KPrintDialogPage(0, (driver && driver->findOption("PageSize") ? driver : 0), parent, name)
{
	init();
}

KPQtPage::~KPQtPage()
{
}

void KPQtPage::init()
{
	setTitle(i18n("Print Format"));

	// widget creation
	m_pagesize = new QComboBox(this);
	QLabel	*m_pagesizelabel = new QLabel(i18n("Page s&ize:"), this);
	m_pagesizelabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	m_pagesizelabel->setBuddy(m_pagesize);
	m_orientbox = new QButtonGroup(0, Qt::Vertical, i18n("Orientation"), this);
	m_colorbox = new QButtonGroup(0, Qt::Vertical, i18n("Color Mode"), this);
	QRadioButton	*m_portrait = new QRadioButton(i18n("&Portrait"), m_orientbox);
	QRadioButton	*m_landscape = new QRadioButton(i18n("&Landscape"), m_orientbox);
	m_orientpix = new QLabel(m_orientbox);
	m_orientpix->setAlignment(Qt::AlignCenter);
	QRadioButton	*m_color = new QRadioButton(i18n("Colo&r"), m_colorbox);
	QRadioButton	*m_grayscale = new QRadioButton(i18n("&Grayscale"), m_colorbox);
	m_colorpix = new QLabel(m_colorbox);
	m_colorpix->setAlignment(Qt::AlignCenter);
	m_nupbox = new QButtonGroup(0, Qt::Vertical, i18n("Pages per Sheet"), this);
	QRadioButton	*m_nup1 = new QRadioButton("&1", m_nupbox);
	QRadioButton	*m_nup2 = new QRadioButton("&2", m_nupbox);
	QRadioButton	*m_nup4 = new QRadioButton("&4", m_nupbox);
	QRadioButton	*m_nupother = new QRadioButton(i18n("Ot&her"), m_nupbox);
	m_nuppix = new QLabel(m_nupbox);
	m_nuppix->setAlignment(Qt::AlignCenter);

	// layout creation
	QGridLayout	*lay0 = new QGridLayout(this, 3, 2, 0, 10);
	lay0->setRowStretch(1,1);
	lay0->setRowStretch(2,1);
	lay0->addWidget(m_pagesizelabel,0,0);
	lay0->addWidget(m_pagesize,0,1);
	lay0->addWidget(m_orientbox,1,0);
	lay0->addWidget(m_colorbox,1,1);
	lay0->addWidget(m_nupbox,2,0);
	QGridLayout	*lay1 = new QGridLayout(m_orientbox->layout(), 2, 2, 10);
	lay1->addWidget(m_portrait,0,0);
	lay1->addWidget(m_landscape,1,0);
	lay1->addMultiCellWidget(m_orientpix,0,1,1,1);
	QGridLayout	*lay2 = new QGridLayout(m_colorbox->layout(), 2, 2, 10);
	lay2->addWidget(m_color,0,0);
	lay2->addWidget(m_grayscale,1,0);
	lay2->addMultiCellWidget(m_colorpix,0,1,1,1);
	QGridLayout	*lay3 = new QGridLayout(m_nupbox->layout(), 4, 2, 5);
	lay3->addWidget(m_nup1,0,0);
	lay3->addWidget(m_nup2,1,0);
	lay3->addWidget(m_nup4,2,0);
	lay3->addWidget(m_nupother,3,0);
	lay3->addMultiCellWidget(m_nuppix,0,3,1,1);

	// initialization
	radioCursor(m_orientbox);
	radioCursor(m_colorbox);
	radioCursor(m_nupbox);
	m_portrait->setChecked(true);
	slotOrientationChanged(0);
	m_color->setChecked(true);
	slotColorModeChanged(0);
	m_nup1->setChecked(true);
	slotNupChanged(0);

	if (!KXmlCommandManager::self()->checkCommand("psnup"))
		m_nupbox->setEnabled(false);
	if (KPrinter::applicationType() != KPrinter::Dialog)
	{
		m_orientbox->setEnabled(false);
		m_colorbox->setEnabled(false);
		m_pagesize->setEnabled(driver());
		m_pagesizelabel->setEnabled(driver());
	}

	if (!driver())
	{
		for (int i=0; i<KPrinter::NPageSize-1; i++)
			m_pagesize->insertItem(i18n(page_sizes[i].text));
		// default page size to locale settings
		m_pagesize->setCurrentItem(findIndex((KPrinter::PageSize)(KGlobal::locale()->pageSize())));
	}
	else
	{
		DrListOption	*lopt = static_cast<DrListOption*>(driver()->findOption("PageSize"));
		QPtrListIterator<DrBase>	it(*(lopt->choices()));
		for (; it.current(); ++it)
		{
			m_pagesize->insertItem(it.current()->get("text"));
			if (it.current() == lopt->currentChoice())
				m_pagesize->setCurrentItem(m_pagesize->count()-1);
		}
	}

	// connections
	connect(m_orientbox,SIGNAL(clicked(int)),SLOT(slotOrientationChanged(int)));
	connect(m_colorbox,SIGNAL(clicked(int)),SLOT(slotColorModeChanged(int)));
	connect(m_nupbox,SIGNAL(clicked(int)),SLOT(slotNupChanged(int)));
}

void KPQtPage::slotOrientationChanged(int ID)
{
	m_orientpix->setPixmap(UserIcon((ID == ORIENT_PORTRAIT_ID ? "kdeprint_portrait" : "kdeprint_landscape")));
}

void KPQtPage::slotColorModeChanged(int ID)
{
	m_colorpix->setPixmap(UserIcon((ID == COLORMODE_COLOR_ID ? "kdeprint_color" : "kdeprint_grayscale")));
}

void KPQtPage::slotNupChanged(int ID)
{
	QString	pixstr;
	switch (ID)
	{
		case NUP_1: pixstr = "kdeprint_nup1"; break;
		case NUP_2: pixstr = "kdeprint_nup2"; break;
		case NUP_4: pixstr = "kdeprint_nup4"; break;
		case NUP_OTHER: pixstr = "kdeprint_nupother"; break;
	}
	m_nuppix->setPixmap(UserIcon(pixstr));
}

void KPQtPage::setOptions(const QMap<QString,QString>& opts)
{
	int 	ID = (opts["kde-orientation"] == "Landscape" ? ORIENT_LANDSCAPE_ID : ORIENT_PORTRAIT_ID);
	m_orientbox->setButton(ID);
	slotOrientationChanged(ID);
	ID = (opts["kde-colormode"] == "GrayScale" ? COLORMODE_GRAYSCALE_ID : COLORMODE_COLOR_ID);
	m_colorbox->setButton(ID);
	slotColorModeChanged(ID);
	if (driver())
	{
		QString	val = opts["PageSize"];
		if (!val.isEmpty())
		{
			DrListOption	*opt = static_cast<DrListOption*>(driver()->findOption("PageSize"));
			DrBase	*ch = opt->findChoice(val);
			if (ch)
				m_pagesize->setCurrentItem(opt->choices()->findRef(ch));
		}
	}
	else if (!opts["kde-pagesize"].isEmpty())
		m_pagesize->setCurrentItem(findIndex(opts["kde-pagesize"].toInt()));
	ID = NUP_1;
	if (opts["_kde-filters"].find("psnup") != -1)
	{
		ID = opts["_kde-psnup-nup"].toInt();
		if (ID == 1 || ID == 2 || ID == 4)
		{
			if (ID == 4) ID = 3;
			ID--;
		}
		else
		{
			ID = NUP_OTHER;
		}
	}
	m_nupbox->setButton(ID);
	slotNupChanged(ID);
}

void KPQtPage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	opts["kde-orientation"] = (m_orientbox->id(m_orientbox->selected()) == ORIENT_LANDSCAPE_ID ? "Landscape" : "Portrait");
	opts["kde-colormode"] = (m_colorbox->id(m_colorbox->selected()) == COLORMODE_GRAYSCALE_ID ? "GrayScale" : "Color");
	if (driver())
	{
		DrListOption	*opt = static_cast<DrListOption*>(driver()->findOption("PageSize"));
		if (opt)
		{
			DrBase	*ch = opt->choices()->at(m_pagesize->currentItem());
			if (ch && (incldef || ch->name() != opt->get("default")))
			{
				opts["PageSize"] = ch->name();
			}
		}
	}
	else
		opts["kde-pagesize"] = QString::number(page_sizes[m_pagesize->currentItem()].ID);
	int	ID = m_nupbox->id(m_nupbox->selected());
	QString	s = opts["_kde-filters"];
	if (ID == NUP_1)
	{
		opts.remove("_kde-psnup-nup");
	}
	else if (ID != NUP_OTHER)
	{
		int	nup(ID == NUP_2 ? 2 : 4);
		if (s.find("psnup") == -1)
		{
			QStringList	fl = QStringList::split(',', s, false);
			KXmlCommandManager::self()->insertCommand(fl, "psnup");
			s = fl.join(",");
		}
		opts["_kde-psnup-nup"] = QString::number(nup);
	}
	opts["_kde-filters"] = s;
}
#include "kpqtpage.moc"
