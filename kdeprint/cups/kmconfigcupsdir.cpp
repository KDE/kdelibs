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

#include "kmconfigcupsdir.h"

#include <qcheckbox.h>
#include <kurlrequester.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <kcursor.h>

KMConfigCupsDir::KMConfigCupsDir(QWidget *parent)
: KMConfigPage(parent,"ConfigCupsDir")
{
	setPageName(i18n("Directory"));
	setPageHeader(i18n("CUPS Directory Settings"));
	setPagePixmap("folder");

	QGroupBox	*m_dirbox = new QGroupBox(0, Qt::Vertical, i18n("Installation Directory"), this);
	m_installdir = new KURLRequester(m_dirbox);
	m_installdir->setMode((KFile::Mode)(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly));
	m_stddir = new QCheckBox(i18n("Standard installation (/)"), m_dirbox);
	m_stddir->setCursor(KCursor::handCursor());

	QVBoxLayout	*lay0 = new QVBoxLayout(this, 5, 10);
	lay0->addWidget(m_dirbox);
	lay0->addStretch(1);
	QVBoxLayout	*lay1 = new QVBoxLayout(m_dirbox->layout(), 10);
	lay1->addWidget(m_stddir);
	lay1->addWidget(m_installdir);

	connect(m_stddir,SIGNAL(toggled(bool)),m_installdir,SLOT(setDisabled(bool)));
	m_stddir->setChecked(true);
}

void KMConfigCupsDir::loadConfig(KConfig *conf)
{
	conf->setGroup("CUPS");
	QString	dir = conf->readEntry("InstallDir",QString::null);
	m_stddir->setChecked(dir.isEmpty());
	m_installdir->setURL(dir);
}

void KMConfigCupsDir::saveConfig(KConfig *conf)
{
	conf->setGroup("CUPS");
	conf->writeEntry("InstallDir",(m_stddir->isChecked() ? QString::null : m_installdir->url()));
}
