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

#include <kpagewidgetmodel.h>

#include <qlayout.h>
#include <klocale.h>
#include <kconfig.h>
#include <kicon.h>
#include <QFrame>

KMConfigDialog::KMConfigDialog(QWidget *parent, const char *name)
: KPageDialog( parent )
{
  setFaceType( List );
  setCaption( i18n("KDE Print Configuration") );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );
  setObjectName( name );
  setModal( true );
  showButtonSeparator( true );

	addConfigPage(new KMConfigGeneral(this));
	addConfigPage(new KMConfigPreview(this));
	addConfigPage(new KMConfigFonts(this));
	addConfigPage(new KMConfigCommand(this));
	addConfigPage(new KMConfigFilter(this));
	addConfigPage(new KMConfigJobs(this));
	KMFactory::self()->uiManager()->setupConfigDialog(this);

	// initialize pages
	KConfig	*conf = KMFactory::self()->printConfig();
	QListIterator<KMConfigPage*>	it(m_pages);
	while(it.hasNext())
		it.next()->loadConfig(conf);

	// resize dialog
	resize(450,400);
}

void KMConfigDialog::addConfigPage(KMConfigPage *page)
{
	if (page)
	{
		QFrame	*frame = new QFrame( this );
		page->setParent(frame);
		QVBoxLayout	*lay = new QVBoxLayout(frame);
		lay->setMargin(0);
		lay->setSpacing(0);
		lay->addWidget(page);
		m_pages.append(page);

    KPageWidgetItem *item = new KPageWidgetItem( frame, page->pageName() );
    item->setHeader( page->pageHeader() );
    item->setIcon( KIcon( page->pagePixmap() ) );
	}
}

void KMConfigDialog::slotOk()
{
	// save configuration
	KConfig	*conf = KMFactory::self()->printConfig();
	QListIterator<KMConfigPage*>	it(m_pages);
	while(it.hasNext())
		it.next()->saveConfig(conf);
	KMFactory::self()->saveConfig();

	// close the dialog
	KPageDialog::accept();
}
#include "kmconfigdialog.moc"
