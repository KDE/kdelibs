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

#include "kmlpduimanager.h"
#include "kmwizard.h"
#include "kmwbackend.h"
#include "kmpropertypage.h"
#include "kmpropbackend.h"
#include "kmpropdriver.h"

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
	backend->addBackend(KMWizard::Local,i18n("Local printer (parallel, serial, USB)"),false);
	backend->addBackend(KMWizard::LPD,i18n("Remote LPD queue"),true);
	backend->addBackend(KMWizard::SMB,i18n("SMB shared printer (Windows)"),true,KMWizard::Password);
	backend->addBackend(KMWizard::TCP,i18n("Network printer (TCP)"),true);
	backend->addBackend(KMWizard::File,i18n("File printer (print to file)"),true);
}

void KMLpdUiManager::setupPropertyPages(KMPropertyPage *p)
{
	p->addPropPage(new KMPropBackend(p, "Backend"));
	p->addPropPage(new KMPropDriver(p, "Driver"));
}
