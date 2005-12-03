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

#include "kpschedulepage.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qregexp.h>
#include <q3datetimeedit.h>
#include <qdatetime.h>
#include <qlineedit.h>
#include <klocale.h>
#include <kseparator.h>
#include <knuminput.h>

#include <time.h>

KPSchedulePage::KPSchedulePage(QWidget *parent)
    : KPrintDialogPage(parent)
{
	//WhatsThis strings.... (added by pfeifle@kde.org)
	QString whatsThisBillingInfo = i18n(    " <qt> <p><b>Print Job Billing and Accounting</b></p> "
						" <p>Insert a meaningful string here to associate"
						" the current print job with a certain account. This"
						" string will appear in the CUPS \"page_log\" to help"
						" with the print accounting in your organization. (Leave"
						" it empty if you do not need it.)"
						" <p> It is useful for people"
						" who print on behalf of different \"customers\", like"
						" print service bureaux, letter shops, press and prepress"
						" companies, or secretaries who serve different bosses, etc.</p>"
						" <br> "
                                                " <hr> "
						" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
						" with the CUPS commandline job option parameter:</em> "
						" <pre>"
						"    -o job-billing=...         # example: \"Marketing_Department\" or \"Joe_Doe\" "
						" </pre>"
						" </p> "
                                                " </qt>" );

	QString whatsThisScheduledPrinting = i18n(" <qt> <p><b>Scheduled Printing</b></p> "
						" <p>Scheduled printing lets you control the time"
						" of the actual printout, while you can still send away your"
						" job <b>now</b> and have it out of your way."
						" <p> Especially useful"
						" is the \"Never (hold indefinitely)\" option. It allows you"
						" to park your job until a time when you (or a printer administrator)"
						" decides to manually release it."
						" <p> This is often required in"
						" enterprise environments, where you normally are not"
						" allowed to directly and immediately access the huge production"
						" printers in your <em>Central Repro Department</em>. However it"
						" is okay to send jobs to the queue which is under the control of the"
						" operators (who, after all, need to make sure that the 10,000"
						" sheets of pink paper which is required by the Marketing"
						" Department for a particular job are available and loaded"
						" into the paper trays).</p>"
						" <br> "
						" <hr> "
						" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
						" with the CUPS commandline job option parameter:</em> "
						" <pre>"
						"    -o job-hold-until=...      # example: \"indefinite\" or \"no-hold\" "
						" </pre>"
						" </p> "
						" </qt>" );

	QString whatsThisPageLabel = i18n(      " <qt> <p><b>Page Labels</b></p> "
						" <p>Page Labels are printed by CUPS at the top and bottom"
						" of each page. They appear on the pages surrounded by a little"
						" frame box."
						" <p>They contain any string you type into the line edit field.</p>"
						" <br> "
						" <hr> "
						" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
						" with the CUPS commandline job option parameter:</em> "
						" <pre>"
						"    -o page-label=\"...\"      # example: \"Company Confidential\" "
						" </pre>"
						" </p> "
						" </qt>" );

	QString whatsThisJobPriority = i18n(    " <qt> <p><b>Job Priority</b></p> "
						" <p>Usually CUPS prints all jobs per queue according to"
						" the \"FIFO\" principle: <em>First In, First Out</em>."
						" <p> The"
						" job priority option allows you to re-order the queue according"
						" to your needs."
						" <p> It works in both directions: you can increase"
						" as well as decrease priorities. (Usually you can only control"
						" your <b>own</b> jobs)."
						" <p> Since the default job priority is \"50\", any job sent"
						" with, for example, \"49\" will be printed only after all those"
						" others have finished. Conversely, a"
						" \"51\" or higher priority job will go right to the top of"
						" a populated queue (if no other, higher prioritized one is present).</p>"
						" <br> "
						" <hr> "
						" <p><em><b>Additional hint for power users:</b> This KDEPrint GUI element matches "
						" with the CUPS commandline job option parameter:</em> "
						" <pre>"
						"    -o job-priority=...   # example: \"10\" or \"66\" or \"99\" "
						" </pre>"
						" </p> "
						" </qt>" );

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
        m_time->setWhatsThis(whatsThisScheduledPrinting);
	m_tedit = new Q3TimeEdit(this);
	m_tedit->setAutoAdvance(true);
	m_tedit->setTime(QTime::currentTime());
	m_tedit->setEnabled(false);
        m_tedit->setWhatsThis(whatsThisScheduledPrinting);
	m_billing = new QLineEdit(this);
        m_billing->setWhatsThis(whatsThisBillingInfo);
	m_pagelabel = new QLineEdit(this);
        m_pagelabel->setWhatsThis(whatsThisPageLabel);
	m_priority = new KIntNumInput(50,this);
        m_priority->setWhatsThis(whatsThisJobPriority);
	m_priority->setRange(1, 100, 10, true);

	QLabel	*lab = new QLabel(i18n("&Scheduled printing:"), this);
	lab->setBuddy(m_time);
        lab->setWhatsThis(whatsThisScheduledPrinting);
	QLabel	*lab1 = new QLabel(i18n("&Billing information:"), this);
        lab1->setWhatsThis(whatsThisBillingInfo);
	lab1->setBuddy(m_billing);
	QLabel	*lab2 = new QLabel(i18n("T&op/Bottom page label:"), this);
        lab2->setWhatsThis(whatsThisPageLabel);
	lab2->setBuddy(m_pagelabel);
	m_priority->setLabel(i18n("&Job priority:"), Qt::AlignVCenter|Qt::AlignLeft);
        m_priority->setWhatsThis(whatsThisJobPriority);

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
	t = opts["job-billing"].trimmed();
	t.replace(re, "");
	m_billing->setText(t);
	t = opts["page-label"].trimmed();
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
