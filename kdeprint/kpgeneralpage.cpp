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

#include "kpgeneralpage.h"
#include "driver.h"
#include "kmprinter.h"
#include "kprinter.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qradiobutton.h>

#include <kcursor.h>
#include <klocale.h>
#include <kiconloader.h>

// Some ID's
#define ORIENT_PORTRAIT_ID	0
#define ORIENT_LANDSCAPE_ID	1
#define ORIENT_REVLANDSCAPE_ID	2
#define ORIENT_REVPORTRAIT_ID	3

#define DUPLEX_NONE_ID	0
#define DUPLEX_LONG_ID	1
#define DUPLEX_SHORT_ID	2

#define NUP_1_ID	0
#define NUP_2_ID	1
#define NUP_4_ID	2

void radioCursor(QButtonGroup *bg)
{
	QButton	*btn;
	int	index(0);
	while ((btn=bg->find(index++)) != 0)
		btn->setCursor(KCursor::handCursor());
}

void initCombo(QComboBox *cb, DrListOption *opt)
{
	cb->clear();
	QPtrListIterator<DrBase>	it(*(opt->choices()));
	for (;it.current();++it)
	{
		cb->insertItem(it.current()->get("text"));
		if (it.current() == opt->currentChoice())
			cb->setCurrentItem(cb->count()-1);
	}
}

void setComboItem(QComboBox *cb, const QString& txt)
{
	for (int i=0;i<cb->count();i++)
		if (cb->text(i) == txt)
		{
			cb->setCurrentItem(i);
			return;
		}
}

int findOption(const char *strs[], int n, const QString& txt)
{
	for (int i=0;i<n;i+=2)
		if (txt == strs[i]) return (i/2);
	return (-1);
}

//*************************************************************************************************

// default values in case of NULL driver
#define	DEFAULT_SIZE	16
static const char*	default_size[] = {
	"A4", I18N_NOOP("ISO A4"),
	"Letter", I18N_NOOP("US Letter"),
	"Legal", I18N_NOOP("US Legal"),
	"Ledger", I18N_NOOP("Ledger"),
	"Tabloid", I18N_NOOP("Tabloid"),
	"Folio", I18N_NOOP("Folio"),
	"Comm10", I18N_NOOP("US #10 Envelope"),
	"DL", I18N_NOOP("ISO DL Envelope")
};
#define	DEFAULT_SOURCE	8
static const char*	default_source[] = {
	"Upper", I18N_NOOP("Upper Tray"),
	"Lower", I18N_NOOP("Lower Tray"),
	"MultiPurpose", I18N_NOOP("Multi-Purpose Tray"),
	"LargeCapacity", I18N_NOOP("Large Capacity Tray")
};
#define	DEFAULT_TYPE	4
static const char*	default_type[] = {
	"Normal", I18N_NOOP("Normal"),
	"Transparency", I18N_NOOP("Transparency")
};

//***************************************************************************************************

