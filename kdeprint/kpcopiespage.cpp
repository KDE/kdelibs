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

#include "kpcopiespage.h"
#include "kmfactory.h"
#include "kmuimanager.h"
#include "kprinter.h"

#include <qlabel.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kseparator.h>

KPCopiesPage::KPCopiesPage(KPrinter *prt, QWidget *parent, const char *name)
: KPrintDialogPage(parent,name)
{
	m_printer = prt;

	setTitle(i18n("Copies"));
	setId(KPrinter::CopiesPage);

	// widget creation
	QButtonGroup	*m_pagebox = new QButtonGroup(0, Qt::Vertical, i18n("Page selection"), this);
	m_all = new QRadioButton(i18n("All"), m_pagebox);
	m_current = new QRadioButton(i18n("Current"), m_pagebox);
	m_range = new QRadioButton(i18n("Range"), m_pagebox);
	m_rangeedit = new QLineEdit(m_pagebox);
	QToolTip::add(m_rangeedit, i18n("<p>Enter pages or group of pages to print separated by commas (1,2-5,8).</p>"));
	QWhatsThis::add(m_rangeedit, i18n("<p>Enter pages or group of pages to print separated by commas (1,2-5,8).</p>"));
	//QLabel	*m_rangeexpl = new QLabel(m_pagebox);
	//m_rangeexpl->setText(i18n("<p>Enter pages or group of pages to print separated by commas (1,2-5,8).</p>"));
	QGroupBox	*m_copybox = new QGroupBox(0, Qt::Vertical, i18n("Copies"), this);
	m_collate = new QCheckBox(i18n("Collate"), m_copybox);
	m_order = new QCheckBox(i18n("Reverse"), m_copybox);
	m_collatepix = new QLabel(m_copybox);
	m_collatepix->setAlignment(Qt::AlignCenter);
	m_collatepix->setMinimumHeight(70);
	QLabel	*m_copieslabel = new QLabel(i18n("Copies:"), m_copybox);
	m_copies = new QSpinBox(m_copybox);
	m_copies->setRange(1,999);
	m_pageset = new QComboBox(m_pagebox);
	m_pageset->insertItem(i18n("All pages"));
	m_pageset->insertItem(i18n("Odd pages"));
	m_pageset->insertItem(i18n("Even pages"));
	QLabel	*m_pagesetlabel = new QLabel(i18n("Page set:"), m_pagebox);
	KSeparator	*sepline = new KSeparator(Horizontal, m_pagebox);
	sepline->setMinimumHeight(10);

	// layout creation
	QGridLayout	*l1 = new QGridLayout(this, 1, 2, 0, 5);
	l1->setRowStretch(0,1);
	l1->setColStretch(0,1);
	l1->setColStretch(1,1);
	l1->addWidget(m_pagebox,0,0);
	l1->addWidget(m_copybox,0,1);
	QVBoxLayout	*l3 = new QVBoxLayout(m_pagebox->layout(), 5);
	l3->addWidget(m_all);
	l3->addWidget(m_current);
	QHBoxLayout	*l4 = new QHBoxLayout(0, 0, 5);
	l3->addLayout(l4);
	l4->addWidget(m_range,0);
	l4->addWidget(m_rangeedit,1);
	//l3->addWidget(m_rangeexpl);
	l3->addWidget(sepline);
	QHBoxLayout	*l2 = new QHBoxLayout(0, 0, 5);
	l3->addLayout(l2);
	l2->addWidget(m_pagesetlabel,0);
	l2->addWidget(m_pageset,1);
	QGridLayout	*l5 = new QGridLayout(m_copybox->layout(), 4, 2, 10);
	l5->setRowStretch(4,1);
	l5->addWidget(m_copieslabel,0,0);
	l5->addWidget(m_copies,0,1);
	l5->addMultiCellWidget(m_collatepix,1,2,0,0);
	l5->addWidget(m_collate,1,1);
	l5->addWidget(m_order,2,1);

	// some initialization
	m_all->setChecked(true);
	m_copies->setValue(1);
	initialize();
	slotCollateClicked();

	// connections
	connect(m_rangeedit,SIGNAL(textChanged(const QString&)),SLOT(slotRangeEntered()));
	connect(m_collate,SIGNAL(clicked()),SLOT(slotCollateClicked()));
	connect(m_order,SIGNAL(clicked()),SLOT(slotCollateClicked()));
}

KPCopiesPage::~KPCopiesPage()
{
}

void KPCopiesPage::slotRangeEntered()
{
	m_range->setChecked(true);
}

void KPCopiesPage::slotCollateClicked()
{
	QString	s("kdeprint_");
	s.append((m_collate->isChecked() ? "collate" : "uncollate"));
	if (m_order->isChecked()) s.append("_reverse");
	m_collatepix->setPixmap(UserIcon(s));
}

void KPCopiesPage::initialize()
{
	int	f = KMFactory::self()->uiManager()->copyFlags(m_printer);

	m_current->setEnabled((f & KMUiManager::Current));
	m_range->setEnabled((f & KMUiManager::Range));
	m_rangeedit->setEnabled((f & KMUiManager::Range));
	m_collate->setEnabled((f & KMUiManager::Collate));
	m_order->setEnabled((f & KMUiManager::Order));
	m_pageset->setEnabled((f & KMUiManager::PageSet));

	// by default, if collate disabled, set it to true
	m_collate->setChecked(!(f & KMUiManager::NoAutoCollate));
	slotCollateClicked();
}

void KPCopiesPage::setOptions(const QMap<QString,QString>& options)
{
	QString	value;
	// copies
	value = options["kde-copies"];
	if (!value.isEmpty()) m_copies->setValue(value.toInt());
	// output order
	value = options["kde-pageorder"];
	if (m_order->isEnabled()) m_order->setChecked(value == "Reverse");
	// collate
	value = options["kde-collate"];
	if (m_collate->isEnabled()) m_collate->setChecked(value == "Collate");
	// update pixmap
	slotCollateClicked();
	// page ranges
	value = options["kde-range"];
	if (!value.isEmpty() && m_range->isEnabled())
	{
		m_rangeedit->setText(value);
		m_range->setChecked(true);
	}
	else if (options["kde-current"] == "1")
		m_current->setChecked(true);
	else
		m_all->setChecked(true);
	// page set
	value = options["kde-pageset"];
	if (!value.isEmpty() && m_pageset->isEnabled())
		m_pageset->setCurrentItem(value.toInt());
	else
		m_pageset->setCurrentItem(0);
}

void KPCopiesPage::getOptions(QMap<QString,QString>& options, bool)
{
	// copies
	options["kde-copies"] = m_copies->text();
	// output order
	options["kde-pageorder"] = (m_order->isChecked() ? "Reverse" : "Forward");
	// collate
	options["kde-collate"] = (m_collate->isChecked() ? "Collate" : "Uncollate");
	// ranges
	options["kde-current"] = (m_current->isChecked() ? "1" : "0");
	options["kde-range"] = (m_range->isChecked() ? m_rangeedit->text() : QString::fromLatin1(""));
	// page set
	options["kde-pageset"] = QString::number(m_pageset->currentItem());
}

void KPCopiesPage::reload()
{
	initialize();
}

#include "kpcopiespage.moc"
