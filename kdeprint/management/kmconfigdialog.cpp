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

#include "kmconfigdialog.h"
#include "kmconfigpage.h"
#include "kmfactory.h"
#include "kmuimanager.h"

#include "kmconfiggeneral.h"

#include <qlayout.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>

KMConfigDialog::KMConfigDialog(QWidget *parent, const char *name)
: KDialogBase(KDialogBase::IconList,i18n("KDE print manager configuration"),KDialogBase::Ok|KDialogBase::Cancel,KDialogBase::Ok,parent,name)
{
	m_pages.setAutoDelete(false);
	addConfigPage(new KMConfigGeneral(this));
	KMFactory::self()->uiManager()->setupConfigDialog(this);

	// initialize pages
	KConfig	*conf = KMFactory::self()->printConfig();
	QPtrListIterator<KMConfigPage>	it(m_pages);
	for (;it.current();++it)
		it.current()->loadConfig(conf);

	// resize dialog
	resize(450,300);
}

void KMConfigDialog::addConfigPage(KMConfigPage *page)
{
	if (page)
	{
		QFrame	*frame = addPage(page->pageName(),page->pageHeader(),DesktopIcon(page->pagePixmap()));
		page->reparent(frame,QPoint(0,0));
		QVBoxLayout	*lay = new QVBoxLayout(frame, 0, 0);
		lay->addWidget(page);
		m_pages.append(page);
	}
}

void KMConfigDialog::slotOk()
{
	// save configuration
	KConfig	*conf = KMFactory::self()->printConfig();
	QPtrListIterator<KMConfigPage>	it(m_pages);
	for (;it.current();++it)
		it.current()->saveConfig(conf);
	conf->sync();

	// close the dialog
	KDialogBase::slotOk();
}
#include "kmconfigdialog.moc"