KPGeneralPage::KPGeneralPage(KMPrinter *pr, DrMain *dr, QWidget *parent, const char *name)
: KPrintDialogPage(pr,dr,parent,name)
{
	setTitle(i18n("General"));

	// widget creation
	QLabel	*m_pagesizelabel = new QLabel(i18n("Page s&ize:"), this);
	m_pagesizelabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	QLabel	*m_papertypelabel = new QLabel(i18n("Paper t&ype:"), this);
	m_papertypelabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	QLabel	*m_inputslotlabel = new QLabel(i18n("Paper so&urce:"), this);
	m_inputslotlabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
	m_pagesize = new QComboBox(this);
	m_papertype = new QComboBox(this);
	m_inputslot = new QComboBox(this);
	m_pagesizelabel->setBuddy(m_pagesize);
	m_papertypelabel->setBuddy(m_papertype);
	m_inputslotlabel->setBuddy(m_inputslot);
	m_orientbox = new QButtonGroup(0, Qt::Vertical, i18n("Orientation"), this);
	m_duplexbox = new QButtonGroup(0, Qt::Vertical, i18n("Duplex Printing"), this);
	m_nupbox = new QButtonGroup(0, Qt::Vertical, i18n("Pages per Sheet"), this);
	m_bannerbox = new QGroupBox(0, Qt::Vertical, i18n("Banners"), this);
	QRadioButton	*m_portrait = new QRadioButton(i18n("&Portrait"), m_orientbox);
	QRadioButton	*m_landscape = new QRadioButton(i18n("&Landscape"), m_orientbox);
	QRadioButton	*m_revland = new QRadioButton(i18n("&Reverse landscape"), m_orientbox);
	QRadioButton	*m_revport = new QRadioButton(i18n("R&everse portrait"), m_orientbox);
	m_portrait->setChecked(true);
	m_orientpix = new QLabel(m_orientbox);
	m_orientpix->setAlignment(Qt::AlignCenter);
	QRadioButton	*m_dupnone = new QRadioButton(i18n("duplex orientation", "&None"), m_duplexbox);
	QRadioButton	*m_duplong = new QRadioButton(i18n("duplex orientation", "Lon&g side"), m_duplexbox);
	QRadioButton	*m_dupshort = new QRadioButton(i18n("duplex orientation", "S&hort side"), m_duplexbox);
	m_dupnone->setChecked(true);
	m_duplexpix = new QLabel(m_duplexbox);
	m_duplexpix->setAlignment(Qt::AlignCenter);
	QRadioButton	*m_nup1 = new QRadioButton("&1", m_nupbox);
	QRadioButton	*m_nup2 = new QRadioButton("&2", m_nupbox);
	QRadioButton	*m_nup4 = new QRadioButton("&4", m_nupbox);
	m_nup1->setChecked(true);
	m_nuppix = new QLabel(m_nupbox);
	m_nuppix->setAlignment(Qt::AlignCenter);
	m_startbanner = new QComboBox(m_bannerbox);
	m_endbanner = new QComboBox(m_bannerbox);
	QLabel	*m_startbannerlabel = new QLabel(i18n("S&tart:"), m_bannerbox);
	QLabel	*m_endbannerlabel = new QLabel(i18n("En&d:"), m_bannerbox);
	m_startbannerlabel->setBuddy(m_startbanner);
	m_endbannerlabel->setBuddy(m_endbanner);

	// layout creation
	QVBoxLayout	*lay0 = new QVBoxLayout(this, 0, 10);
	QGridLayout	*lay1 = new QGridLayout(0, 3, 2, 0, 10);
	QGridLayout	*lay2 = new QGridLayout(0, 2, 2, 0, 10);
	lay0->addStretch(1);
	lay0->addLayout(lay1);
	lay0->addStretch(1);
	lay0->addLayout(lay2);
	lay1->addWidget(m_pagesizelabel, 0, 0);
	lay1->addWidget(m_papertypelabel, 1, 0);
	lay1->addWidget(m_inputslotlabel, 2, 0);
	lay1->addWidget(m_pagesize, 0, 1);
	lay1->addWidget(m_papertype, 1, 1);
	lay1->addWidget(m_inputslot, 2, 1);
	lay2->addWidget(m_orientbox, 0, 0);
	lay2->addWidget(m_bannerbox, 1, 0);
	lay2->addWidget(m_duplexbox, 0, 1);
	lay2->addWidget(m_nupbox, 1, 1);
	lay2->setColStretch(0, 1);
	lay2->setColStretch(1, 1);
	QGridLayout	*lay3 = new QGridLayout(m_orientbox->layout(), 4, 2, 5);
	lay3->addWidget(m_portrait, 0, 0);
	lay3->addWidget(m_landscape, 1, 0);
	lay3->addWidget(m_revland, 2, 0);
	lay3->addWidget(m_revport, 3, 0);
	lay3->addMultiCellWidget(m_orientpix, 0, 3, 1, 1);
	QGridLayout	*lay4 = new QGridLayout(m_duplexbox->layout(), 3, 2, 5);
	lay4->addWidget(m_dupnone, 0, 0);
	lay4->addWidget(m_duplong, 1, 0);
	lay4->addWidget(m_dupshort, 2, 0);
	lay4->addMultiCellWidget(m_duplexpix, 0, 2, 1, 1);
	lay4->setRowStretch( 0, 1 );
	QGridLayout	*lay5 = new QGridLayout(m_nupbox->layout(), 3, 2, 5);
	lay5->addWidget(m_nup1, 0, 0);
	lay5->addWidget(m_nup2, 1, 0);
	lay5->addWidget(m_nup4, 2, 0);
	lay5->addMultiCellWidget(m_nuppix, 0, 2, 1, 1);
	QGridLayout	*lay6 = new QGridLayout(m_bannerbox->layout(), 2, 2, 5);
	lay6->addWidget(m_startbannerlabel, 0, 0);
	lay6->addWidget(m_endbannerlabel, 1, 0);
	lay6->addWidget(m_startbanner, 0, 1);
	lay6->addWidget(m_endbanner, 1, 1);
	lay6->setColStretch(1, 1);

	// connections (+ misc)
	connect(m_orientbox,SIGNAL(clicked(int)),SLOT(slotOrientationChanged(int)));
	connect(m_nupbox,SIGNAL(clicked(int)),SLOT(slotNupChanged(int)));
	connect(m_duplexbox,SIGNAL(clicked(int)),SLOT(slotDuplexChanged(int)));
	radioCursor(m_orientbox);
	radioCursor(m_nupbox);
	radioCursor(m_duplexbox);

	// init
	initialize();
}

