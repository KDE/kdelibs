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

#include "kmpages.h"
#include "kminfopage.h"
#include "kmjobviewer.h"
#include "kmpropertypage.h"
#include "kminstancepage.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kdialog.h>

KMPages::KMPages(QWidget *parent, const char *name)
: QTabWidget(parent,name)
{
	m_pages.setAutoDelete(false);
	initialize();
}

KMPages::~KMPages()
{
}

void KMPages::setPrinter(KMPrinter *p)
{
	Q3PtrListIterator<KMPrinterPage>	it(m_pages);
	for (int i=0;it.current();++it,i++)
		it.current()->setPrinter(p);
}

void KMPages::initialize()
{
	setMargin(KDialog::marginHint());

	// Info page
	KMInfoPage	*infopage = new KMInfoPage(this );
        infopage->setObjectName("InfoPage");
	addTab(infopage, SmallIcon("help"), i18n("Information"));
	m_pages.append(infopage);

	// Job page
	KMJobViewer	*jobviewer = new KMJobViewer(this );
        jobviewer->setObjectName( "JobViewer" );
	addTab(jobviewer, SmallIcon("folder"), i18n("Jobs"));
	m_pages.append(jobviewer);

	// Property page
	KMPropertyPage	*proppage = new KMPropertyPage(this );
        proppage->setObjectName( "Property");
	addTab(proppage, SmallIcon("configure"), i18n("Properties"));
	m_pages.append(proppage);

	// Instance page
	KMInstancePage	*instpage = new KMInstancePage(this );
        instpage->setObjectName( "Instance");
	addTab(instpage, SmallIcon("fileprint"), i18n("Instances"));
	m_pages.append(instpage);

	// initialize pages
	setPrinter(0);
}
