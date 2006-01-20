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

#include "kmconfigcupsdir.h"

#include <qcheckbox.h>
#include <kurlrequester.h>
#include <klocale.h>
#include <kdialog.h>
#include <kconfig.h>
#include <q3groupbox.h>
#include <qlayout.h>
#include <kcursor.h>

KMConfigCupsDir::KMConfigCupsDir(QWidget *parent)
    : KMConfigPage(parent)
{
        setObjectName( "ConfigCupsDir" );
	setPageName(i18n("Folder"));
	setPageHeader(i18n("CUPS Folder Settings"));
	setPagePixmap("folder");

	Q3GroupBox *m_dirbox = new Q3GroupBox(0, Qt::Vertical, i18n("Installation Folder"), this);
	m_installdir = new KUrlRequester(m_dirbox);
	m_installdir->setMode((KFile::Mode)(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly));
	m_stddir = new QCheckBox(i18n("Standard installation (/)"), m_dirbox);
	m_stddir->setCursor(KCursor::handCursor());

	QVBoxLayout *lay0 = new QVBoxLayout(this, 0, KDialog::spacingHint());
	lay0->addWidget(m_dirbox);
	lay0->addStretch(1);
	QVBoxLayout *lay1 = new QVBoxLayout(m_dirbox->layout(), 10);
	lay1->addWidget(m_stddir);
	lay1->addWidget(m_installdir);

	connect(m_stddir,SIGNAL(toggled(bool)),m_installdir,SLOT(setDisabled(bool)));
	m_stddir->setChecked(true);
}

void KMConfigCupsDir::loadConfig(KConfig *conf)
{
	conf->setGroup("CUPS");
	QString	dir = conf->readPathEntry("InstallDir");
	m_stddir->setChecked(dir.isEmpty());
	m_installdir->setURL(dir);
}

void KMConfigCupsDir::saveConfig(KConfig *conf)
{
	conf->setGroup("CUPS");
	conf->writePathEntry("InstallDir",(m_stddir->isChecked() ? QString() : m_installdir->url()));
}
