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

#include "kmlpdfactory.h"
#include "kmlpdmanager.h"
#include "kmlpduimanager.h"
#include "klpdprinterimpl.h"

extern "C"
{
	void* init_kdeprint_lpd()
	{
		return new KLpdFactory;
	}
};

KLpdFactory::KLpdFactory(QObject *parent, const char *name)
: KLibFactory(parent,name)
{
}

KLpdFactory::~KLpdFactory()
{
}

QObject* KLpdFactory::createObject(QObject *parent, const char *name, const char *classname, const QStringList&)
{
	if (strcmp(classname,"KMManager") == 0)
		return new KMLpdManager(parent,name);
	else if (strcmp(classname,"KMUiManager") == 0)
		return new KMLpdUiManager(parent,name);
	else if (strcmp(classname,"KPrinterImpl") == 0)
		return new KLpdPrinterImpl(parent,name);
	else
		return NULL;
}
