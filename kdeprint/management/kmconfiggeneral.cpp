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

#include "kmconfiggeneral.h"

#include <qlayout.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <krun.h>
#include <kmimemagic.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kcursor.h>

KMConfigGeneral::KMConfigGeneral(QWidget *parent)
: KMConfigPage(parent,"ConfigTimer")
{
	setPageName(i18n("General"));
	setPageHeader(i18n("General settings"));
	setPagePixmap("fileprint");

	QGroupBox	*m_timerbox = new QGroupBox(0, Qt::Vertical, i18n("Timer delay (seconds)"), this);
	m_timer = new KIntNumInput(m_timerbox,"Timer");
	m_timer->setRange(0,30);
	m_timer->setSpecialValueText(i18n("Disabled"));

	QGroupBox	*m_testpagebox = new QGroupBox(0, Qt::Vertical, i18n("Test page"), this);
	m_defaulttestpage = new QCheckBox(i18n("Use non default test page"), m_testpagebox, "TestPageCheck");
	m_testpage = new KURLRequester(m_testpagebox,"TestPage");
	QPushButton	*m_preview = new QPushButton(i18n("Preview..."), m_testpagebox);
	connect(m_defaulttestpage,SIGNAL(toggled(bool)),m_testpage,SLOT(setEnabled(bool)));
	connect(m_defaulttestpage,SIGNAL(toggled(bool)),m_preview,SLOT(setEnabled(bool)));
	connect(m_preview,SIGNAL(clicked()),SLOT(slotTestPagePreview()));
	m_testpage->setDisabled(true);
	m_preview->setDisabled(true);
	m_defaulttestpage->setCursor(KCursor::handCursor());

	//layout
	QVBoxLayout	*lay0 = new QVBoxLayout(this, 5, 10);
	lay0->addWidget(m_timerbox);
	lay0->addWidget(m_testpagebox);
	lay0->addStretch(1);
	QVBoxLayout	*lay1 = new QVBoxLayout(m_timerbox->layout(), 0);
	lay1->addSpacing(10);
	lay1->addWidget(m_timer);
	QVBoxLayout	*lay2 = new QVBoxLayout(m_testpagebox->layout(), 10);
	QHBoxLayout	*lay3 = new QHBoxLayout(0, 0, 0);
	lay2->addWidget(m_defaulttestpage);
	lay2->addWidget(m_testpage);
	lay2->addLayout(lay3);
	lay3->addWidget(m_preview);
	lay3->addStretch(1);
}

void KMConfigGeneral::loadConfig(KConfig *conf)
{
	conf->setGroup("General");
	m_timer->setValue(conf->readNumEntry("TimerDelay",5));
	QString	tpage = conf->readEntry("TestPage",QString::null);
	if (!tpage.isEmpty())
	{
		m_defaulttestpage->setChecked(true);
		m_testpage->setURL(tpage);
	}
}

void KMConfigGeneral::saveConfig(KConfig *conf)
{
	conf->setGroup("General");
	conf->writeEntry("TimerDelay",m_timer->value());
	conf->writeEntry("TestPage",(m_defaulttestpage->isChecked() ? m_testpage->url() : QString::null));
}

void KMConfigGeneral::slotTestPagePreview()
{
	QString	tpage = m_testpage->url();
	if (tpage.isEmpty())
		KMessageBox::error(this, i18n("Empty file name."));
	else
		KRun::runURL(tpage, KMimeMagic::self()->findFileType(tpage)->mimeType());
}
#include "kmconfiggeneral.moc"
