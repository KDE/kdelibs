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

#include "kphpgl2page.h"

#include <qcheckbox.h>
#include <q3groupbox.h>
#include <qlayout.h>
#include <klocale.h>
#include <knuminput.h>

KPHpgl2Page::KPHpgl2Page(QWidget *parent)
    : KPrintDialogPage(parent)
{
	//WhatsThis strings.... (added by pfeifle@kde.org)
	QString whatsThisBlackplotHpgl2Page = i18n( " <qt> "
			" <b>Print in Black Only (Blackplot)</b> "
			" <p>The \'blackplot\' option specifies that all pens should plot in black-only:"
			" The default is to use the colors defined in the plot file, or the standard "
			" pen colors defined in the HP-GL/2 reference manual from Hewlett Packard. </p> "
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o blackplot=true "
			" </pre> "
			" </p> "
			" </qt>" );

	QString whatsThisFitplotHpgl2Page = i18n( " <qt> "
			" <b>Scale Print Image to Page Size</b> "
			" <p>The 'fitplot' option specifies that the HP-GL image should be scaled to fill "
			" exactly the page with the (elsewhere selected) media size. </p> "
			" <p>The default is 'fitplot is disabled'. The default will therefore use the absolute "
			" distances specified in the plot file. (You should be aware that HP-GL files are very "
			" often CAD drawings intended for large format plotters. On standard office printers "
			" they will therefore lead to the drawing printout being spread across multiple pages.) </p> "
			" <p><b>Note:</b>This feature depends upon an accurate plot size (PS) command in the "
			" HP-GL/2 file. If no plot size is given in the file the filter converting the HP-GL "
			" to PostScript assumes the plot is ANSI E size. </p> "
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches with the CUPS commandline job option parameter:</em> "
			" <pre> "
			"    -o fitplot=true  "
			" </pre>"
			" </p> "
			" </qt>" );

	QString whatsThisPenwidthHpgl2Page = i18n( " <qt> "
			" <b>Set Pen Width for HP-GL (if not defined in file)</b>. "
			" <p>The pen width value can be set here in case the original HP-GL file does not have it "
			" set. The pen width specifies the value in micrometers. The default value of 1000 produces "
			" lines that are 1000 micrometers == 1 millimeter in width. Specifying a pen width of 0 "
			" produces lines that are exactly 1 pixel wide. </p> "
			" <p><b>Note:</b> The penwidth option set here is ignored if the pen widths are set inside "
			" the plot file itself..</p> "
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o penwidth=...   # example: \"2000\" or \"500\" "
			" </pre>"
			" </p> "
			" </qt>" );

	QString whatsThisAllOptionsHpgl2Page = i18n( " <qt> "
			" <b>HP-GL Print Options</b> "
			" <p>All options on this page are only applicable if you use KDEPrint to send HP-GL and "
			" HP-GL/2 files to one of your printers. </p> "
			" <p>HP-GL and HP-GL/2 are page description languages developed by Hewlett-Packard to drive "
			" Pen Plotting devices. </p> "
			" <p>KDEPrint can (with the help of CUPS) convert the HP-GL file format and print it "
			" on any installed printer. </p> "
			" <p><b>Note 1:</b> To print HP-GL files, start 'kprinter' and simply load the file into "
			" the running kprinter.</p>"
			" <p><b>Note 2:</b> The 'fitplot' parameter provided on this dialog does also work for "
			" printing PDF files (if your CUPS version is more recent than 1.1.22).</p> "
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> These KDEPrint GUI elements match with CUPS commandline job option parameters:</em> "
			" <pre>"
			"     -o blackplot=...  # examples: \"true\" or \"false\" "
			" <br> "
			"    -o fitplot=...    # examples: \"true\" or \"false\" "
			" <br> "
			"    -o penwidth=...   # examples: \"true\" or \"false\" "
			" </pre>"
			" </p> "
			" </qt>" );


	setTitle("HP-GL/2");

	Q3GroupBox	*box = new Q3GroupBox(0, Qt::Vertical, i18n("HP-GL/2 Options"), this);

	m_blackplot = new QCheckBox(i18n("&Use only black pen"), box);
	  m_blackplot->setWhatsThis(whatsThisBlackplotHpgl2Page);

	m_fitplot = new QCheckBox(i18n("&Fit plot to page"), box);
	  m_fitplot->setWhatsThis(whatsThisFitplotHpgl2Page);

	m_penwidth = new KIntNumInput(1000,box);
	m_penwidth->setLabel(i18n("&Pen width:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_penwidth->setSuffix(" [um]");
	m_penwidth->setRange(0, 10000, 100, true);
	  m_penwidth->setWhatsThis(whatsThisPenwidthHpgl2Page);

	QVBoxLayout	*l0 = new QVBoxLayout(this, 0, 10);
	l0->addWidget(box);
	l0->addStretch(1);

	QVBoxLayout	*l1 = new QVBoxLayout(box->layout(), 10);
	l1->addWidget(m_blackplot);
	l1->addWidget(m_fitplot);
	l1->addWidget(m_penwidth);
	  this->setWhatsThis(whatsThisAllOptionsHpgl2Page);
}

KPHpgl2Page::~KPHpgl2Page()
{
}

void KPHpgl2Page::setOptions(const QMap<QString,QString>& opts)
{
	QString	value;
	if (opts.contains("blackplot") && ((value=opts["blackplot"]).isEmpty() || value == "true"))
		m_blackplot->setChecked(true);
	if (opts.contains("fitplot") && ((value=opts["fitplot"]).isEmpty() || value == "true"))
		m_fitplot->setChecked(true);
	if (!(value=opts["penwidth"]).isEmpty())
		m_penwidth->setValue(value.toInt());
}

void KPHpgl2Page::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	if (incldef || m_penwidth->value() != 1000)
		opts["penwidth"] = QString::number(m_penwidth->value());
	if (m_blackplot->isChecked())
		opts["blackplot"] = "true";
	else if (incldef)
		opts["blackplot"] = "false";
	else
		opts.remove("blackplot");
	if (m_fitplot->isChecked())
		opts["fitplot"] = "true";
	else if (incldef)
		opts["fitplot"] = "false";
	else
		opts.remove("fitplot");
}
