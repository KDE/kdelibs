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
#include "driver.h"
#include "kprinter.h"

#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <knuminput.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kseparator.h>
#include <kdebug.h>

KPTextPage::KPTextPage(DrMain *driver, QWidget *parent, const char *name)
: KPrintDialogPage(0, driver, parent, name)
{
	setTitle(i18n("Text"));
	m_block = false;

	QGroupBox	*formatbox = new QGroupBox(0, Qt::Vertical, i18n("Text Format"), this);
	QGroupBox	*prettybox = new QGroupBox(0, Qt::Vertical, i18n("Syntax Highlighting"), this);
	QGroupBox	*marginbox = new QGroupBox(0, Qt::Vertical, i18n("Margins"), this);

	m_cpi = new KIntNumInput(10, formatbox);
	m_cpi->setLabel(i18n("&Chars per inch:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_cpi->setRange(1, 999, 1, false);
	m_lpi = new KIntNumInput(m_cpi, 6, formatbox);
	m_lpi->setLabel(i18n("&Lines per inch:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_lpi->setRange(1, 999, 1, false);
	m_columns = new KIntNumInput(m_lpi, 1, formatbox);
	m_columns->setLabel(i18n("C&olumns:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_columns->setRange(1, 10, 1, false);
	KSeparator	*sep = new KSeparator(Qt::Horizontal, formatbox);
	connect(m_columns, SIGNAL(valueChanged(int)), SLOT(slotColumnsChanged(int)));

	m_prettypix = new QLabel(prettybox);
	m_prettypix->setAlignment(Qt::AlignCenter);
	QRadioButton	*off = new QRadioButton(i18n("&Disabled"), prettybox);
	QRadioButton	*on = new QRadioButton(i18n("&Enabled"), prettybox);
	m_prettyprint = new QButtonGroup(prettybox);
	m_prettyprint->hide();
	m_prettyprint->insert(off, 0);
	m_prettyprint->insert(on, 1);
	m_prettyprint->setButton(0);
	connect(m_prettyprint, SIGNAL(clicked(int)), SLOT(slotPrettyChanged(int)));
	slotPrettyChanged(0);

	m_margin = new MarginWidget(marginbox);
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
	if (!(value=opts["page-top"]).isEmpty() && value.toInt() != m_margin->top())
	{
		marginset = true;
		m_margin->setTop(value.toInt());
	}
	if (!(value=opts["page-bottom"]).isEmpty() && value.toInt() != m_margin->bottom())
	{
		marginset = true;
		m_margin->setBottom(value.toInt());
	}
	if (!(value=opts["page-left"]).isEmpty() && value.toInt() != m_margin->left())
	{
		marginset = true;
		m_margin->setLeft(value.toInt());
	}
	if (!(value=opts["page-right"]).isEmpty() && value.toInt() != m_margin->right())
	{
		marginset = true;
		m_margin->setRight(value.toInt());
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
		opts["page-top"] = QString::number(m_margin->top());
		opts["page-bottom"] = QString::number(m_margin->bottom());
		opts["page-left"] = QString::number(m_margin->left());
		opts["page-right"] = QString::number(m_margin->right());
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

void KPTextPage::slotColumnsChanged(int c)
{
	// TO BE IMPLEMENTED
}

void KPTextPage::initPageSize(bool landscape)
{
	QSize	sz(-1, -1), mg(18, 36);
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
				mg = ps->margins();
				sz = ps->pageSize();
			}
		}
	}
	m_margin->setPageSize(sz.width(), sz.height());
	m_margin->setOrientation(landscape ? KPrinter::Landscape : KPrinter::Portrait);
	m_margin->setDefaultMargins(mg.height(), mg.height(), mg.width(), mg.width());
	m_margin->setCustomEnabled(false);
}

#include "kptextpage.moc"
