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

#include "kmwinfopage.h"
#include "kmwizard.h"

#include <qlabel.h>
#include <qlayout.h>
#include <klocale.h>
#include <kactivelabel.h>

KMWInfoPage::KMWInfoPage(QWidget *parent)
    : KMWizardPage(parent)
{
	m_ID = KMWizard::Start;
	m_title = i18n("Introduction");
	m_nextpage = KMWizard::Backend;

	//QLabel	*m_label = new QLabel(this);
	KActiveLabel	*m_label = new KActiveLabel(this);
	m_label->setText(i18n("<p>Welcome,</p><br>"
		"<p>This wizard will help to install a new printer on your computer. "
		"It will guide you through the various steps of the process of installing "
		"and configuring a printer for your printing system. At each step, you "
		"can always go back using the <b>Back</b> button.</p><br>"
		"<p>We hope you'll enjoy this tool!</p><br>"
		"<p align=right><a href=\"http://printing.kde.org\"><i>"
		"The KDE printing team</i></a>.</p>"));

	QVBoxLayout	*main_ = new QVBoxLayout(this, 0, 0);
	main_->addWidget(m_label);
}
