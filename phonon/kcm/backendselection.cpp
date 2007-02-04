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
#include <kcmoduleproxy.h>

BackendSelection::BackendSelection( QWidget* parent )
	: QWidget( parent )
{
	setupUi( this );

    m_emptyPage = stackedWidget->addWidget(new QWidget());

	connect( m_select, SIGNAL( itemSelectionChanged() ),
			SLOT( selectionChanged() ) );
	//connect( m_website, SIGNAL( leftClickedUrl( const QString & ) ),
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
    foreach (KCModuleProxy *proxy, m_kcms) {
        if (proxy) {
            proxy->load();
        }
    }
}

void BackendSelection::showBackendKcm(const KService::Ptr &backendService)
{
    QString parentComponent = backendService->library();
    if (!m_kcms.contains(parentComponent)) {
        const KService::List offers = KServiceTypeTrader::self()->query("KCModule",
                QString("'%1' in [X-KDE-ParentComponents]").arg(parentComponent));
        if (offers.isEmpty()) {
            m_kcms.insert(parentComponent, 0);
        } else {
            KCModuleProxy *proxy = new KCModuleProxy(offers.first());
            connect(proxy, SIGNAL(changed(bool)), SIGNAL(changed()));
            m_kcms.insert(parentComponent, proxy);
            stackedWidget->addWidget(proxy);
        }
    }
    QWidget *w = m_kcms.value(parentComponent);
    if (w) {
        stackedWidget->setCurrentWidget(w);
    } else {
        stackedWidget->setCurrentIndex(m_emptyPage);
    }
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
    // save embedded KCMs
    foreach (KCModuleProxy *proxy, m_kcms) {
        if (proxy) {
            proxy->save();
        }
    }

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
    foreach (KCModuleProxy *proxy, m_kcms) {
        if (proxy) {
            proxy->defaults();
        }
    }

	loadServices( KServiceTypeTrader::self()->defaultOffers( "PhononBackend" ) );
}

void BackendSelection::selectionChanged()
{
	KService::Ptr service;
    foreach (QListWidgetItem *item, m_select->selectedItems()) {
        service = m_services[item->text()];
        m_up->setEnabled(m_select->row(item) > 0);
        m_down->setEnabled(m_select->row(item) < m_select->count() - 1);
        break;
    }
    if(service) {
        m_icon->setPixmap(KIcon(service->icon()).pixmap(32));
        m_name->setText(service->name());
        m_comment->setText(service->comment());
        const QString website = service->property("X-KDE-PhononBackendInfo-Website").toString();
        m_website->setText(QString("<a href=\"%1\">%1</a>").arg(website));
        m_version->setText(service->property("X-KDE-PhononBackendInfo-Version").toString());
        showBackendKcm(service);
    } else {
        m_up->setEnabled(false);
        m_down->setEnabled(false);
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

// vim: sw=4 ts=4
