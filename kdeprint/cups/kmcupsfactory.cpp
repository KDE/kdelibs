/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <goffioul@imec.be>
 *
 *  $Id$
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

#include "kmcupsfactory.h"
#include "kmcupsmanager.h"
#include "kmcupsjobmanager.h"
#include "kmcupsuimanager.h"
#include "kcupsprinterimpl.h"

#include <kglobal.h>
#include <klocale.h>

extern "C"
{
	void* init_libkdeprint_cups()
	{
		return new KCupsFactory;
	}
};

KCupsFactory::KCupsFactory(QObject *parent, const char *name)
: KLibFactory(parent,name)
{
	KGlobal::locale()->insertCatalogue("ppdtranslations");
}

KCupsFactory::~KCupsFactory()
{
}

QObject* KCupsFactory::createObject(QObject *parent, const char *name, const char *classname, const QStringList&)
{
	if (strcmp(classname,"KMManager") == 0)
		return new KMCupsManager(parent,name);
	else if (strcmp(classname,"KMJobManager") == 0)
		return new KMCupsJobManager(parent,name);
	else if (strcmp(classname,"KMUiManager") == 0)
		return new KMCupsUiManager(parent,name);
	else if (strcmp(classname,"KPrinterImpl") == 0)
		return new KCupsPrinterImpl(parent,name);
	else
		return NULL;
}