KPGeneralPage::~KPGeneralPage()
{
}

void KPGeneralPage::initialize()
{
	if (driver())
	{
		// Size, Type and Source
		DrListOption	*opt = (DrListOption*)driver()->findOption("PageSize");
		if (opt) initCombo(m_pagesize,opt);
		else m_pagesize->setEnabled(false);
		opt = (DrListOption*)driver()->findOption("MediaType");
		if (opt) initCombo(m_papertype,opt);
		else m_papertype->setEnabled(false);
		opt = (DrListOption*)driver()->findOption("InputSlot");
		if (opt) initCombo(m_inputslot,opt);
		else m_inputslot->setEnabled(false);
		// Duplex
		opt = (DrListOption*)driver()->findOption("Duplex");
		if (opt)
		{
			if ( opt->choices()->count() == 2 )
			{
				// probably a On/Off option instead of the standard PS one
				QButton *btn = m_duplexbox->find( DUPLEX_SHORT_ID );
				m_duplexbox->remove( btn );
				btn->hide();
				//delete btn;
				m_duplexbox->find( DUPLEX_NONE_ID )->setText( i18n( "Disabled" ) );
				m_duplexbox->find( DUPLEX_LONG_ID )->setText( i18n( "Enabled" ) );
				m_duplexpix->hide();
			}
			if (opt->currentChoice())
			{
				int	ID(DUPLEX_NONE_ID);
				if (opt->currentChoice()->name() == "DuplexNoTumble" || opt->currentChoice()->name() == "On") ID = DUPLEX_LONG_ID;
				else if (opt->currentChoice()->name() == "DuplexTumble") ID = DUPLEX_SHORT_ID;
				m_duplexbox->setButton(ID);
				slotDuplexChanged(ID);
			}
		}
		else m_duplexbox->setEnabled(false);
	}
	else
	{
		// PageSize
		for (int i=1;i<DEFAULT_SIZE;i+=2)
			m_pagesize->insertItem(i18n(default_size[i]));
		// set default page size using locale settings
		QString	psname = pageSizeToPageName((KPrinter::PageSize)(KGlobal::locale()->pageSize()));
		int index = findOption(default_size, DEFAULT_SIZE, psname);
		if (index >= 0)
			m_pagesize->setCurrentItem(index);
		// MediaType
		for (int i=1;i<DEFAULT_TYPE;i+=2)
			m_papertype->insertItem(i18n(default_type[i]));
		// PageSize
		for (int i=1;i<DEFAULT_SOURCE;i+=2)
			m_inputslot->insertItem(i18n(default_source[i]));
	}

	// Banners
	QStringList	values = QStringList::split(',',printer()->option("kde-banners-supported"),false);
	if (values.count() > 0)
	{
		for (QStringList::ConstIterator it = values.begin(); it != values.end(); ++it)
		{
			m_startbanner->insertItem(*it);
			m_endbanner->insertItem(*it);
		}
		values = QStringList::split(',',printer()->option("kde-banners"),false);
		while (values.count() < 2) values.append("none");
		setComboItem(m_startbanner, values[0]);
		setComboItem(m_endbanner, values[1]);
	}
	else
		m_bannerbox->setEnabled(false);


	slotOrientationChanged(ORIENT_PORTRAIT_ID);
	slotNupChanged(NUP_1_ID);
}

