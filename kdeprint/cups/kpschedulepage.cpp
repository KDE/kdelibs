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
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qdatetime.h>
#include <qvalidator.h>
#include <klocale.h>

#include <time.h>

int fixHour(int h)
{
	if (h > 23)
		return h % 24;
	else if (h < 0)
		return fixHour(h+24);
	else
		return h;
}

QString num2str(int n)
{
	QString	s;
	s.setNum(n);
	if (s.length() == 1)
		s.prepend("0");
	return s;
}

KPSchedulePage::KPSchedulePage(QWidget *parent, const char *name)
: KPrintDialogPage(parent, name)
{
	setTitle(i18n("Schedule"));
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
	m_time->insertItem(i18n("Day time"));
	m_time->insertItem(i18n("Evening"));
	m_time->insertItem(i18n("Night"));
	m_time->insertItem(i18n("Weekend"));
	m_time->insertItem(i18n("Second shift (after close of business)"));
	m_time->insertItem(i18n("Third shift (after midnight)"));
	int	w = fontMetrics().width("00") + 10;
	m_hh = new QLineEdit(this);
	m_hh->setMaxLength(2);
	m_hh->setValidator(new QIntValidator(0, 23, m_hh));
	m_hh->setMaximumWidth(w);
	m_hh->setAlignment(Qt::AlignCenter);
	m_mm = new QLineEdit(this);
	m_mm->setMaxLength(2);
	m_mm->setValidator(new QIntValidator(0, 59, m_mm));
	m_mm->setMaximumWidth(w);
	m_mm->setAlignment(Qt::AlignCenter);
	m_ss = new QLineEdit(this);
	m_ss->setMaxLength(2);
	m_ss->setValidator(new QIntValidator(0, 59, m_ss));
	m_ss->setMaximumWidth(w);
	m_ss->setAlignment(Qt::AlignCenter);
	m_group = new QButtonGroup(this);
	m_group->hide();

	QStringList	l = QStringList::split(":", QTime::currentTime().toString(), false);
	m_hh->setText(l[0]);
	m_mm->setText(l[1]);
	m_ss->setText("00");

	QLabel	*lab = new QLabel(i18n("&Send print job to printer:"), this);
	lab->setBuddy(m_time);
	QLabel	*labc1 = new QLabel(":", this);
	QLabel	*labc2 = new QLabel(":", this);

	QRadioButton	*r1 = new QRadioButton(QString::null, this);
	QRadioButton	*r2 = new QRadioButton(i18n("&at:"), this);

	m_group->insert(r1, 0);
	m_group->insert(r2, 1);
	m_group->setButton(0);

	QVBoxLayout	*l0 = new QVBoxLayout(this, 10, 10);
	l0->addWidget(lab);
	QHBoxLayout	*l1 = new QHBoxLayout(0, 0, 10), *l2 = new QHBoxLayout(0, 0, 10), *l3 = new QHBoxLayout(0, 0, 3);
	l0->addLayout(l1);
	l1->addSpacing(10);
	l1->addWidget(r1);
	l1->addWidget(m_time);
	l1->addStretch(1);
	l0->addLayout(l2);
	l2->addSpacing(10);
	l2->addWidget(r2);
	l2->addLayout(l3);
	l3->addWidget(m_hh);
	l3->addWidget(labc1);
	l3->addWidget(m_mm);
	l3->addWidget(labc2);
	l3->addWidget(m_ss);
	l2->addStretch(1);
	l0->addStretch(1);

	connect(m_time, SIGNAL(activated(int)), SLOT(slotTimeChanged()));
	connect(m_hh, SIGNAL(textChanged(const QString&)), SLOT(slotHourChanged()));
	connect(m_mm, SIGNAL(textChanged(const QString&)), SLOT(slotHourChanged()));
	connect(m_ss, SIGNAL(textChanged(const QString&)), SLOT(slotHourChanged()));
	connect(r2, SIGNAL(clicked()), m_hh, SLOT(setFocus()));
}

KPSchedulePage::~KPSchedulePage()
{
}

bool KPSchedulePage::isValid(QString& msg)
{
	if (m_group->id(m_group->selected()) == 1)
	{
		if (!QTime::isValid(m_hh->text().toInt(), m_mm->text().toInt(), m_ss->text().toInt()))
		{
			msg = i18n("The time specified is not valid.");
			return false;
		}
	}
	return true;
}

void KPSchedulePage::setOptions(const QMap<QString,QString>& opts)
{
	QString	t = opts["job-hold-until"];
	if (!t.isEmpty())
	{
		int	ID(0);
		int	item(0);

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
			QStringList	l = QStringList::split(":", t, false);
			while (l.count() < 3)
				l.append("00");
			m_hh->setText(num2str(fixHour(l[0].toInt()-m_gmtdiff)));
			m_mm->setText(num2str(l[1].toInt()));
			m_ss->setText(num2str(l[2].toInt()));
			ID = 1;
		}

		if (ID == 0)
			m_time->setCurrentItem(item);

		m_group->setButton(ID);
	}
}

void KPSchedulePage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
	int	ID = m_group->id(m_group->selected());
	if (incldef || ID == 1 || m_time->currentItem() != 0)
	{
		QString	t;
		if (ID == 0)
		{
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
			}
		}
		else
		{
			int	hh = fixHour(m_hh->text().toInt() + m_gmtdiff);
			t = QString::number(hh) + ":" + m_mm->text() + ":" + m_ss->text();
		}
		opts["job-hold-until"] = t;
	}
}

void KPSchedulePage::slotTimeChanged()
{
	m_group->setButton(0);
}

void KPSchedulePage::slotHourChanged()
{
	m_group->setButton(1);
}

#include "kpschedulepage.moc"
