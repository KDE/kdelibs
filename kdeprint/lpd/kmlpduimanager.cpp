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

#include "kmlpduimanager.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "kmwizard.h"
#include "kmwbackend.h"
#include "kmpropertypage.h"
#include "kmpropbackend.h"
#include "kmpropdriver.h"

#include <qfile.h>
#include <klocale.h>

KMLpdUiManager::KMLpdUiManager(QObject *parent, const char *name)
: KMUiManager(parent,name)
{
}

KMLpdUiManager::~KMLpdUiManager()
{
}

void KMLpdUiManager::setupWizard(KMWizard *wizard)
{
	KMWBackend	*backend = wizard->backendPage();
	backend->addBackend(KMWizard::Local,i18n("Local printer (parallel, serial, USB)"),true);
	backend->addBackend(KMWizard::LPD,i18n("Remote LPD queue"),true);
	backend->addBackend(KMWizard::SMB,i18n("SMB shared printer (Windows)"),false,KMWizard::Password);
	backend->addBackend(KMWizard::TCP,i18n("Network printer (TCP)"),false);
	backend->addBackend(KMWizard::File,i18n("File printer (print to file)"),true);

	KMManager	*mgr = KMFactory::self()->manager();
	if (QFile::exists(mgr->driverDirectory()+"/smbprint")) backend->enableBackend(KMWizard::SMB,true);
	if (QFile::exists(mgr->driverDirectory()+"/directprint")) backend->enableBackend(KMWizard::TCP,true);
	if (QFile::exists(mgr->driverDirectory()+"/ncpprint")) backend->enableBackend(KMWizard::Custom+1,true);
}

void KMLpdUiManager::setupPropertyPages(KMPropertyPage *p)
{
	p->addPropPage(new KMPropBackend(p, "Backend"));
	p->addPropPage(new KMPropDriver(p, "Driver"));
}
