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

#include <config.h>

#include "kmwquota.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <qspinbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <klocale.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#define	N_TIME_LIMITS	6
static int time_periods[] = {
	1,		// second
	60,		// minute
	3600,		// hour
	86400,		// day
	604800,		// week
	2592000		// month (30 days)
};
static const char* time_keywords[] = {
	I18N_NOOP("second(s)"),
	I18N_NOOP("minute(s)"),
	I18N_NOOP("hour(s)"),
	I18N_NOOP("day(s)"),
	I18N_NOOP("week(s)"),
	I18N_NOOP("month(s)")
};

int findUnit(int& period)
{
	int	unit(0);
	for (int i=N_TIME_LIMITS-1;i>=0;i--)
	{
		if (period < time_periods[i])
			continue;
		int	d = period / time_periods[i];
		if ((d*time_periods[i]) == period)
		{
			unit = i;
			break;
		}
	}
	period /= time_periods[unit];
	return unit;
}

const char* unitKeyword(int i)
{ return time_keywords[i]; }

KMWQuota::KMWQuota(QWidget *parent, const char *name)
: KMWizardPage(parent, name)
{
	m_ID = KMWizard::Custom+3;
	m_title = i18n("Printer quota settings");
	m_nextpage = KMWizard::Custom+4;

	m_period = new QSpinBox(this);
	m_period->setRange(-1, INT_MAX);
	m_period->setSpecialValueText(i18n("No quota"));
	m_sizelimit = new QSpinBox(this);
	m_sizelimit->setRange(0, INT_MAX);
	m_sizelimit->setSpecialValueText(i18n("None"));
	m_pagelimit = new QSpinBox(this);
	m_pagelimit->setRange(0, INT_MAX);
	m_pagelimit->setSpecialValueText(i18n("None"));
	m_timeunit = new QComboBox(this);
	for (int i=0;i<N_TIME_LIMITS;i++)
		m_timeunit->insertItem(i18n(time_keywords[i]));
	m_timeunit->setCurrentItem(3);

	QLabel	*lab1 = new QLabel(i18n("&Period:"), this);
	QLabel	*lab2 = new QLabel(i18n("&Size limit (KB):"), this);
	QLabel	*lab3 = new QLabel(i18n("&Page limit:"), this);

	lab1->setBuddy(m_period);
	lab2->setBuddy(m_sizelimit);
	lab3->setBuddy(m_pagelimit);

	QLabel	*lab4 = new QLabel(i18n("<p>Set here the quota for this printer. Using limits of <b>0</b> means "
					"that no quota will be used. This is equivalent to set quota period to "
					"<b><nobr>No quota</nobr></b> (-1). Quota limits are defined on a per-user base and "
					"applied to all users.</p>"), this);

	QGridLayout	*l0 = new QGridLayout(this, 5, 3, 0, 10);
	l0->setRowStretch(4, 1);
	l0->setColStretch(1, 1);
	l0->addMultiCellWidget(lab4, 0, 0, 0, 2);
	l0->addWidget(lab1, 1, 0);
	l0->addWidget(lab2, 2, 0);
	l0->addWidget(lab3, 3, 0);
	l0->addWidget(m_period, 1, 1);
	l0->addWidget(m_timeunit, 1, 2);
	l0->addMultiCellWidget(m_sizelimit, 2, 2, 1, 2);
	l0->addMultiCellWidget(m_pagelimit, 3, 3, 1, 2);
}

KMWQuota::~KMWQuota()
{
}

bool KMWQuota::isValid(QString& msg)
{
	if (m_period->value() >= 0 && m_sizelimit->value() == 0 && m_pagelimit->value() == 0)
	{
		msg = i18n("You must at least specify one quota limit!");
		return false;
	}
	return true;
}

void KMWQuota::initPrinter(KMPrinter *p)
{
	int	qu(-1), si(0), pa(0), un(3);
	qu = p->option("job-quota-period").toInt();
	si = p->option("job-k-limit").toInt();
	pa = p->option("job-page-limit").toInt();
	if (si == 0 && pa == 0)
		// no quota
		qu = -1;
	m_sizelimit->setValue(si);
	m_pagelimit->setValue(pa);
	if (qu > 0)
	{
		un = findUnit(qu);
	}
	m_timeunit->setCurrentItem(un);
	m_period->setValue(qu);
}

void KMWQuota::updatePrinter(KMPrinter *p)
{
	int	qu(m_period->value()), si(m_sizelimit->value()), pa(m_pagelimit->value());
	if (qu == -1)
	{
		// no quota, set limits to 0
		si = 0;
		pa = 0;
		qu = 0;
	}
	qu *= time_periods[m_timeunit->currentItem()];

	p->setOption("job-quota-period", QString::number(qu));
	p->setOption("job-k-limit", QString::number(si));
	p->setOption("job-page-limit", QString::number(pa));
}
#include "kmwquota.moc"
