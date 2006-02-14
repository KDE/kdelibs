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

#include "kpgeneralpage.h"
#include "driver.h"
#include "kmprinter.h"
#include "kprinter.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <q3buttongroup.h>
#include <qlayout.h>
#include <qradiobutton.h>

#include <kcursor.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>

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

static void initCombo(QComboBox *cb, DrListOption *opt)
{
	cb->clear();
        foreach (DrBase* choice, opt->choices())
	{
		cb->addItem(choice->get("text"));
		if (choice == opt->currentChoice())
			cb->setCurrentIndex(cb->count()-1);
	}
}

static void setComboItem(QComboBox *cb, const QString& txt)
{
	for (int i=0;i<cb->count();i++)
		if (cb->itemText(i) == txt)
		{
			cb->setCurrentIndex(i);
			return;
		}
}

static int findOption(const char *strs[], int n, const QString& txt)
{
	for (int i=0;i<n;i+=2)
		if (txt == strs[i]) return (i/2);
	return (-1);
}

//*************************************************************************************************

// default values in case of NULL driver
static const char*	default_size[] = {
	"A4", I18N_NOOP("ISO A4"),
	"Letter", I18N_NOOP("US Letter"),
	"Legal", I18N_NOOP("US Legal"),
	"Ledger", I18N_NOOP("Ledger"),
	"Folio", I18N_NOOP("Folio"),
	"Comm10", I18N_NOOP("US #10 Envelope"),
	"DL", I18N_NOOP("ISO DL Envelope"),
	"Tabloid", I18N_NOOP("Tabloid"),
	"A3", I18N_NOOP( "ISO A3" ),
	"A2", I18N_NOOP( "ISO A2" ),
	"A1", I18N_NOOP( "ISO A1" ),
	"A0", I18N_NOOP( "ISO A0" )
};

#define SMALLSIZE_BEGIN   0
#define MEDIUMSIZE_BEGIN 14
#define HIGHSIZE_BEGIN   20
#define DEFAULT_SIZE     24

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

