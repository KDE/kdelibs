/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <kdeprint@swing.be>
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

#include "marginwidget.h"
#include "marginpreview.h"
#include "marginvaluewidget.h"
#include "kprinter.h"

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <klocale.h>
#include <kglobal.h>

MarginWidget::MarginWidget(QWidget *parent, bool allowMetricUnit)
    : QWidget(parent), m_default(4, 0), m_pagesize( 2 )
{
	//WhatsThis strings.... (added by pfeifle@kde.org)
	QString whatsThisTopMarginWidget = i18n( " <qt> "
			" <p><b>Top Margin</b></p>. "
			" <p>This spinbox/text edit field lets you control the top margin of your printout if the printing "
			" application does not define its margins internally. </p> "
			" <p>The setting works for instance for ASCII text file printing, or for printing from KMail and "
			" and Konqueror.. </p>"
			" <p><b>Note:</b></p>This margin setting is not intended for KOffice or OpenOffice.org printing, "
			" because these applications (or rather their users) are expected to do it by themselves. "
			" It also does not work for PostScript or PDF file, which in most cases have their margins hardcoded "
			" internally.</p> "
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o page-top=...      # use values from \"0\" or higher. \"72\" is equal to 1 inch. "
			" </pre>"
			" </p> "
			" </qt>" );

	QString whatsThisBottomMarginWidget = i18n( " <qt> "
			" <p><b>Bottom Margin</b></p>. "
			" <p>This spinbox/text edit field lets you control the bottom margin of your printout if the printing "
			" application does not define its margins internally. </p> "
			" <p>The setting works for instance for ASCII text file printing, or for printing from KMail and "
			" and Konqueror. </p>"
			" <p><b>Note:</b></p>This margin setting is not intended for KOffice or OpenOffice.org printing, "
			" because these applications (or rather their users) are expected to do it by themselves. "
			" It also does not work for PostScript or PDF file, which in most cases have their margins hardcoded "
			" internally.</p> "
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o page-bottom=...      # use values from \"0\" or higher. \"72\" is equal to 1 inch. "
			" </pre>"
			" </qt>" );

	QString whatsThisLeftMarginWidget = i18n( " <qt> "
			" <p><b>Left Margin</b></p>. "
			" <p>This spinbox/text edit field lets you control the left margin of your printout if the printing "
			" application does not define its margins internally. </p> "
			" <p>The setting works for instance for ASCII text file printing, or for printing from KMail and "
			" and Konqueror. </p>"
			" <p><b>Note:</b></p>This margin setting is not intended for KOffice or OpenOffice.org printing, "
			" because these applications (or rather their users) are expected to do it by themselves. "
			" It also does not work for PostScript or PDF file, which in most cases have their margins hardcoded "
			" internally.</p> "
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o page-left=...      # use values from \"0\" or higher. \"72\" is equal to 1 inch. "
			" </pre>"
			" </qt>" );

	QString whatsThisRightMarginWidget = i18n( " <qt> "
			" <p><b>Right Margin</b></p>. "
			" <p>This spinbox/text edit field lets you control the right margin of your printout if the printing "
			" application does not define its margins internally. </p> "
			" <p>The setting works for instance for ASCII text file printing, or for printing from KMail and "
			" and Konqueror. </p>"
			" <p><b>Note:</b></p>This margin setting is not intended for KOffice or OpenOffice.org printing, "
			" because these applications (or rather their users) are expected to do it by themselves. "
			" It also does not work for PostScript or PDF file, which in most cases have their margins hardcoded "
			" internally.</p> "
			" <br> "
			" <hr> "
			" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
			" with the CUPS commandline job option parameter:</em> "
			" <pre>"
			"    -o page-right=...      # use values from \"0\" or higher. \"72\" is equal to 1 inch. "
			" </pre>"
			" </qt>" );

	QString whatsThisMeasurementUnitMarginWidget = i18n( " <qt> "
			" <p><b>Change Measurement Unit<b></p>. "
			" <p>You can change the units of measurement for the page"
			" margins here. Select from Millimeter, Centimeter, Inch or Pixels (1 pixel == 1/72 inch). "
			" </p> "
			" </qt>" );

	QString whatsThisCheckboxMarginWidget = i18n( " <qt> "
			" <p><b>Custom Margins Checkbox</b></p>. "
			" <p>Enable this checkbox if you want to modify the margins of your printouts "
			" <p>You can change margin settings in 4 ways: "
			" <ul> "
			" <li>Edit the text fields. </li> "
			" <li>Click spinbox arrows. </li> "
			" <li>Scroll wheel of wheelmouses. </li> "
			" <li>Drag margins in preview frame with mouse. </li> "
			" </ul> "
			" <b>Note:</b> The margin setting does not work if you load such files directly into "
			" kprinter, which have their print margins hardcoded internally, like as most "
			" PDF or PostScript files. It works for all ASCII text files however. It also may not "
			" work with non-KDE applications which fail to "
			" fully utilize the KDEPrint framework, such as OpenOffice.org. </p> "
			" </qt>" );

	QString whatsThisDragAndPreviewMarginWidget = i18n( " <qt> "
			" <p><b>\"Drag-your-Margins\" </p>. "
			" <p>Use your mouse to drag and set each margin on this little preview window. </p> "
			" </qt>" );

	m_symetric = m_block = false;
	m_pagesize[ 0 ] = 595;
	m_pagesize[ 1 ] = 842;
	m_landscape = false;

	m_custom = new QCheckBox(i18n("&Use custom margins"), this);
	  m_custom->setWhatsThis(whatsThisCheckboxMarginWidget);
	m_top = new MarginValueWidget(0, 0.0, this);
	  m_top->setWhatsThis(whatsThisTopMarginWidget);
	m_bottom = new MarginValueWidget(m_top, 0.0, this);
	  m_bottom->setWhatsThis(whatsThisBottomMarginWidget);
	m_left = new MarginValueWidget(m_bottom, 0.0, this);
	  m_left->setWhatsThis(whatsThisLeftMarginWidget);
	m_right = new MarginValueWidget(m_left, 0.0, this);
	  m_right->setWhatsThis(whatsThisRightMarginWidget);
	m_top->setLabel(i18n("&Top:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_bottom->setLabel(i18n("&Bottom:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_left->setLabel(i18n("Le&ft:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_right->setLabel(i18n("&Right:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_units = new QComboBox(this);
	  m_units->setWhatsThis(whatsThisMeasurementUnitMarginWidget);
	m_units->addItem(i18n("Pixels (1/72nd in)"));
	if ( allowMetricUnit )
	{
		m_units->addItem(i18n("Inches (in)"));
		m_units->addItem(i18n("Centimeters (cm)"));
		m_units->addItem( i18n( "Millimeters (mm)" ) );
	}
	m_units->setCurrentIndex(0);
	connect(m_units, SIGNAL(activated(int)), m_top, SLOT(setMode(int)));
	connect(m_units, SIGNAL(activated(int)), m_bottom, SLOT(setMode(int)));
	connect(m_units, SIGNAL(activated(int)), m_left, SLOT(setMode(int)));
	connect(m_units, SIGNAL(activated(int)), m_right, SLOT(setMode(int)));
	m_preview = new MarginPreview(this);
	  m_preview->setWhatsThis(whatsThisDragAndPreviewMarginWidget);
	m_preview->setMinimumSize(60, 80);
	m_preview->setPageSize(m_pagesize[ 0 ], m_pagesize[ 1 ]);
	connect(m_preview, SIGNAL(marginChanged(int,float)), SLOT(slotMarginPreviewChanged(int,float)));
	connect(m_top, SIGNAL(marginChanged(float)), SLOT(slotMarginValueChanged()));
	connect(m_bottom, SIGNAL(marginChanged(float)), SLOT(slotMarginValueChanged()));
	connect(m_left, SIGNAL(marginChanged(float)), SLOT(slotMarginValueChanged()));
	connect(m_right, SIGNAL(marginChanged(float)), SLOT(slotMarginValueChanged()));
	slotMarginValueChanged();
	connect(m_custom, SIGNAL(toggled(bool)), m_top, SLOT(setEnabled(bool)));
	connect(m_custom, SIGNAL(toggled(bool)), m_left, SLOT(setEnabled(bool)));
	//connect(m_custom, SIGNAL(toggled(bool)), m_units, SLOT(setEnabled(bool)));
	connect(m_custom, SIGNAL(toggled(bool)), SLOT(slotCustomMarginsToggled(bool)));
	connect(m_custom, SIGNAL(toggled(bool)), m_preview, SLOT(enableRubberBand(bool)));
	m_top->setEnabled(false);
	m_bottom->setEnabled(false);
	m_left->setEnabled(false);
	m_right->setEnabled(false);
	//m_units->setEnabled(false);

	QGridLayout	*l3 = new QGridLayout(this);
  l3->setMargin(0);
  l3->setSpacing(10);
	l3->addWidget(m_custom, 0, 0);
	l3->addWidget(m_top, 1, 0);
	l3->addWidget(m_bottom, 2, 0);
	l3->addWidget(m_left, 3, 0);
	l3->addWidget(m_right, 4, 0);
	l3->addWidget(m_units, 6, 0);
	l3->addWidget(m_preview, 0, 6, 1, 1);

	if ( allowMetricUnit )
	{
		int	mode = (KGlobal::locale()->measureSystem() == KLocale::Metric ? 2 : 1);
		m_top->setMode(mode);
		m_bottom->setMode(mode);
		m_left->setMode(mode);
		m_right->setMode(mode);
		m_units->setCurrentIndex(mode);
	}
}

MarginWidget::~MarginWidget()
{
}

void MarginWidget::slotCustomMarginsToggled(bool b)
{
	m_bottom->setEnabled(b && !m_symetric);
	m_right->setEnabled(b && !m_symetric);
	if (!b)
		resetDefault();
}

void MarginWidget::setSymetricMargins(bool on)
{
	if (on == m_symetric)
		return;

	m_symetric = on;
	m_bottom->setEnabled(on && m_custom->isChecked());
	m_right->setEnabled(on && m_custom->isChecked());
	if (on)
	{
		connect(m_top, SIGNAL(marginChanged(float)), m_bottom, SLOT(setMargin(float)));
		connect(m_left, SIGNAL(marginChanged(float)), m_right, SLOT(setMargin(float)));
		m_bottom->setMargin(m_top->margin());
		m_right->setMargin(m_left->margin());
	}
	else
	{
		disconnect(m_top, 0, m_bottom, 0);
		disconnect(m_left, 0, m_right, 0);
	}
	m_preview->setSymetric(on);
}

void MarginWidget::slotMarginValueChanged()
{
	if (m_block)
		return;
	m_preview->setMargins(m_top->margin(), m_bottom->margin(), m_left->margin(), m_right->margin());
}

void MarginWidget::slotMarginPreviewChanged(int type, float value)
{
	m_block = true;
	switch (type)
	{
		case MarginPreview::TMoving:
			m_top->setMargin(value);
			break;
		case MarginPreview::BMoving:
			m_bottom->setMargin(value);
			break;
		case MarginPreview::LMoving:
			m_left->setMargin(value);
			break;
		case MarginPreview::RMoving:
			m_right->setMargin(value);
			break;
	}
	m_block = false;
}

void MarginWidget::setPageSize(float w, float h)
{
	// takes care of the orientation and the resolution
	int	dpi = m_top->resolution();
	m_pagesize[ 0 ] = w;
	m_pagesize[ 1 ] = h;
	if (m_landscape)
		m_preview->setPageSize((m_pagesize[ 1 ]*dpi)/72, (m_pagesize[ 0 ]*dpi)/72);
	else
		m_preview->setPageSize((m_pagesize[ 0 ]*dpi)/72, (m_pagesize[ 1 ]*dpi)/72);
}

float MarginWidget::top() const
{
	return m_top->margin();
}

float MarginWidget::bottom() const
{
	return m_bottom->margin();
}

float MarginWidget::left() const
{
	return m_left->margin();
}

float MarginWidget::right() const
{
	return m_right->margin();
}

void MarginWidget::setTop(float value)
{
	m_top->setMargin(value);
}

void MarginWidget::setBottom(float value)
{
	m_bottom->setMargin(value);
}

void MarginWidget::setLeft(float value)
{
	m_left->setMargin(value);
}

void MarginWidget::setRight(float value)
{
	m_right->setMargin(value);
}

void MarginWidget::setResolution(int dpi)
{
	m_top->setResolution(dpi);
	m_bottom->setResolution(dpi);
	m_left->setResolution(dpi);
	m_right->setResolution(dpi);
}

void MarginWidget::setDefaultMargins(float t, float b, float l, float r)
{
	int	dpi = m_top->resolution();
	m_default[0] = (t*dpi)/72;
	m_default[1] = (b*dpi)/72;
	m_default[2] = (l*dpi)/72;
	m_default[3] = (r*dpi)/72;
	if (!m_custom->isChecked())
		resetDefault();
}

void MarginWidget::resetDefault()
{
	m_top->setMargin(m_landscape ? m_default[2] : m_default[0]);
	m_bottom->setMargin(m_landscape ? m_default[3] : m_default[1]);
	m_left->setMargin(m_landscape ? m_default[1] : m_default[2]);
	m_right->setMargin(m_landscape ? m_default[0] : m_default[3]);
}

void MarginWidget::setCustomEnabled(bool on)
{
	m_custom->setChecked(on);
}

bool MarginWidget::isCustomEnabled() const
{
	return m_custom->isChecked();
}

void MarginWidget::setOrientation(int orient)
{
	m_landscape = (orient == KPrinter::Landscape);
	setPageSize(m_pagesize[ 0 ], m_pagesize[ 1 ]);
}

#include "marginwidget.moc"
