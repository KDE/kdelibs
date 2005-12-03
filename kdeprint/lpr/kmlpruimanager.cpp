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

#include "kmlpruimanager.h"
#include "kmpropertypage.h"
#include "kprinterpropertydialog.h"
#include "kpqtpage.h"
#include "kmconfigdialog.h"

#include "kmpropdriver.h"
#include "kmpropbackend.h"
#include "kmwizard.h"
#include "kmwbackend.h"
#include "kmconfiglpr.h"

#include <klocale.h>

KMLprUiManager::KMLprUiManager(QObject *parent, const char *name, const QStringList & /*args*/)
    : KMUiManager(parent)
{
}

KMLprUiManager::~KMLprUiManager()
{
}

void KMLprUiManager::setupPropertyPages(KMPropertyPage *pages)
{
    KMPropBackend *pb = new KMPropBackend( pages );
    pb->setObjectName( "Backend" );
    pages->addPropPage( pb );
    KMPropDriver *pd = new KMPropDriver( pages );
    pd->setObjectName( "Driver" );
    pages->addPropPage( pd );
}

void KMLprUiManager::setupPrinterPropertyDialog(KPrinterPropertyDialog *dlg)
{
    KPQtPage *qp = new KPQtPage( dlg->driver(), dlg );
    qp->setObjectName( "QtPage" );
    dlg->addPage( qp );
}

void KMLprUiManager::setupWizard(KMWizard *wizard)
{
	KMWBackend	*backend = wizard->backendPage();

	backend->addBackend(KMWizard::Local, true);
	backend->addBackend(KMWizard::LPD, true);
	backend->addBackend(KMWizard::TCP, true);
	backend->addBackend(KMWizard::SMB, true, KMWizard::Password);
}

void KMLprUiManager::setupConfigDialog(KMConfigDialog *dlg)
{
	dlg->addConfigPage(new KMConfigLpr(dlg));
}