KPGeneralPage::KPGeneralPage(KMPrinter *pr, DrMain *dr, QWidget *parent)
    : KPrintDialogPage(pr,dr,parent)
{
	//WhatsThis strings.... (added by pfeifle@kde.org)
	QString whatsThisPrintPropertiesGeneralPage = i18n( " <qt> "
			" <p><b>\"General\"</b> </p> "
			" <p>This dialog page contains <em>general</em> print job settings."
			" General settings are applicable to most printers, most jobs "
			" and most job file types. "
                        " <p>To get more specific help, enable the \"WhatsThis\" cursor and click on any of the "
                        " text labels or GUI elements of this dialog. "
			" </qt>" );
	QString whatsThisGeneralPageSizeLabel = i18n( " <qt> "
			" <p><b>Page size:</b> Select paper size to be printed on from "
			" the drop-down menu. </p>"
			" <p>The exact list of choices depends on the printer driver (\"PPD\") you have installed.</p> "
                        " <br> "
                        " <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o PageSize=...         # examples: \"A4\" or \"Letter\" "
			" </pre>"
			" </p> "
			" </qt>" );
	QString whatsThisGeneralPaperTypeLabel = i18n( " <qt> "
			" <p><b>Paper type:</b> Select paper type to be printed on from "
			" the drop-down menu. </p>"
			" <p>The exact list of choices depends on the printer driver (\"PPD\") you have installed. </p>"
                        " <br> "
                        " <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o MediaType=...        # example: \"Transparency\" "
			" </pre>"
			" </p> "
			" </qt>" );
	QString whatsThisGeneralPaperSourceLabel = i18n( " <qt> "
			" <p><b>Paper source:</b> Select paper source tray for the paper"
                        " to be printed on from the drop-down menu. "
			" <p>The exact list of choices depends on the printer driver (\"PPD\") you have installed. </p>"
                        " <br> "
                        " <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o InputSlot=...        # examples: \"Lower\" or \"LargeCapacity\" "
			" </pre>"
			" </p> "
			" </qt>" );
	QString whatsThisGeneralOrientationLabel = i18n( " <qt> "
			" <p><b>Image Orientation:</b> Orientation of the printed "
                        " page image on your paper is controlled by the radio buttons. By default, "
                        " the orientation is <em>Portrait</em> "
                        " <p>You can select 4 alternatives: "
                        " <ul> "
                        " <li> <b>Portrait.</b>.Portrait is the default setting. </li> "
                        " <li> <b>Landscape.</b> </li> "
                        " <li> <b>Reverse Landscape.</b> Reverse Landscape prints the images upside down. </li> "
                        " <li> <b>Reverse Portrait.</b> Reverse Portrait prints the image upside down.</li> "
                        " </ul> "
                        " The icon changes according to your selection.</p>"
                        " <br> "
                        " <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o orientation-requested=...       # examples: \"landscape\" or \"reverse-portrait\" "
			" </pre>"
			" </p> "
			" </qt>" );
	QString whatsThisGeneralDuplexLabel = i18n( " <qt> "
			" <p><b>Duplex Printing:</b> These controls may be grayed out if your printer "
                        " does not support <em>duplex printing</em> (i.e. printing on both sides of the sheet). "
                        " These controls are active if your printer supports duplex printing. "
                        " <p> "
                        " You can choose from 3 alternatives: </p>"
                        " <ul> "
                        " <li> <b>None.</b> This prints each page of the job on one side of the sheets only. </li> "
                        " <li> <b>Long Side.</b> This prints the job on both sides of the paper sheets. "
                        " It prints the job in a way so that the backside has the same orientation as the front "
                        " side if you turn the paper over the long edge. (Some printer drivers name this mode "
                        " <em>duplex-non-tumbled</em>).  </li> "
                        " <li> <b>Short Side.</b> This prints the job on both sides of the paper sheets. "
                        " It prints the job so that the backside has the reverse orientation from the front "
                        " side if you turn the paper over the long edge, but the same orientation, if you turn it over "
                        " the short edge. (Some printer drivers name this mode "
                        " <em>duplex-tumbled</em>).  </li> "
                        " </ul> "
                        " <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o duplex=...       # examples: \"tumble\" or \"two-sided-short-edge\" "
			" </pre>"
			" </p> "
			" </qt>" );
	QString whatsThisGeneralBannersLabel = i18n( " <qt> "
			" <p><b>Banner Page(s):</b> Select banner(s) to print one or two special sheets "
                        " of paper just before or after your main job. </p>"
			" <p>Banners may contain some pieces of job information, such as user name, time of printing, job"
                        " title and more. </p>"
                        " <p>Banner pages are useful to separate different jobs more easily, especially in a multi-user "
                        " environment. </p>"
                        " <p><em><b>Hint:</em></b> You can design your own banner pages. To make use of them, just put the banner "
                        " file into the standard CUPS <em>banners</em> directory (This is usually <em>\"/usr/share/cups/banner/\"</em> "
			" Your custom banner(s) must have one of the supported printable formats. "
                        " Supported formats are ASCII text, PostScript, PDF and nearly any image format such as PNG, JPEG or "
                        " GIF. Your added banner pages will appear in the drop down menu after a restart of CUPS. </p>"
                        " <p>CUPS comes with a selection of banner pages. </p>"
                        " <br> "
                        " <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o job-sheets=...       # examples: \"standard\" or \"topsecret\" "
			" </pre>"
			" </p> "
			" </qt>" );
	QString whatsThisGeneralPagesPerSheetLabel = i18n( " <qt> "
			" <p><b>Pages per Sheet:</b> "
                        " You can choose to print more than one page onto each sheet of paper. "
                        " This is sometimes useful to save paper. </p>"
                        " <p><b>Note 1:</b> the page images get scaled down accordingly to print 2 or 4 pages per sheet. "
                        " The page image does not get scaled if you print 1 page per sheet (the default setting.). "
                        " <p><b>Note 2:</b> If you select multiple pages per sheet here, the scaling and re-arranging is done "
                        " by your printing system. Be aware, that some printers can by themselves print multiple pages per sheet. "
                        " In this case you find the option in the printer driver settings. Be careful: if you enable multiple "
                        " pages per sheet in both places, your printout will not look as you intended. </p>"
                        " <br> "
                        " <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o number-up=...        # examples: \"2\" or \"4\" "
			" </pre>"
			" </p> "
			" </qt>" );


	setTitle(i18n("General"));

	// widget creation
	QLabel	*m_pagesizelabel = new QLabel(i18n("Page s&ize:"), this);
	m_pagesizelabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
          m_pagesizelabel->setWhatsThis(whatsThisGeneralPageSizeLabel);

	QLabel	*m_papertypelabel = new QLabel(i18n("Paper t&ype:"), this);
	m_papertypelabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
          m_papertypelabel->setWhatsThis(whatsThisGeneralPaperTypeLabel);

	QLabel	*m_inputslotlabel = new QLabel(i18n("Paper so&urce:"), this);
	m_inputslotlabel->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
          m_inputslotlabel->setWhatsThis(whatsThisGeneralPaperSourceLabel);

	m_pagesize = new QComboBox(this);
          m_pagesize->setWhatsThis(whatsThisGeneralPageSizeLabel);

	m_papertype = new QComboBox(this);
          m_papertype->setWhatsThis(whatsThisGeneralPaperTypeLabel);

	m_inputslot = new QComboBox(this);
          m_inputslot->setWhatsThis(whatsThisGeneralPaperSourceLabel);

	m_pagesizelabel->setBuddy(m_pagesize);
	m_papertypelabel->setBuddy(m_papertype);
	m_inputslotlabel->setBuddy(m_inputslot);

	m_orientbox = new Q3ButtonGroup(0, Qt::Vertical, i18n("Orientation"), this);
          m_orientbox->setWhatsThis(whatsThisGeneralOrientationLabel);

	m_duplexbox = new Q3ButtonGroup(0, Qt::Vertical, i18n("Duplex Printing"), this);
          m_duplexbox->setWhatsThis(whatsThisGeneralDuplexLabel);

	m_nupbox = new Q3ButtonGroup(0, Qt::Vertical, i18n("Pages per Sheet"), this);
          m_nupbox->setWhatsThis(whatsThisGeneralPagesPerSheetLabel);

	m_bannerbox = new Q3GroupBox(0, Qt::Vertical, i18n("Banners"), this);
          m_bannerbox->setWhatsThis(whatsThisGeneralBannersLabel);

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
	QVBoxLayout	*lay0 = new QVBoxLayout(this);
  lay0->setMargin(0);
  lay0->setSpacing(KDialog::spacingHint());
  this->setWhatsThis(whatsThisPrintPropertiesGeneralPage);
	QGridLayout	*lay1 = new QGridLayout(0);
  lay1->setMargin(0);
  lay1->setSpacing(KDialog::spacingHint());
	QGridLayout	*lay2 = new QGridLayout(0);
  lay2->setMargin(0);
  lay2->setSpacing(KDialog::spacingHint());
	lay0->addStretch(1);
	lay0->addLayout(lay1);
	lay0->addStretch(1);
	lay0->addLayout(lay2);
	lay0->addStretch(2);
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
	lay2->setColumnStretch(0, 1);
	lay2->setColumnStretch(1, 1);
	QGridLayout	*lay3 = new QGridLayout(0);
	lay3->setSpacing(KDialog::spacingHint());
  m_orientbox->layout()->addItem(lay3);
	lay3->addWidget(m_portrait, 0, 0);
	lay3->addWidget(m_landscape, 1, 0);
	lay3->addWidget(m_revland, 2, 0);
	lay3->addWidget(m_revport, 3, 0);
	lay3->addWidget(m_orientpix, 0, 3, 1, 1);
	QGridLayout	*lay4 = new QGridLayout(0);
  lay4->setSpacing(KDialog::spacingHint());
  m_duplexbox->layout()->addItem(lay4);
	lay4->addWidget(m_dupnone, 0, 0);
	lay4->addWidget(m_duplong, 1, 0);
	lay4->addWidget(m_dupshort, 2, 0);
	lay4->addWidget(m_duplexpix, 0, 2, 1, 1);
	lay4->setRowStretch( 0, 1 );
	QGridLayout	*lay5 = new QGridLayout(0);
  lay5->setSpacing(KDialog::spacingHint());
  m_nupbox->layout()->addItem(lay5);
	lay5->addWidget(m_nup1, 0, 0);
	lay5->addWidget(m_nup2, 1, 0);
	lay5->addWidget(m_nup4, 2, 0);
	lay5->addWidget(m_nuppix, 0, 2, 1, 1);
	QGridLayout	*lay6 = new QGridLayout(0);
  lay6->setSpacing(KDialog::spacingHint());
  m_bannerbox->layout()->addItem(lay6);
	lay6->addWidget(m_startbannerlabel, 0, 0);
	lay6->addWidget(m_endbannerlabel, 1, 0);
	lay6->addWidget(m_startbanner, 0, 1);
	lay6->addWidget(m_endbanner, 1, 1);
	lay6->setColumnStretch(1, 1);

	// connections (+ misc)
	connect(m_orientbox,SIGNAL(clicked(int)),SLOT(slotOrientationChanged(int)));
	connect(m_nupbox,SIGNAL(clicked(int)),SLOT(slotNupChanged(int)));
	connect(m_duplexbox,SIGNAL(clicked(int)),SLOT(slotDuplexChanged(int)));

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
			if ( opt->choices().count() == 2 )
			{
				// probably a On/Off option instead of the standard PS one
				QAbstractButton *btn = m_duplexbox->find( DUPLEX_SHORT_ID );
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
		for (int i=SMALLSIZE_BEGIN+1;i<MEDIUMSIZE_BEGIN;i+=2)
			m_pagesize->addItem(i18n(default_size[i]));
		if ( printer()->printerCap() & KMPrinter::CapMedium )
			for ( int i=MEDIUMSIZE_BEGIN+1; i<HIGHSIZE_BEGIN; i+=2 )
				m_pagesize->addItem(i18n(default_size[i]));
		if ( printer()->printerCap() & KMPrinter::CapLarge )
			for ( int i=HIGHSIZE_BEGIN+1; i<DEFAULT_SIZE; i+=2 )
				m_pagesize->addItem(i18n(default_size[i]));
		// set default page size using locale settings
		QString	psname = pageSizeToPageName((KPrinter::PageSize)(KGlobal::locale()->pageSize()));
		int index = findOption(default_size, DEFAULT_SIZE, psname);
		if (index >= 0)
			m_pagesize->setCurrentIndex(index);
		// MediaType
		for (int i=1;i<DEFAULT_TYPE;i+=2)
			m_papertype->addItem(i18n(default_type[i]));
		// PageSize
		for (int i=1;i<DEFAULT_SOURCE;i+=2)
			m_inputslot->addItem(i18n(default_source[i]));

		// Enable duplex setting if supported
		m_duplexbox->setEnabled( printer()->printerCap() & KMPrinter::CapDuplex );
	}

	// Banners
	QStringList	values = printer()->option("kde-banners-supported").split(',',QString::SkipEmptyParts);
	if (values.count() > 0)
	{
		for (QStringList::ConstIterator it = values.begin(); it != values.end(); ++it)
		{
			m_startbanner->addItem(*it);
			m_endbanner->addItem(*it);
		}
		values = printer()->option("kde-banners").split(',',QString::SkipEmptyParts);
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
		value = opts["media"];
		QStringList	l = value.split(',',QString::SkipEmptyParts);
		for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it)
		{
			value = *it;
			DrBase	*ch;
			if ((ch = (driver()->findOption("PageSize"))) &&
                            (ch = (static_cast<DrListOption*>(ch))->findChoice(value)))
			{
				if (m_pagesize->isEnabled())
					setComboItem(m_pagesize, ch->get("text"));
			}
			else if ((ch = (driver()->findOption("MediaType"))) &&
                                 (ch = (static_cast<DrListOption*>(ch))->findChoice(value)))

			{
				if (m_papertype->isEnabled())
					setComboItem(m_papertype, ch->get("text"));
			}
			else if ((ch = (driver()->findOption("InputSlot"))) &&
                                 (ch = (static_cast<DrListOption*>(ch))->findChoice(value)))
			{
				if (m_inputslot)
					setComboItem(m_inputslot, ch->get("text"));
			}
			else
			{
				kWarning() << "media option '" << value << "' not handled." << endl;
			}
		}
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
			QStringList	l = value.split(',',QString::SkipEmptyParts);
			for(QStringList::ConstIterator it = l.begin(); it != l.end(); ++it)
			{
				value = *it;

				if ((index=findOption(default_size,DEFAULT_SIZE,value)) >= 0)
					m_pagesize->setCurrentIndex(index);
				else if ((index=findOption(default_type,DEFAULT_TYPE,value)) >= 0)
					m_papertype->setCurrentIndex(index);
				else if ((index=findOption(default_source,DEFAULT_SOURCE,value)) >= 0)
					m_inputslot->setCurrentIndex(index);
				else
					kWarning() << "media option '" << value << "' not handled." << endl;
			}
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
		QStringList	l = value.split(',',QString::SkipEmptyParts);
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
		int	ID = qMin(value.toInt(&ok)-1,2);
		if (ok)
		{
			m_nupbox->setButton(ID);
			slotNupChanged(ID);
		}
	}

	if ( m_orientbox->isEnabled() )
		m_orientbox->setDisabled( opts[ "kde-orientation-fixed" ] == "1" );
	if ( m_pagesize->isEnabled() )
		m_pagesize->setDisabled( opts[ "kde-pagesize-fixed" ] == "1" );
}

void KPGeneralPage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	QString	value;

	if (driver())
	{
		DrListOption	*opt;
		if ((opt=(DrListOption*)driver()->findOption("PageSize")) != NULL)
		{
			DrBase	*ch = opt->choices().at(m_pagesize->currentIndex());
			if (incldef || ch->name() != opt->get("default")) opts["PageSize"] = ch->name();
		}
		if ((opt=(DrListOption*)driver()->findOption("MediaType")) != NULL)
		{
			DrBase	*ch = opt->choices().at(m_papertype->currentIndex());
			if (incldef || ch->name() != opt->get("default")) opts["MediaType"] = ch->name();
		}
		if ((opt=(DrListOption*)driver()->findOption("InputSlot")) != NULL)
		{
			DrBase	*ch = opt->choices().at(m_inputslot->currentIndex());
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
		value = QString("%1,%2,%3").arg(default_size[m_pagesize->currentIndex()*2]).arg(default_type[m_papertype->currentIndex()*2]).arg(default_source[m_inputslot->currentIndex()*2]);
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
		QStringList	l = printer()->option("kde-banners").split(',',QString::SkipEmptyParts);
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
