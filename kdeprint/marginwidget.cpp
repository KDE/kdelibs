/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <goffioul@imec.be>
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

MarginWidget::MarginWidget(QWidget *parent, const char* name, bool allowMetricUnit)
: QWidget(parent, name), m_default(4, 0)
{
	m_symetric = m_block = false;
	m_pagesize = QSize(595, 842);
	m_landscape = false;

	m_custom = new QCheckBox(i18n("&Use custom margins"), this);
	m_top = new MarginValueWidget(0, 0.0, this);
	m_bottom = new MarginValueWidget(m_top, 0.0, this);
	m_left = new MarginValueWidget(m_bottom, 0.0, this);
	m_right = new MarginValueWidget(m_left, 0.0, this);
	m_top->setLabel(i18n("&Top:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_bottom->setLabel(i18n("&Bottom:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_left->setLabel(i18n("Le&ft:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_right->setLabel(i18n("&Right:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_units = new QComboBox(this);
	m_units->insertItem(i18n("Pixels"));
	if ( allowMetricUnit )
	{
		m_units->insertItem(i18n("Inches (in)"));
		m_units->insertItem(i18n("Centimeters (cm)"));
		m_units->insertItem( i18n( "Millimeters (mm)" ) );
	}
	m_units->setCurrentItem(0);
	connect(m_units, SIGNAL(activated(int)), m_top, SLOT(setMode(int)));
	connect(m_units, SIGNAL(activated(int)), m_bottom, SLOT(setMode(int)));
	connect(m_units, SIGNAL(activated(int)), m_left, SLOT(setMode(int)));
	connect(m_units, SIGNAL(activated(int)), m_right, SLOT(setMode(int)));
	m_preview = new MarginPreview(this);
	m_preview->setMinimumSize(60, 80);
	m_preview->setPageSize(m_pagesize.width(), m_pagesize.height());
	connect(m_preview, SIGNAL(marginChanged(int,int)), SLOT(slotMarginPreviewChanged(int,int)));
	connect(m_top, SIGNAL(marginChanged(int)), SLOT(slotMarginValueChanged()));
	connect(m_bottom, SIGNAL(marginChanged(int)), SLOT(slotMarginValueChanged()));
	connect(m_left, SIGNAL(marginChanged(int)), SLOT(slotMarginValueChanged()));
	connect(m_right, SIGNAL(marginChanged(int)), SLOT(slotMarginValueChanged()));
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

	QGridLayout	*l3 = new QGridLayout(this, 7, 2, 0, 10);
	l3->addWidget(m_custom, 0, 0);
	l3->addWidget(m_top, 1, 0);
	l3->addWidget(m_bottom, 2, 0);
	l3->addWidget(m_left, 3, 0);
	l3->addWidget(m_right, 4, 0);
	l3->addRowSpacing(5, 10);
	l3->addWidget(m_units, 6, 0);
	l3->addMultiCellWidget(m_preview, 0, 6, 1, 1);

	if ( allowMetricUnit )
	{
		int	mode = (KGlobal::locale()->measureSystem() == KLocale::Metric ? 2 : 1);
		m_top->setMode(mode);
		m_bottom->setMode(mode);
		m_left->setMode(mode);
		m_right->setMode(mode);
		m_units->setCurrentItem(mode);
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
		connect(m_top, SIGNAL(marginChanged(int)), m_bottom, SLOT(setMargin(int)));
		connect(m_left, SIGNAL(marginChanged(int)), m_right, SLOT(setMargin(int)));
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

void MarginWidget::slotMarginPreviewChanged(int type, int value)
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

void MarginWidget::setPageSize(int w, int h)
{
	// takes care of the orientation and the resolution
	int	dpi = m_top->resolution();
	m_pagesize = QSize(w, h);
	if (m_landscape)
		m_preview->setPageSize((m_pagesize.height()*dpi+36)/72, (m_pagesize.width()*dpi+36)/72);
	else
		m_preview->setPageSize((m_pagesize.width()*dpi+36)/72, (m_pagesize.height()*dpi+36)/72);
}

int MarginWidget::top() const
{
	return m_top->margin();
}

int MarginWidget::bottom() const
{
	return m_bottom->margin();
}

int MarginWidget::left() const
{
	return m_left->margin();
}

int MarginWidget::right() const
{
	return m_right->margin();
}

void MarginWidget::setTop(int value)
{
	m_top->setMargin(value);
}

void MarginWidget::setBottom(int value)
{
	m_bottom->setMargin(value);
}

void MarginWidget::setLeft(int value)
{
	m_left->setMargin(value);
}

void MarginWidget::setRight(int value)
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

void MarginWidget::setDefaultMargins(int t, int b, int l, int r)
{
	int	dpi = m_top->resolution();
	m_default[0] = (t*dpi+36)/72;
	m_default[1] = (b*dpi+36)/72;
	m_default[2] = (l*dpi+36)/72;
	m_default[3] = (r*dpi+36)/72;
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
	setPageSize(m_pagesize.width(), m_pagesize.height());
}

#include "marginwidget.moc"
