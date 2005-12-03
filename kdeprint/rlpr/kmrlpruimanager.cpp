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

#include "kmrlpruimanager.h"
#include "kmpropertypage.h"
#include "kmwizard.h"
#include "kmconfigdialog.h"

#include "kmwrlpr.h"
#include "kmproprlpr.h"
#include "kmconfigproxy.h"

#include <klocale.h>

KMRlprUiManager::KMRlprUiManager(QObject *parent, const char *name, const QStringList & /*args*/)
: KMUiManager(parent)
{
}

KMRlprUiManager::~KMRlprUiManager()
{
}

void KMRlprUiManager::setupPropertyPages(KMPropertyPage *pages)
{
    KMPropRlpr *page = new KMPropRlpr(pages );
    page->setObjectName( "RlprPage" );
    pages->addPropPage(page );
}

void KMRlprUiManager::setupWizard(KMWizard *wizard)
{
	wizard->setNextPage(KMWizard::Start,KMWizard::Custom+1);
        KMWRlpr *lpr = new KMWRlpr( wizard );
        lpr->setObjectName( "Rlpr" );
	// add page
	wizard->addPage( lpr );
}

void KMRlprUiManager::setupConfigDialog(KMConfigDialog *dlg)
{
	dlg->addConfigPage(new KMConfigProxy(dlg));
}
