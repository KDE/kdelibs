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

#include "kmwbanners.h"
#include "kmwizard.h"
#include "kmprinter.h"
#include "kmfactory.h"
#include "kmmanager.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmap.h>
#include <klocale.h>

QStringList defaultBanners()
{
	QStringList	bans;
	QPtrList<KMPrinter>	*list = KMFactory::self()->manager()->printerList(false);
	if (list && list->count() > 0)
	{
		QPtrListIterator<KMPrinter>	it(*list);
		for (;it.current() && !it.current()->isPrinter(); ++it) ;
		if (it.current() && KMFactory::self()->manager()->completePrinter(it.current()))
		{
			QString	s = list->getFirst()->option("kde-banners-supported");
			bans = QStringList::split(',',s,false);
		}
	}
	if (bans.count() == 0)
		bans.append("none");
	return bans;
}

static struct
{
	const char *banner;
	const char *name;
} bannermap[] = 
{
	{ "none", I18N_NOOP( "No Banner" ) },
	{ "classified", I18N_NOOP( "Classified" ) },
	{ "confidential", I18N_NOOP( "Confidential" ) },
	{ "secret", I18N_NOOP( "Secret" ) },
	{ "standard", I18N_NOOP( "Standard" ) },
	{ "topsecret", I18N_NOOP( "Top Secret" ) },
	{ "unclassified", I18N_NOOP( "Unclassified" ) },
	{ 0, 0 }
};

QString mapBanner( const QString& ban )
{
	static QMap<QString,QString> map;
	if ( map.size() == 0 )
		for ( int i=0; bannermap[ i ].banner; i++ )
			map[ bannermap[ i ].banner ] = bannermap[ i ].name;
	QMap<QString,QString>::ConstIterator it = map.find( ban );
	if ( it == map.end() )
		return ban;
	else
		return it.data();
}

//**************************************************************************************************************

KMWBanners::KMWBanners(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::Banners;
	m_title = i18n("Banner Selection");
	m_nextpage = KMWizard::Custom+3;

	m_start = new QComboBox(this);
	m_end = new QComboBox(this);

	QLabel	*l1 = new QLabel(i18n("&Starting banner:"),this);
	QLabel	*l2 = new QLabel(i18n("&Ending banner:"),this);

	l1->setBuddy(m_start);
	l2->setBuddy(m_end);

	QLabel	*l0 = new QLabel(this);
	l0->setText(i18n("<p>Select the default banners associated with this printer. These "
			 "banners will be inserted before and/or after each print job sent "
			 "to the printer. If you don't want to use banners, select <b>No Banner</b>.</p>"));

	QGridLayout	*lay = new QGridLayout(this, 5, 2, 0, 10);
	lay->setColStretch(1,1);
	lay->addRowSpacing(1,20);
	lay->setRowStretch(4,1);
	lay->addMultiCellWidget(l0,0,0,0,1);
	lay->addWidget(l1,2,0);
	lay->addWidget(l2,3,0);
	lay->addWidget(m_start,2,1);
	lay->addWidget(m_end,3,1);
}

void KMWBanners::initPrinter(KMPrinter *p)
{
	if (p)
	{
		if (m_start->count() == 0)
		{
			m_bans = QStringList::split(',',p->option("kde-banners-supported"),false);
			if (m_bans.count() == 0)
				m_bans = defaultBanners();
			if (m_bans.find("none") == m_bans.end())
				m_bans.prepend("none");
			for ( QStringList::Iterator it=m_bans.begin(); it!=m_bans.end(); ++it )
			{
				m_start->insertItem( i18n( mapBanner(*it).utf8() ) );
				m_end->insertItem( i18n( mapBanner(*it).utf8() ) );
			}
		}
		QStringList	l = QStringList::split(',',p->option("kde-banners"),false);
		while (l.count() < 2)
			l.append("none");
		m_start->setCurrentItem(m_bans.findIndex(l[0]));
		m_end->setCurrentItem(m_bans.findIndex(l[1]));
	}
}

void KMWBanners::updatePrinter(KMPrinter *p)
{
	if (m_start->count() > 0)
	{
		p->setOption("kde-banners",m_bans[m_start->currentItem()]+","+m_bans[m_end->currentItem()]);
	}
}
