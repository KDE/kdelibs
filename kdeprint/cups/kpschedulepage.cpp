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

#include "kpschedulepage.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qregexp.h>
#include <qdatetimeedit.h>
#include <qdatetime.h>
#include <qlineedit.h>
#include <klocale.h>
#include <kseparator.h>
#include <knuminput.h>

#include <time.h>

KPSchedulePage::KPSchedulePage(QWidget *parent, const char *name)
: KPrintDialogPage(parent, name)
{
	setTitle(i18n("Advanced Options"));
	setOnlyRealPrinters(true);

	// compute difference in hours between GMT and local time
	time_t	ct = time(0);
	struct tm	*ts = gmtime(&ct);
	m_gmtdiff = ts->tm_hour;
	ts = localtime(&ct);
	m_gmtdiff -= ts->tm_hour;

	m_time = new QComboBox(this);
	m_time->insertItem(i18n("Immediately"));
	m_time->insertItem(i18n("Never (hold indefinitely)"));
	m_time->insertItem(i18n("Daytime (6 am - 6 pm)"));
	m_time->insertItem(i18n("Evening (6 pm - 6 am)"));
	m_time->insertItem(i18n("Night (6 pm - 6 am)"));
	m_time->insertItem(i18n("Weekend"));
	m_time->insertItem(i18n("Second Shift (4 pm - 12 am)"));
	m_time->insertItem(i18n("Third Shift (12 am - 8 am)"));
	m_time->insertItem(i18n("Specified Time"));
	m_tedit = new QTimeEdit(this);
	m_tedit->setAutoAdvance(true);
	m_tedit->setTime(QTime::currentTime());
	m_tedit->setEnabled(false);
	m_billing = new QLineEdit(this);
	m_pagelabel = new QLineEdit(this);
	m_priority = new KIntNumInput(50, this);
	m_priority->setRange(1, 100, 10, true);

	QLabel	*lab = new QLabel(i18n("&Schedule printing:"), this);
	lab->setBuddy(m_time);
	QLabel	*lab1 = new QLabel(i18n("&Billing information:"), this);
	lab1->setBuddy(m_billing);
	QLabel	*lab2 = new QLabel(i18n("T&op/Bottom page label:"), this);
	lab2->setBuddy(m_pagelabel);
	m_priority->setLabel(i18n("&Job priority:"), Qt::AlignVCenter|Qt::AlignLeft);

	KSeparator	*sep0 = new KSeparator(this);
	sep0->setFixedHeight(10);

	QGridLayout	*l0 = new QGridLayout(this, 6, 2, 0, 7);
	l0->addWidget(lab, 0, 0);
	QHBoxLayout	*l1 = new QHBoxLayout(0, 0, 5);
	l0->addLayout(l1, 0, 1);
	l1->addWidget(m_time);
	l1->addWidget(m_tedit);
	l0->addWidget(lab1, 1, 0);
	l0->addWidget(lab2, 2, 0);
	l0->addWidget(m_billing, 1, 1);
	l0->addWidget(m_pagelabel, 2, 1);
	l0->addMultiCellWidget(sep0, 3, 3, 0, 1);
	l0->addMultiCellWidget(m_priority, 4, 4, 0, 1);
	l0->setRowStretch(5, 1);

	connect(m_time, SIGNAL(activated(int)), SLOT(slotTimeChanged()));
}

KPSchedulePage::~KPSchedulePage()
{
}

bool KPSchedulePage::isValid(QString& msg)
{
	if (m_time->currentItem() == 8 && !m_tedit->time().isValid())
	{
		msg = i18n("The time specified is not valid.");
		return false;
	}
	return true;
}

void KPSchedulePage::setOptions(const QMap<QString,QString>& opts)
{
	QString	t = opts["job-hold-until"];
	if (!t.isEmpty())
	{
		int	item(-1);

		if (t == "no-hold") item = 0;
		else if (t == "indefinite") item = 1;
		else if (t == "day-time") item = 2;
		else if (t == "evening") item = 3;
		else if (t == "night") item = 4;
		else if (t == "weekend") item = 5;
		else if (t == "second-shift") item = 6;
		else if (t == "third-shift") item = 7;
		else
		{
			QTime	qt = QTime::fromString(t);
			m_tedit->setTime(qt.addSecs(-3600 * m_gmtdiff));
			item = 8;
		}

		if (item != -1)
		{
			m_time->setCurrentItem(item);
			slotTimeChanged();
		}
	}
	QRegExp	re("^\"|\"$");
	t = opts["job-billing"].stripWhiteSpace();
	t.replace(re, "");
	m_billing->setText(t);
	t = opts["page-label"].stripWhiteSpace();
	t.replace(re, "");
	m_pagelabel->setText(t);
	int	val = opts["job-priority"].toInt();
	if (val != 0)
		m_priority->setValue(val);
}

void KPSchedulePage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	if (incldef || m_time->currentItem() != 0)
	{
		QString	t;
		switch (m_time->currentItem())
		{
			case 0: t = "no-hold"; break;
			case 1: t = "indefinite"; break;
			case 2: t = "day-time"; break;
			case 3: t = "evening"; break;
			case 4: t = "night"; break;
			case 5: t = "weekend"; break;
			case 6: t = "second-shift"; break;
			case 7: t = "third-shift"; break;
			case 8:
				t = m_tedit->time().addSecs(3600 * m_gmtdiff).toString();
				break;
		}
		opts["job-hold-until"] = t;
	}
	if (incldef || !m_billing->text().isEmpty())
		opts["job-billing"] = "\"" + m_billing->text() + "\"";
	if (incldef || !m_pagelabel->text().isEmpty())
		opts["page-label"] = "\"" + m_pagelabel->text() + "\"";
	if (incldef || m_priority->value() != 50)
		opts["job-priority"] = QString::number(m_priority->value());
}

void KPSchedulePage::slotTimeChanged()
{
	m_tedit->setEnabled(m_time->currentItem() == 8);
	if (m_time->currentItem() == 8)
		m_tedit->setFocus();
}

#include "kpschedulepage.moc"
