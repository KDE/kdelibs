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

#include "kmextuimanager.h"
#include "kpqtpage.h"
#include "kprinterpropertydialog.h"

#include <klocale.h>

KMExtUiManager::KMExtUiManager(QObject *parent, const char *name, const QStringList & /*args*/)
: KMUiManager(parent,name)
{
	m_printdialogflags |= KMUiManager::PrintCommand;
}

KMExtUiManager::~KMExtUiManager()
{
}

int KMExtUiManager::pluginPageCap()
{
	return KMUiManager::NoAutoCollate;
}

void KMExtUiManager::setupPrinterPropertyDialog( KPrinterPropertyDialog *dlg )
{
	dlg->addPage( new KPQtPage( dlg, "QtPage" ) );
}