void KPGeneralPage::setOptions(const QMap<QString,QString>& opts)
{
	QString	value;

	if (driver())
	{
		value = opts["PageSize"];
		if (m_pagesize->isEnabled() && !value.isEmpty())
		{
			DrBase	*ch = ((DrListOption*)driver()->findOption("PageSize"))->findChoice(value);
			if (ch) setComboItem(m_pagesize, ch->get("text"));
		}
		value = opts["MediaType"];
		if (m_papertype->isEnabled() && !value.isEmpty())
		{
			DrBase	*ch = ((DrListOption*)driver()->findOption("MediaType"))->findChoice(value);
			if (ch) setComboItem(m_papertype, ch->get("text"));
		}
		value = opts["InputSlot"];
		if (m_inputslot->isEnabled() && !value.isEmpty())
		{
			DrBase	*ch = ((DrListOption*)driver()->findOption("InputSlot"))->findChoice(value);
			if (ch) setComboItem(m_inputslot, ch->get("text"));
		}

		value = opts["Duplex"];
		if (m_duplexbox->isEnabled() && !value.isEmpty())
		{
			int	ID(0);
			if (value == "DuplexNoTumble" || value == "On") ID = 1;
			else if (value == "DuplexTumble") ID = 2;
			m_duplexbox->setButton(ID);
			slotDuplexChanged(ID);
		}
	}
	else
	{
		// Try to find "media" option
		value = opts["media"];
		if (!value.isEmpty())
		{
			int	index(-1);
			QStringList	l = QStringList::split(',',value,false);
			if (l.count() > 0 && (index=findOption(default_size,DEFAULT_SIZE,l[0])) >= 0)
				m_pagesize->setCurrentItem(index);
			if (l.count() > 1 && (index=findOption(default_type,DEFAULT_TYPE,l[1])) >= 0)
				m_papertype->setCurrentItem(index);
			if (l.count() > 2 && (index=findOption(default_source,DEFAULT_SOURCE,l[2])) >= 0)
				m_inputslot->setCurrentItem(index);
		}

		// Try to find "sides" option
		value = opts["sides"];
		int	ID(0);
		if (value == "two-sided-long-edge") ID = 1;
		else if (value == "two-sided-short-edge") ID = 2;
		m_duplexbox->setButton(ID);
		slotDuplexChanged(ID);
	}

	// Banners
	value = opts["job-sheets"];
	if (!value.isEmpty())
	{
		QStringList	l = QStringList::split(',',value,false);
		if (l.count() > 0) setComboItem(m_startbanner,l[0]);
		if (l.count() > 1) setComboItem(m_endbanner,l[1]);
	}

	// Orientation
	value = opts["orientation-requested"];
	if (!value.isEmpty())
	{
		bool	ok;
		int	ID = value.toInt(&ok)-3;
		if (ok)
		{
			m_orientbox->setButton(ID);
			slotOrientationChanged(ID);
		}
	}

	// N-up
	value = opts["number-up"];
	if (!value.isEmpty())
	{
		bool	ok;
		int	ID = QMIN(value.toInt(&ok)-1,2);
		if (ok)
		{
			m_nupbox->setButton(ID);
			slotNupChanged(ID);
		}
	}
}

