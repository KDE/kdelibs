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

#include "kptextpage.h"
#include "marginwidget.h"
#include "marginpreview.h"
#include "driver.h"

#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <knuminput.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kseparator.h>

KPTextPage::KPTextPage(DrMain *driver, QWidget *parent, const char *name)
: KPrintDialogPage(0, driver, parent, name)
{
	setTitle(i18n("Text"));

	QGroupBox	*formatbox = new QGroupBox(0, Qt::Vertical, i18n("Text Format"), this);
	QGroupBox	*prettybox = new QGroupBox(0, Qt::Vertical, i18n("Syntax Highlighting"), this);
	QGroupBox	*marginbox = new QGroupBox(0, Qt::Vertical, i18n("Margins"), this);

	m_cpi = new KIntNumInput(10, formatbox);
	m_cpi->setLabel(i18n("Chars per inch:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_cpi->setRange(1, 999, 1, false);
	m_lpi = new KIntNumInput(m_cpi, 6, formatbox);
	m_lpi->setLabel(i18n("Lines per inch:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_lpi->setRange(1, 999, 1, false);
	m_columns = new KIntNumInput(m_lpi, 1, formatbox);
	m_columns->setLabel(i18n("Columns"), Qt::AlignLeft|Qt::AlignVCenter);
	m_columns->setRange(1, 10, 1, false);
	KSeparator	*sep = new KSeparator(Qt::Horizontal, formatbox);
	connect(m_columns, SIGNAL(valueChanged(int)), SLOT(slotColumnsChanged(int)));

	m_prettypix = new QLabel(prettybox);
	m_prettypix->setAlignment(Qt::AlignCenter);
	QRadioButton	*off = new QRadioButton(i18n("Disabled"), prettybox);
	QRadioButton	*on = new QRadioButton(i18n("Enabled"), prettybox);
	m_prettyprint = new QButtonGroup(prettybox);
	m_prettyprint->hide();
	m_prettyprint->insert(off, 0);
	m_prettyprint->insert(on, 1);
	m_prettyprint->setButton(0);
	connect(m_prettyprint, SIGNAL(clicked(int)), SLOT(slotPrettyChanged(int)));
	slotPrettyChanged(0);

	m_custom = new QCheckBox(i18n("Use custom margins"), marginbox);
	m_top = new MarginWidget(0, 36.0, marginbox);
	m_bottom = new MarginWidget(m_top, 36.0, marginbox);
	m_left = new MarginWidget(m_bottom, 18.0, marginbox);
	m_right = new MarginWidget(m_left, 18.0, marginbox);
	m_top->setLabel(i18n("Top:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_bottom->setLabel(i18n("Bottom:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_left->setLabel(i18n("Left:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_right->setLabel(i18n("Right:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_units = new QComboBox(marginbox);
	m_units->insertItem(i18n("Pixels"));
	m_units->insertItem(i18n("Inches (in)"));
	m_units->insertItem(i18n("Centimeters (cm)"));
	m_units->setCurrentItem(0);
	connect(m_units, SIGNAL(activated(int)), m_top, SLOT(setMode(int)));
	connect(m_units, SIGNAL(activated(int)), m_bottom, SLOT(setMode(int)));
	connect(m_units, SIGNAL(activated(int)), m_left, SLOT(setMode(int)));
	connect(m_units, SIGNAL(activated(int)), m_right, SLOT(setMode(int)));
	m_preview = new MarginPreview(marginbox);
	m_preview->setPageSize(595, 842);
	connect(m_top, SIGNAL(marginChanged(int)), SLOT(slotMarginChanged()));
	connect(m_bottom, SIGNAL(marginChanged(int)), SLOT(slotMarginChanged()));
	connect(m_left, SIGNAL(marginChanged(int)), SLOT(slotMarginChanged()));
	connect(m_right, SIGNAL(marginChanged(int)), SLOT(slotMarginChanged()));
	slotMarginChanged();
	connect(m_custom, SIGNAL(toggled(bool)), m_top, SLOT(setEnabled(bool)));
	connect(m_custom, SIGNAL(toggled(bool)), m_bottom, SLOT(setEnabled(bool)));
	connect(m_custom, SIGNAL(toggled(bool)), m_left, SLOT(setEnabled(bool)));
	connect(m_custom, SIGNAL(toggled(bool)), m_right, SLOT(setEnabled(bool)));
	connect(m_custom, SIGNAL(toggled(bool)), m_units, SLOT(setEnabled(bool)));
	m_top->setEnabled(false);
	m_bottom->setEnabled(false);
	m_left->setEnabled(false);
	m_right->setEnabled(false);
	m_units->setEnabled(false);

	QGridLayout	*l0 = new QGridLayout(this, 2, 2, 10, 10);
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
	QGridLayout	*l3 = new QGridLayout(marginbox->layout(), 7, 2, 10);
	l3->addWidget(m_custom, 0, 0);
	l3->addWidget(m_top, 1, 0);
	l3->addWidget(m_bottom, 2, 0);
	l3->addWidget(m_left, 3, 0);
	l3->addWidget(m_right, 4, 0);
	l3->addRowSpacing(5, 10);
	l3->addWidget(m_units, 6, 0);
	l3->addMultiCellWidget(m_preview, 0, 6, 1, 1);
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
	DrMain	*dr = driver();
	QString	psname = opts["PageSize"];
	if (dr)
	{
		if (psname.isEmpty())
		{
			DrListOption	*o = (DrListOption*)dr->findOption("PageSize");
			if (o)
				psname = o->get("default");
		}
		if (!psname.isEmpty())
		{
			DrPageSize	*ps = dr->findPageSize(psname);
			if (ps)
			{
				QSize	sz = ps->pageSize();
				m_preview->setNoPreview(false);
				m_preview->setPageSize(sz.width(), sz.height());
				sz = ps->margins();
				m_top->setMargin(sz.height());
				m_bottom->setMargin(sz.height());
				m_left->setMargin(sz.width());
				m_right->setMargin(sz.width());
				slotMarginChanged();
			}
			else
				m_preview->setNoPreview(true);
		}
	}
	else
		m_preview->setNoPreview(true);

	bool	marginset(false);
	if (!(value=opts["page-top"]).isEmpty())
	{
		marginset = true;
		m_top->setMargin(value.toInt());
	}
	if (!(value=opts["page-bottom"]).isEmpty())
	{
		marginset = true;
		m_bottom->setMargin(value.toInt());
	}
	if (!(value=opts["page-left"]).isEmpty())
	{
		marginset = true;
		m_left->setMargin(value.toInt());
	}
	if (!(value=opts["page-right"]).isEmpty())
	{
		marginset = true;
		m_right->setMargin(value.toInt());
	}
	m_custom->setChecked(marginset);
	slotMarginChanged();
}

void KPTextPage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	if (incldef || m_cpi->value() != 10)
		opts["cpi"] = QString::number(m_cpi->value());
	if (incldef || m_lpi->value() != 6)
		opts["lpi"] = QString::number(m_lpi->value());
	if (incldef || m_columns->value() != 1)
		opts["columns"] = QString::number(m_columns->value());

	if (m_custom->isChecked())
	{
		opts["page-top"] = QString::number(m_top->margin());
		opts["page-bottom"] = QString::number(m_bottom->margin());
		opts["page-left"] = QString::number(m_left->margin());
		opts["page-right"] = QString::number(m_right->margin());
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
	else
		opts.remove("prettyprint");
}

void KPTextPage::slotMarginChanged()
{
	int	t(m_top->margin()), b(m_bottom->margin()), l(m_left->margin()), r(m_right->margin());
	m_preview->setMargins(t, b, l, r);
}

void KPTextPage::slotPrettyChanged(int ID)
{
	QString	iconstr = (ID == 0 ? "kdeprint_nup1" : "kdeprint_prettyprint");
	m_prettypix->setPixmap(UserIcon(iconstr));
}

void KPTextPage::slotColumnsChanged(int c)
{
	// TO BE IMPLEMENTED
}

#include "kptextpage.moc"
