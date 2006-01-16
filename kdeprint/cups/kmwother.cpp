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

#include "kmwother.h"
#include "kmwizard.h"
#include "kmprinter.h"
#include "kmmanager.h"
#include "cupsinfos.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <q3header.h>
#include <QHash>

#include <klocale.h>
#include <klistview.h>
#include <kiconloader.h>
#include <kdebug.h>

KMWOther::KMWOther(QWidget *parent)
    : KMWizardPage(parent)
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
	connect( m_uriview, SIGNAL( pressed( Q3ListViewItem* ) ), SLOT( slotPressed( Q3ListViewItem* ) ) );

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
	m_uri->setText(p->device());

	if ( m_uriview->childCount() == 0 )
	{
		QStringList l = KMManager::self()->detectLocalPrinters();
		if ( l.isEmpty() || l.count() % 4 != 0 )
			return;

		Q3ListViewItem *item = 0, *lastparent = 0, *root;
		root = new Q3ListViewItem( m_uriview, i18n( "CUPS Server %1:%2" ).arg( CupsInfos::self()->host() ).arg( CupsInfos::self()->port() ) );
		root->setPixmap( 0, SmallIcon( "gear" ) );
		root->setOpen( true );
		QHash<QString, Q3ListViewItem*> parents, last;
		for ( QStringList::Iterator it=l.begin(); it!=l.end(); ++it )
		{
			QString cl = *it;
			QString uri = *( ++it );
			QString desc = *( ++it );
			QString prt = *( ++it );
			if ( !prt.isEmpty() )
				desc.append( " [" + prt + "]" );
			Q3ListViewItem *parent = parents.value( cl );
			if ( !parent )
			{
				parent = new Q3ListViewItem( root, lastparent, cl );
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
			item = new Q3ListViewItem( parent, last.value( cl ), desc, uri);
			last.insert( cl, item );
		}
	}
}

void KMWOther::updatePrinter(KMPrinter *p)
{
	p->setDevice( m_uri->text() );
}

void KMWOther::slotPressed( Q3ListViewItem *item )
{
	if ( item && !item->text( 1 ).isEmpty() )
		m_uri->setText( item->text( 1 ) );
}

#include "kmwother.moc"