void KPGeneralPage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	QString	value;

	if (driver())
	{
		DrListOption	*opt;
		if ((opt=(DrListOption*)driver()->findOption("PageSize")) != NULL)
		{
			DrBase	*ch = opt->choices()->at(m_pagesize->currentItem());
			if (incldef || ch->name() != opt->get("default")) opts["PageSize"] = ch->name();
		}
		if ((opt=(DrListOption*)driver()->findOption("MediaType")) != NULL)
		{
			DrBase	*ch = opt->choices()->at(m_papertype->currentItem());
			if (incldef || ch->name() != opt->get("default")) opts["MediaType"] = ch->name();
		}
		if ((opt=(DrListOption*)driver()->findOption("InputSlot")) != NULL)
		{
			DrBase	*ch = opt->choices()->at(m_inputslot->currentItem());
			if (incldef || ch->name() != opt->get("default")) opts["InputSlot"] = ch->name();
		}

		if (m_duplexbox->isEnabled() && (opt=(DrListOption*)driver()->findOption("Duplex")) != NULL)
		{
			bool twoChoices = ( m_duplexbox->count() == 2 );
			switch (m_duplexbox->id(m_duplexbox->selected()))
			{
				case DUPLEX_NONE_ID: value = ( twoChoices ? "Off" : "None" ); break;
				case DUPLEX_LONG_ID: value = ( twoChoices ? "On" : "DuplexNoTumble" ); break;
				case DUPLEX_SHORT_ID: value = "DuplexTumble"; break;
			}
			if (incldef || value != opt->get("default")) opts["Duplex"] = value;
		}
	}
	else
	{
		value = QString("%1,%2,%3").arg(default_size[m_pagesize->currentItem()*2]).arg(default_type[m_papertype->currentItem()*2]).arg(default_source[m_inputslot->currentItem()*2]);
		opts["media"] = value;

		if (m_duplexbox->isEnabled())
		{
			switch (m_duplexbox->id(m_duplexbox->selected()))
			{
				case 0: value = "one-sided"; break;
				case 1: value = "two-sided-long-edge"; break;
				case 2: value = "two-sided-short-edge"; break;
			}
			opts["sides"] = value;
		}
	}

	value = QString::number(m_orientbox->id(m_orientbox->selected())+3);
	if (value != "3" || incldef) opts["orientation-requested"] = value;

	if (m_nupbox->isEnabled())
	{
		switch (m_nupbox->id(m_nupbox->selected()))
		{
			case 0: value = "1"; break;
			case 1: value = "2"; break;
			case 2: value = "4"; break;
		}
		if (value != "1" || incldef) opts["number-up"] = value;
	}

	if (m_bannerbox->isEnabled())
	{
		QStringList	l = QStringList::split(',',printer()->option("kde-banners"),false);
		if (incldef || (l.count() == 2 && (l[0] != m_startbanner->currentText() || l[1] != m_endbanner->currentText()))
		    || (l.count() == 0 && (m_startbanner->currentText() != "none" || m_endbanner->currentText() != "none")))
		{
			value = QString("%1,%2").arg(m_startbanner->currentText()).arg(m_endbanner->currentText());
			opts["job-sheets"] = value;
		}
	}
}

void KPGeneralPage::slotOrientationChanged(int ID)
{
	QString	iconstr;
	switch (ID)
	{
		case ORIENT_PORTRAIT_ID: iconstr = "kdeprint_portrait"; break;
		case ORIENT_LANDSCAPE_ID: iconstr = "kdeprint_landscape"; break;
		case ORIENT_REVPORTRAIT_ID: iconstr = "kdeprint_revportrait"; break;
		case ORIENT_REVLANDSCAPE_ID: iconstr = "kdeprint_revlandscape"; break;
		default: iconstr = "kdeprint_portrait"; break;
	}
	m_orientpix->setPixmap(UserIcon(iconstr));
}

void KPGeneralPage::slotNupChanged(int ID)
{
	QString	iconstr;
	switch (ID)
	{
		case NUP_1_ID: iconstr = "kdeprint_nup1"; break;
		case NUP_2_ID: iconstr = "kdeprint_nup2"; break;
		case NUP_4_ID: iconstr = "kdeprint_nup4"; break;
		default: iconstr = "kdeprint_nup1"; break;
	}
	m_nuppix->setPixmap(UserIcon(iconstr));
}

void KPGeneralPage::slotDuplexChanged(int ID)
{
	if (m_duplexbox->isEnabled())
	{
		QString	iconstr;
		switch (ID)
		{
			case DUPLEX_NONE_ID: iconstr = "kdeprint_duplex_none"; break;
			case DUPLEX_LONG_ID: iconstr = "kdeprint_duplex_long"; break;
			case DUPLEX_SHORT_ID: iconstr = "kdeprint_duplex_short"; break;
			default: iconstr = "kdeprint_duplex_none"; break;
		}
		m_duplexpix->setPixmap(UserIcon(iconstr));
	}
}
#include "kpgeneralpage.moc"
