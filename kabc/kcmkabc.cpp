/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    $Id$
*/

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <klocale.h>
#include <klistbox.h>
#include <ksimpleconfig.h>
#include <kurlrequester.h>

#include "kcmkabc.h"

#include "resource.h"
#include "resourceconfigdlg.h"
#include "resourcefactory.h"

class ConfigBoxItem : public QListBoxText
{
public:
    ConfigBoxItem( const QString &text = QString::null );

    QString key;
    QString type;

    void setText( const QString &text);
};

ConfigBoxItem::ConfigBoxItem( const QString &text )
    : QListBoxText( text )
{
    type = "";
}

void ConfigBoxItem::setText( const QString &text )
{
    QListBoxText::setText( text );
}

ConfigPageImpl::ConfigPageImpl( QWidget *parent, const char *name )
    : ConfigPage( parent, name )
{
    KABC::ResourceFactory *factory = KABC::ResourceFactory::self();

    types = factory->resources();

    for ( QStringList::Iterator it = types.begin(); it != types.end(); ++it ) {
	KABC::ResourceInfo *info = factory->info( (*it) );
	if ( !info )
	    typeCombo->insertItem( "no name available" );
	else
	    typeCombo->insertItem( info->name );
    }

    config = new KConfig( "kabcrc" );

    connect( addButton, SIGNAL(clicked()), this, SLOT(slotAdd()) );
    connect( removeButton, SIGNAL(clicked()), this, SLOT(slotRemove()) );
    connect( editButton, SIGNAL(clicked()), this, SLOT(slotEdit()) );
    connect( listBox, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()) );

    load();
}

void ConfigPageImpl::load()
{
    QStringList keys;

    config->setGroup( "General" );
    keys = config->readListEntry( "ResourceKeys" );

    for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
	config->setGroup( "Resource_" + (*it) );
	ConfigBoxItem *item = new ConfigBoxItem( config->readEntry( "ResourceName" ) );
	item->key = (*it);
	item->type = config->readEntry( "ResourceType" );

	listBox->insertItem( item );
    }

    emit changed( false );
}

void ConfigPageImpl::save()
{
    QStringList keys;

    QListBoxItem *item = listBox->item( 0 );
    while ( item != 0 ) {
	ConfigBoxItem *configItem = dynamic_cast<ConfigBoxItem*>( item );
	keys.append( configItem->key );
	item = item->next();
    }

    config->setGroup( "General" );
    config->writeEntry( "ResourceKeys", keys );

    config->sync();

    emit changed( false );
}

void ConfigPageImpl::defaults()
{
    delete config;
    config = new KConfig( "kabcrc" );

    listBox->clear();
    slotSelectionChanged();
    typeCombo->setCurrentItem( 0 );

    load();

    emit changed( true );
}

void ConfigPageImpl::slotAdd()
{
    QString key = KApplication::randomString( 10 );
    QString type = types[typeCombo->currentItem()];

    config->setGroup( "Resource_" + key );

    ResourceConfigDlg dlg( this, type, config, "ResourceConfigDlg" );

    if ( dlg.exec() ) {
	config->writeEntry( "ResourceName", dlg.resourceName->text() );
	config->writeEntry( "ResourceType", type );
	config->writeEntry( "ResourceIsReadOnly", dlg.resourceIsReadOnly->isChecked() );
	config->writeEntry( "ResourceIsFast", dlg.resourceIsFast->isChecked() );

	ConfigBoxItem *item = new ConfigBoxItem( dlg.resourceName->text() );
	item->key = key;
	item->type = type;
	listBox->insertItem( item );
	emit changed( true );
    } else {
	config->deleteGroup( "Resource_" + key );
    }
}

void ConfigPageImpl::slotRemove()
{
    QListBoxItem *item = listBox->item( listBox->currentItem() );
    QString key = dynamic_cast<ConfigBoxItem*>( item )->key;

    config->deleteGroup( "Resource_" + key );

    listBox->removeItem( listBox->currentItem() );
    emit changed( true );
}

void ConfigPageImpl::slotEdit()
{
    QListBoxItem *item = listBox->item( listBox->currentItem() );
    ConfigBoxItem *configItem = dynamic_cast<ConfigBoxItem*>( item );
    if ( !configItem )
	return;

    QString key = configItem->key;
    QString type = configItem->type;

    config->setGroup( "Resource_" + key );

    ResourceConfigDlg dlg( this, type, config, "ResourceConfigDlg" );

    dlg.resourceName->setText( config->readEntry( "ResourceName" ) );
    dlg.resourceIsReadOnly->setChecked( config->readBoolEntry( "ResourceIsReadOnly" ) );
    dlg.resourceIsFast->setChecked( config->readBoolEntry( "ResourceIsFast" ) );

    if ( dlg.exec() ) {
	config->writeEntry( "ResourceName", dlg.resourceName->text() );
	config->writeEntry( "ResourceType", type );
	config->writeEntry( "ResourceIsReadOnly", dlg.resourceIsReadOnly->isChecked() );
	config->writeEntry( "ResourceIsFast", dlg.resourceIsFast->isChecked() );

	configItem->setText( dlg.resourceName->text() );
	emit changed( true );
    }
}

void ConfigPageImpl::slotSelectionChanged()
{
    bool state = ( listBox->currentItem() != -1 );

    removeButton->setEnabled( state );
    editButton->setEnabled( state );
}

KCMkabc::KCMkabc( QWidget *parent, const char *name )
    : KCModule( parent, name )
{
    QVBoxLayout *layout = new QVBoxLayout( this );
    mConfigPage = new ConfigPageImpl( this );
    layout->addWidget( mConfigPage );
    connect( mConfigPage, SIGNAL( changed( bool ) ), SIGNAL( changed( bool ) ) );
}

void KCMkabc::load()
{
    mConfigPage->load();
}

void KCMkabc::save()
{
    mConfigPage->save();
}

void KCMkabc::defaults()
{
    mConfigPage->defaults();
}

extern "C"
{
    KCModule *create_kabc( QWidget *parent, const char * )
    {
	return new KCMkabc( parent, "kcmkabc" );
    }
}
#include "kcmkabc.moc"
