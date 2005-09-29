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

#include "kptextpage.h"
#include "marginwidget.h"
#include "driver.h"
#include "kprinter.h"

#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <knuminput.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kseparator.h>
#include <kdebug.h>

KPTextPage::KPTextPage(DrMain *driver, QWidget *parent, const char *name)
: KPrintDialogPage(0, driver, parent, name)
{
	//WhatsThis strings.... (added by pfeifle@kde.org)
	QString whatsThisCPITextPage = i18n( " <qt> "
			" <p><b>Characters Per Inch</b></p> "
			" <p>This setting controls the horizontal size of characters when printing a text file. </p>"
			" <p>The default value is 10, meaning that the font is scaled in a way that 10 characters "
			" per inch will be printed. </p> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o cpi=...          # example: \"8\" or \"12\" "
			" </pre>"
			" </p> "
			" </qt>" );

	QString whatsThisLPITextPage = i18n( " <qt> "
			" <p><b>Lines Per Inch</b></p> "
			" <p>This setting controls the vertical size of characters when printing a text file. </p>"
			" <p>The default value is 6, meaning that the font is scaled in a way that 6 lines "
			" per inch will be printed. </p> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o lpi=...         # example \"5\" or \"7\" "
			" </pre>"
			" </p> "
			" </qt>" );

	QString whatsThisColumnsTextPage = i18n( " <qt> "
			" <p><b>Columns</b></p> "
			" <p>This setting controls how many columns of text will be printed on each page when."
			" printing text files. </p> "
			" <p>The default value is 1, meaning that only one column of text per page "
			" will be printed. </p> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o columns=...     # example: \"2\" or \"4\" "
			" </pre>"
			" </p> "
			" </qt>" );

	QString whatsThisPrettyprintPreviewIconTextPage = i18n( " <qt> "
			" Preview icon changes when you turn on or off prettyprint. "
			" </qt>" );
	QString whatsThisFormatTextPage = i18n( " <qt> "
			" <p><b>Text Formats</b></p> "
			" <p>These settings control the appearance of text on printouts. They are only valid for "
			" printing text files or input directly through kprinter. </p> "
			" <p><b>Note:</b> These settings have no effect whatsoever for other input formats than "
			" text, or for printing from applications such as the KDE Advanced Text Editor. (Applications "
			" in general send PostScript to the print system, and 'kate' in particular has its own "
			" knobs to control the print output. </p>."
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"     -o cpi=...         # example: \"8\" or \"12\" "
			" <br> "
			"    -o lpi=...         # example: \"5\" or \"7\" "
			" <br> "
			"    -o columns=...     # example: \"2\" or \"4\" "
			" </pre>"
			" </p> "
			" </qt>" );

	QString whatsThisMarginsTextPage = i18n( " <qt> "
			" <p><b>Margins</b></p> "
			" <p>These settings control the margins of printouts on the paper. They are not valid for "
			" jobs originating from applications which define their own page layout internally and "
			" send PostScript to KDEPrint (such as KOffice or OpenOffice.org). </p> "
			" <p>When printing from KDE applications, such as KMail and Konqueror, or printing an ASCII text "
			" file through kprinter, you can choose your preferred margin settings here. </p> "
			" <p>Margins may be set individually for each edge of the paper. The combo box at the bottom lets you change "
			" the units of measurement between Pixels, Millimeters, Centimeters, and Inches. </p> "
			" <p>You can even use the mouse to grab one margin and drag it to the intended position (see the "
			" preview picture on the right side). </p> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"     -o page-top=...      # example: \"72\" "
			" <br> "
			"    -o page-bottom=...   # example: \"24\" "
			" <br> "
			"    -o page-left=...     # example: \"36\" "
			" <br> "
			"    -o page-right=...    # example: \"12\" "
			" </pre>"
			" </p> "
			" </qt>" );

	QString whatsThisPrettyprintButtonOnTextPage = i18n( " <qt> "
			" <p><b>Turn Text Printing with Syntax Highlighting (Prettyprint) On!</b></p> "
			" <p>ASCII text file printouts can be 'prettyfied' by enabling this option. If you do so, "
			" a header is printed at the top of each page. The header contains "
			" the page number, job title (usually the filename), and the date. In addition, C and "
			" C++ keywords are highlighted, and comment lines are italicized.</p>"
			" <p>This prettyprint option is handled by CUPS.</p> "
			" <p>If you prefer another 'plaintext-to-prettyprint' converter, look for the <em>enscript</em> "
			" pre-filter on the <em>Filters</em> tab. </p>"
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"     -o prettyprint=true. "
			" </pre>"
			" </p> "
			" </qt>" );

	QString whatsThisPrettyprintButtonOffTextPage = i18n( " <qt> "
			" <p><b>Turn Text Printing with Syntax Highlighting (Prettyprint) Off! </b></p> "
			" <p>ASCII text file printing with this option turned off are appearing without a page "
			" header and without syntax highlighting. (You can still set the page margins, though.) </p> "
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o prettyprint=false "
			" </pre>"
			" </p> "
			" </qt>" );

	QString whatsThisPrettyprintFrameTextPage = i18n( " <qt> "
			" <p><b>Print Text with Syntax Highlighting (Prettyprint)</b></p> "
			" <p>ASCII file printouts can be 'prettyfied' by enabling this option. If you do so, "
			" a header is printed at the top of each page. The header contains "
			" the page number, job title (usually the filename), and the date. In addition, C and "
			" C++ keywords are highlighted, and comment lines are italicized.</p>"
			" <p>This prettyprint option is handled by CUPS.</p> "
			" <p>If you prefer another 'plaintext-to-prettyprint' converter, look for the <em>enscript</em> "
			" pre-filter on the <em>Filters</em> tab. </p> "
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"     -o prettyprint=true. "
			" <br> "
			"    -o prettyprint=false "
			" </pre>"
			" </p> "
			" </qt>" );

	setTitle(i18n("Text"));
	m_block = false;

	QGroupBox	*formatbox = new QGroupBox(0, Qt::Vertical, i18n("Text Format"), this);
	  QWhatsThis::add(formatbox, whatsThisFormatTextPage);
	QGroupBox	*prettybox = new QGroupBox(0, Qt::Vertical, i18n("Syntax Highlighting"), this);
	  QWhatsThis::add(prettybox, whatsThisPrettyprintFrameTextPage);
	QGroupBox	*marginbox = new QGroupBox(0, Qt::Vertical, i18n("Margins"), this);
	  QWhatsThis::add(marginbox, whatsThisMarginsTextPage);

	m_cpi = new KIntNumInput(10, formatbox);
	  QWhatsThis::add(m_cpi, whatsThisCPITextPage);
	m_cpi->setLabel(i18n("&Chars per inch:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_cpi->setRange(1, 999, 1, false);
	m_lpi = new KIntNumInput(m_cpi, 6, formatbox);
	  QWhatsThis::add(m_lpi, whatsThisLPITextPage);
	m_lpi->setLabel(i18n("&Lines per inch:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_lpi->setRange(1, 999, 1, false);
	m_columns = new KIntNumInput(m_lpi, 1, formatbox);
	  QWhatsThis::add(m_columns, whatsThisColumnsTextPage);
	m_columns->setLabel(i18n("C&olumns:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_columns->setRange(1, 10, 1, false);
	KSeparator	*sep = new KSeparator(Qt::Horizontal, formatbox);
	connect(m_columns, SIGNAL(valueChanged(int)), SLOT(slotColumnsChanged(int)));

	m_prettypix = new QLabel(prettybox);
	  QWhatsThis::add(m_prettypix, whatsThisPrettyprintPreviewIconTextPage);
	m_prettypix->setAlignment(Qt::AlignCenter);
	QRadioButton	*off = new QRadioButton(i18n("&Disabled"), prettybox);
	  QWhatsThis::add(off, whatsThisPrettyprintButtonOffTextPage);
	QRadioButton	*on = new QRadioButton(i18n("&Enabled"), prettybox);
	  QWhatsThis::add(on, whatsThisPrettyprintButtonOnTextPage);
	m_prettyprint = new QButtonGroup(prettybox);
	m_prettyprint->hide();
	m_prettyprint->insert(off, 0);
	m_prettyprint->insert(on, 1);
	m_prettyprint->setButton(0);
	connect(m_prettyprint, SIGNAL(clicked(int)), SLOT(slotPrettyChanged(int)));
	slotPrettyChanged(0);

	m_margin = new MarginWidget(marginbox);
	  QWhatsThis::add(m_margin, whatsThisMarginsTextPage);
	m_margin->setPageSize(595, 842);

	QGridLayout	*l0 = new QGridLayout(this, 2, 2, 0, 10);
	l0->addWidget(formatbox, 0, 0);
	l0->addWidget(prettybox, 0, 1);
	l0->addMultiCellWidget(marginbox, 1, 1, 0, 1);
	QVBoxLayout	*l1 = new QVBoxLayout(formatbox->layout(), 5);
	l1->addWidget(m_cpi);
	l1->addWidget(m_lpi);
	l1->addWidget(sep);
	l1->addWidget(m_columns);
	QGridLayout	*l2 = new QGridLayout(prettybox->layout(), 2, 2, 10);
	l2->addWidget(off, 0, 0);
	l2->addWidget(on, 1, 0);
	l2->addMultiCellWidget(m_prettypix, 0, 1, 1, 1);
	QVBoxLayout	*l3 = new QVBoxLayout(marginbox->layout(), 10);
	l3->addWidget(m_margin);
}

KPTextPage::~KPTextPage()
{
}

void KPTextPage::setOptions(const QMap<QString,QString>& opts)
{
	QString	value;

	if (!(value=opts["cpi"]).isEmpty())
		m_cpi->setValue(value.toInt());
	if (!(value=opts["lpi"]).isEmpty())
		m_lpi->setValue(value.toInt());
	if (!(value=opts["columns"]).isEmpty())
		m_columns->setValue(value.toInt());
	int	ID(0);
	if (opts.contains("prettyprint") && (opts["prettyprint"].isEmpty() || opts["prettyprint"] == "true"))
		ID = 1;
	m_prettyprint->setButton(ID);
	slotPrettyChanged(ID);

	// get default margins
	m_currentps = opts["PageSize"];
	QString	orient = opts["orientation-requested"];
	bool	landscape = (orient == "4" || orient == "5");
	initPageSize(landscape);

	bool	marginset(false);
	if (!(value=opts["page-top"]).isEmpty() && value.toFloat() != m_margin->top())
	{
		marginset = true;
		m_margin->setTop(value.toFloat());
	}
	if (!(value=opts["page-bottom"]).isEmpty() && value.toFloat() != m_margin->bottom())
	{
		marginset = true;
		m_margin->setBottom(value.toFloat());
	}
	if (!(value=opts["page-left"]).isEmpty() && value.toFloat() != m_margin->left())
	{
		marginset = true;
		m_margin->setLeft(value.toFloat());
	}
	if (!(value=opts["page-right"]).isEmpty() && value.toFloat() != m_margin->right())
	{
		marginset = true;
		m_margin->setRight(value.toFloat());
	}
	m_margin->setCustomEnabled(marginset);
}

void KPTextPage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	if (incldef || m_cpi->value() != 10)
		opts["cpi"] = QString::number(m_cpi->value());
	if (incldef || m_lpi->value() != 6)
		opts["lpi"] = QString::number(m_lpi->value());
	if (incldef || m_columns->value() != 1)
		opts["columns"] = QString::number(m_columns->value());

	//if (m_margin->isCustomEnabled() || incldef)
	if (m_margin->isCustomEnabled())
	{
		opts["page-top"] = QString::number(( int )( m_margin->top()+0.5 ));
		opts["page-bottom"] = QString::number(( int )( m_margin->bottom()+0.5 ));
		opts["page-left"] = QString::number(( int )( m_margin->left()+0.5 ));
		opts["page-right"] = QString::number(( int )( m_margin->right()+0.5 ));
	}
	else
	{
		opts.remove("page-top");
		opts.remove("page-bottom");
		opts.remove("page-left");
		opts.remove("page-right");
	}

	if (m_prettyprint->id(m_prettyprint->selected()) == 1)
		opts["prettyprint"] = "true";
	else if (incldef)
		opts["prettyprint"] = "false";
	else
		opts.remove("prettyprint");
}

void KPTextPage::slotPrettyChanged(int ID)
{
	QString	iconstr = (ID == 0 ? "kdeprint_nup1" : "kdeprint_prettyprint");
	m_prettypix->setPixmap(UserIcon(iconstr));
}

void KPTextPage::slotColumnsChanged(int)
{
	// TO BE IMPLEMENTED
}

void KPTextPage::initPageSize(bool landscape)
{
	float w( -1 ), h( -1 );
	float mt( 36 ), mb( mt ), ml( 18 ), mr( ml );
	if (driver())
	{
		if (m_currentps.isEmpty())
		{
			DrListOption	*o = (DrListOption*)driver()->findOption("PageSize");
			if (o)
				m_currentps = o->get("default");
		}
		if (!m_currentps.isEmpty())
		{
			DrPageSize	*ps = driver()->findPageSize(m_currentps);
			if (ps)
			{
				w = ps->pageWidth();
				h = ps->pageHeight();
				mt = ps->topMargin();
				ml = ps->leftMargin();
				mr = ps->rightMargin();
				mb = ps->bottomMargin();
			}
		}
	}
	m_margin->setPageSize(w, h);
	m_margin->setOrientation(landscape ? KPrinter::Landscape : KPrinter::Portrait);
	m_margin->setDefaultMargins( mt, mb, ml, mr );
	m_margin->setCustomEnabled(false);
}

#include "kptextpage.moc"
