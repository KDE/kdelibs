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

#include "kphpgl2page.h"

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <klocale.h>
#include <knuminput.h>

KPHpgl2Page::KPHpgl2Page(QWidget *parent, const char *name)
: KPrintDialogPage(parent, name)
{
	setTitle("HP-GL/2");

	QGroupBox	*box = new QGroupBox(0, Qt::Vertical, i18n("HP-GL/2 Options"), this);

	m_blackplot = new QCheckBox(i18n("&Use only black pen"), box);
	m_fitplot = new QCheckBox(i18n("&Fit plot to page"), box);
	m_penwidth = new KIntNumInput(1000, box);
	m_penwidth->setLabel(i18n("&Pen width:"), Qt::AlignLeft|Qt::AlignVCenter);
	m_penwidth->setSuffix(" [um]");
	m_penwidth->setRange(0, 10000, 100, true);

	QVBoxLayout	*l0 = new QVBoxLayout(this, 10, 10);
	l0->addWidget(box);
	l0->addStretch(1);
	QVBoxLayout	*l1 = new QVBoxLayout(box->layout(), 10);
	l1->addWidget(m_blackplot);
	l1->addWidget(m_fitplot);
	l1->addWidget(m_penwidth);
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
	else
		opts.remove("blackplot");
	if (m_fitplot->isChecked())
		opts["fitplot"] = "true";
	else
		opts.remove("fitplot");
}
