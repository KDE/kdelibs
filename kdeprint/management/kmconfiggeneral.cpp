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
#include <qlabel.h>
#include <qwhatsthis.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <krun.h>
#include <kmimemagic.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kcursor.h>
#include <klineedit.h>
#include <kguiitem.h>

KMConfigGeneral::KMConfigGeneral(QWidget *parent)
: KMConfigPage(parent,"ConfigTimer")
{
	setPageName(i18n("General"));
	setPageHeader(i18n("General settings"));
	setPagePixmap("fileprint");

	QGroupBox	*m_timerbox = new QGroupBox(0, Qt::Vertical, i18n("Refresh Interval"), this);
	m_timer = new KIntNumInput(m_timerbox,"Timer");
	m_timer->setRange(0,30);
    m_timer->setSuffix( i18n( " sec" ) );
	m_timer->setSpecialValueText(i18n("Disabled"));
	QWhatsThis::add(m_timer, i18n("This time setting controls the refresh rate of various "
			              "<b>KDE Print</b> components like the print manager "
				      "and the job viewer."));

	QGroupBox	*m_testpagebox = new QGroupBox(0, Qt::Vertical, i18n("Test Page"), this);
	m_defaulttestpage = new QCheckBox(i18n("&Specify personal test page"), m_testpagebox, "TestPageCheck");
	m_testpage = new KURLRequester(m_testpagebox,"TestPage");
	m_preview = new KPushButton(KGuiItem(i18n("Preview..."), "filefind"), m_testpagebox);
	connect(m_defaulttestpage,SIGNAL(toggled(bool)),m_testpage,SLOT(setEnabled(bool)));
	connect(m_defaulttestpage,SIGNAL(toggled(bool)),this,SLOT(setEnabledPreviewButton(bool)));
	connect(m_preview,SIGNAL(clicked()),SLOT(slotTestPagePreview()));
        connect(m_testpage->lineEdit(),SIGNAL(textChanged ( const QString & )),this,SLOT(testPageChanged(const QString & )));
	m_testpage->setDisabled(true);
	m_preview->setDisabled(true);
	m_defaulttestpage->setCursor(KCursor::handCursor());

	QGroupBox	*m_statusbox = new QGroupBox(0, Qt::Vertical, i18n("Miscellaneous"), this);
	m_statusmsg = new QCheckBox(i18n("Sho&w printing status message box"), m_statusbox);
	m_uselast = new QCheckBox(i18n("De&faults to the last printer used in the application"), m_statusbox);

	//layout
	QVBoxLayout	*lay0 = new QVBoxLayout(this, 5, 10);
	lay0->addWidget(m_timerbox);
	lay0->addWidget(m_testpagebox);
	lay0->addWidget(m_statusbox);
	lay0->addStretch(1);
	QVBoxLayout	*lay1 = new QVBoxLayout(m_timerbox->layout(), 0);
	lay1->addSpacing(5);
	lay1->addWidget(m_timer);
	QVBoxLayout	*lay2 = new QVBoxLayout(m_testpagebox->layout(), 10);
	QHBoxLayout	*lay3 = new QHBoxLayout(0, 0, 0);
	lay2->addWidget(m_defaulttestpage);
	lay2->addWidget(m_testpage);
	lay2->addLayout(lay3);
	lay3->addStretch(1);
	lay3->addWidget(m_preview);
	QVBoxLayout	*lay4 = new QVBoxLayout(m_statusbox->layout(), 10);
	lay4->addWidget(m_statusmsg);
	lay4->addWidget(m_uselast);
        m_preview->setEnabled( !m_testpage->lineEdit()->text().isEmpty());
}

void KMConfigGeneral::testPageChanged(const QString &test )
{
    m_preview->setEnabled( !test.isEmpty());
}

void KMConfigGeneral::setEnabledPreviewButton(bool b)
{
    m_preview->setEnabled(!m_testpage->lineEdit()->text().isEmpty()&&b);
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
	m_statusmsg->setChecked(conf->readBoolEntry("ShowStatusMsg", true));
	m_uselast->setChecked(conf->readBoolEntry("UseLast", true));
}

void KMConfigGeneral::saveConfig(KConfig *conf)
{
	conf->setGroup("General");
	conf->writeEntry("TimerDelay",m_timer->value());
	conf->writeEntry("TestPage",(m_defaulttestpage->isChecked() ? m_testpage->url() : QString::null));
	if (m_defaulttestpage->isChecked() && KMimeMagic::self()->findFileType(m_testpage->url())->mimeType() != "application/postscript")
		KMessageBox::sorry(this, i18n("The selected test page is not a PostScript file. You may not "
		                              "be able to test your printer anymore."));
	conf->writeEntry("ShowStatusMsg", m_statusmsg->isChecked());
	conf->writeEntry("UseLast", m_uselast->isChecked());
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
