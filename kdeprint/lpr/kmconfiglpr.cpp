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

#include "kmconfiglpr.h"
#include "lprsettings.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <klocale.h>
#include <kconfig.h>

KMConfigLpr::KMConfigLpr(QWidget *parent, const char *name)
: KMConfigPage(parent, name)
{
	setPageName(i18n("Spooler"));
	setPageHeader(i18n("Spooler Settings"));
	setPagePixmap("gear");

	QGroupBox	*m_modebox = new QGroupBox(1, Qt::Vertical, i18n("Spooler"), this);

	m_mode = new QComboBox(m_modebox);
	m_mode->insertItem("LPR (BSD compatible)");
	m_mode->insertItem("LPRng");

	QVBoxLayout	*l0 = new QVBoxLayout(this, 5, 10);
	l0->addWidget(m_modebox);
	l0->addStretch(1);
}

void KMConfigLpr::loadConfig(KConfig*)
{
	m_mode->setCurrentItem(LprSettings::self()->mode());
}

void KMConfigLpr::saveConfig(KConfig *conf)
{
	LprSettings::self()->setMode((LprSettings::Mode)(m_mode->currentItem()));

	QString	modestr;
	switch (m_mode->currentItem())
	{
		default:
		case 0: modestr = "LPR"; break;
		case 1: modestr = "LPRng"; break;
	}
	conf->setGroup("LPR");
	conf->writeEntry("Mode", modestr);
}
