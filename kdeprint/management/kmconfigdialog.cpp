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
#include "kmconfigpreview.h"
#include "kmconfigcommand.h"
#include "kmconfigfilter.h"
#include "kmconfigfonts.h"
#include "kmconfigjobs.h"

#include <qlayout.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>

KMConfigDialog::KMConfigDialog(QWidget *parent, const char *name)
: KDialogBase(IconList,i18n("KDE Print Configuration"),Ok|Cancel,Ok,parent,name,true,true)
{
	m_pages.setAutoDelete(false);
	addConfigPage(new KMConfigGeneral(this));
	addConfigPage(new KMConfigPreview(this));
	addConfigPage(new KMConfigFonts(this));
	addConfigPage(new KMConfigCommand(this));
	addConfigPage(new KMConfigFilter(this));
	addConfigPage(new KMConfigJobs(this));
	KMFactory::self()->uiManager()->setupConfigDialog(this);

	// initialize pages
	KConfig	*conf = KMFactory::self()->printConfig();
	QPtrListIterator<KMConfigPage>	it(m_pages);
	for (;it.current();++it)
		it.current()->loadConfig(conf);

	// resize dialog
	resize(450,400);
}

void KMConfigDialog::addConfigPage(KMConfigPage *page)
{
	if (page)
	{
		QPixmap icon = KGlobal::instance()->iconLoader()->loadIcon(
		                                                           page->pagePixmap(),
		                                                           KIcon::NoGroup,
                        	                                           KIcon::SizeMedium
		                                                          );

		QFrame	*frame = addPage(page->pageName(),page->pageHeader(),icon);
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
	KMFactory::self()->saveConfig();

	// close the dialog
	KDialogBase::slotOk();
}
#include "kmconfigdialog.moc"
