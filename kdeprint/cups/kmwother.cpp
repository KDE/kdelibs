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

#include "kmwother.h"
#include "kmwizard.h"
#include "kmprinter.h"
#include "kmmanager.h"
#include "cupsinfos.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qheader.h>
#include <qdict.h>

#include <klocale.h>
#include <klistview.h>
#include <kiconloader.h>

KMWOther::KMWOther(QWidget *parent, const char *name)
: KMWizardPage(parent,name)
{
	m_ID = KMWizard::Custom+5;
	m_title = i18n("URI Selection");
	m_nextpage = KMWizard::Driver;

	m_uri = new QLineEdit(this);
	QLabel	*l1 = new QLabel(this);
	l1->setText(i18n("<p>Enter the URI corresponding to the printer to be installed. "
	                 "Examples:</p><ul>"
					 "<li>smb://[login[:passwd]@]server/printer</li>"
					 "<li>lpd://server/queue</li>"
					 "<li>parallel:/dev/lp0</li></ul>"));
	QLabel	*l2 = new QLabel(i18n("URI:"), this);
	m_uriview = new KListView( this );
	m_uriview->addColumn( "" );
	m_uriview->header()->hide();
	m_uriview->setSorting( -1 );
	connect( m_uriview, SIGNAL( pressed( QListViewItem* ) ), SLOT( slotPressed( QListViewItem* ) ) );

	QVBoxLayout	*lay1 = new QVBoxLayout(this, 0, 15);
	QVBoxLayout	*lay2 = new QVBoxLayout(0, 0, 5);
	lay1->addWidget(l1);
	lay1->addLayout(lay2);
	lay1->addWidget( m_uriview );
	lay2->addWidget(l2);
	lay2->addWidget(m_uri);
}

void KMWOther::initPrinter(KMPrinter *p)
{
	m_uri->setText(p->device().url());

	if ( m_uriview->childCount() == 0 )
	{
		QStringList l = KMManager::self()->detectLocalPrinters();
		if ( l.isEmpty() || l.count() % 4 != 0 )
			return;

		QListViewItem *item = 0, *lastparent = 0, *root;
		root = new QListViewItem( m_uriview, i18n( "CUPS server" ) + QString( " %1:%2" ).arg( CupsInfos::self()->host() ).arg( CupsInfos::self()->port() ) );
		root->setPixmap( 0, SmallIcon( "gear" ) );
		root->setOpen( true );
		QDict<QListViewItem> parents, last;
		parents.setAutoDelete( false );
		last.setAutoDelete( false );
		for ( QStringList::Iterator it=l.begin(); it!=l.end(); ++it )
		{
			QString cl = *it;
			QString uri = *( ++it );
			QString desc = *( ++it );
			QString prt = *( ++it );
			if ( !prt.isEmpty() )
				desc.append( " [" + prt + "]" );
			QListViewItem *parent = parents.find( cl );
			if ( !parent )
			{
				parent = new QListViewItem( root, lastparent, cl );
				parent->setOpen( true );
				if ( cl == "network" )
					parent->setPixmap( 0, SmallIcon( "network" ) );
				else if ( cl == "direct" )
					parent->setPixmap( 0, SmallIcon( "kdeprint_computer" ) );
				else if ( cl == "serial" )
					parent->setPixmap( 0, SmallIcon( "usb" ) );
				else
					parent->setPixmap( 0, SmallIcon( "package" ) );
				lastparent = parent;
				parents.insert( cl, parent );
			}
			item = new QListViewItem( parent, last.find( cl ), desc, uri);
			last.insert( cl, item );
		}
	}
}

void KMWOther::updatePrinter(KMPrinter *p)
{
	QString	dev = m_uri->text();
	p->setDevice(KURL(dev));
}

void KMWOther::slotPressed( QListViewItem *item )
{
	if ( item && !item->text( 1 ).isEmpty() )
		m_uri->setText( item->text( 1 ) );
}

#include "kmwother.moc"
