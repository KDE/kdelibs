/*  This file is part of the KDE project
    Copyright (C) 2004,2006 Matthias Kretz <kretz@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.

*/

#include "backendselection.h"

#include <kservicetypeprofile.h>
#include <kservicetypetrader.h>
#include <ksimpleconfig.h>
#include <QStringList>
#include <QListWidgetItem>
#include <kapplication.h>
#include <kicon.h>
#include <QList>
#include <QtDBus/QtDBus>

BackendSelection::BackendSelection( QWidget* parent )
	: QWidget( parent )
{
	setupUi( this );

	connect( m_select, SIGNAL( itemSelectionChanged() ),
			SLOT( selectionChanged() ) );
	//connect( m_website, SIGNAL( leftClickedURL( const QString & ) ),
			//kapp, SLOT( invokeBrowser( const QString & ) ) );
	connect( m_up, SIGNAL( clicked() ), SLOT( up() ) );
	connect( m_down, SIGNAL( clicked() ), SLOT( down() ) );
}

void BackendSelection::load()
{
	const KService::List offers = KServiceTypeTrader::self()->query( "PhononBackend",
			"Type == 'Service' and [X-KDE-PhononBackendInfo-InterfaceVersion] == 1" );
	// the offers are already sorted for preference
    loadServices(offers);
}

void BackendSelection::loadServices( const KService::List& offers )
{
	m_services.clear();
	m_select->clear();

    KService::List::const_iterator it = offers.begin();
	const KService::List::const_iterator end = offers.end();
	for( ; it != end; ++it )
	{
		KService::Ptr service = *it;
		m_select->addItem( service->name() );
		m_services[ service->name() ] = service;
	}
	m_select->setItemSelected( m_select->item( 0 ), true );
}

void BackendSelection::save()
{
	// save to servicetype profile
	KService::List services;
	unsigned int count = m_select->count();
	for( unsigned int i = 0; i < count; ++i )
	{
		QListWidgetItem* item = m_select->item( i );
		KService::Ptr service = m_services[ item->text() ];
		services.append( service );
	}
	KServiceTypeProfile::writeServiceTypeProfile( "PhononBackend", services );

	QDBusMessage signal = QDBusMessage::createSignal( "/", "org.kde.Phonon.Factory", "phononBackendChanged" );
	QDBusConnection::sessionBus().send(signal);
}

void BackendSelection::defaults()
{
	loadServices( KServiceTypeTrader::self()->defaultOffers( "PhononBackend" ) );
}

void BackendSelection::selectionChanged()
{
	KService::Ptr service;
	for( int i = 0; i < m_select->count(); ++i )
	{
		QListWidgetItem* item = m_select->item( i );
		if( m_select->isItemSelected( item ) )
		{
			service = m_services[ item->text() ];
			break;
		}
	}
	if( service )
	{
		m_icon->setPixmap( KIcon( service->icon() ).pixmap( 32 ) );
		m_name->setText( service->name() );
		m_comment->setText( service->comment() );
		m_website->setText( service->property( "X-KDE-PhononBackendInfo-Website" ).toString() );
		//m_website->setURL( m_website->text() );
		m_version->setText( service->property( "X-KDE-PhononBackendInfo-Version" ).toString() );
	}
}

void BackendSelection::up()
{
	QList<QListWidgetItem*> selectedList = m_select->selectedItems();
	foreach( QListWidgetItem* selected, selectedList )
	{
		int row = m_select->row( selected );
		if( row > 0 )
		{
			QListWidgetItem* taken = m_select->takeItem( row - 1 );
			m_select->insertItem( row, taken );
			emit changed();
		}
	}
}

void BackendSelection::down()
{
	QList<QListWidgetItem*> selectedList = m_select->selectedItems();
	foreach( QListWidgetItem* selected, selectedList )
	{
		int row = m_select->row( selected );
		if( row + 1 < m_select->count() )
		{
			QListWidgetItem* taken = m_select->takeItem( row + 1 );
			m_select->insertItem( row, taken );
			emit changed();
		}
	}
}

#include "backendselection.moc"

// vim: sw=4 ts=4 noet
