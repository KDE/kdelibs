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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include "kmextmanager.h"
#include "kmprinter.h"

#include <klocale.h>

KMExtManager::KMExtManager(QObject *parent, const char *name, const QStringList & /*args*/)
    : KMManager(parent)
{
	setHasManagement(false);
	m_psprinter = 0;
}

KMExtManager::~KMExtManager()
{
}

void KMExtManager::listPrinters()
{
	if (!m_psprinter)
	{
		m_psprinter = new KMPrinter;
		m_psprinter->setName(i18n("PS_printer"));
		m_psprinter->setPrinterName(m_psprinter->name());
		m_psprinter->setDescription(i18n("PostScript file generator"));
		m_psprinter->setState(KMPrinter::Idle);
		addPrinter(m_psprinter);
	}
	else
		m_psprinter->setDiscarded(false);
}
