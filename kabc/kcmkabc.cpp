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
#include <qinputdialog.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include "kcmkabc.h"

#include "resource.h"
#include "resourceconfigdlg.h"
#include "resourcefactory.h"

class ConfigViewItem : public QListViewItem
{
public:
    ConfigViewItem( QListView *parent, QString name, QString type,
	    QString identifier = QString::null );

    QString key;
    QString type;
};

ConfigViewItem::ConfigViewItem( QListView *parent, QString name,
	QString type, QString identifier )
    : QListViewItem( parent, name, type, identifier )
{
}

ConfigPage::ConfigPage( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    setCaption( i18n( "Resource Configuration" ) );

    QVBoxLayout *mainLayout = new QVBoxLayout( this );

    QGroupBox *groupBox = new QGroupBox( i18n( "Resources" ), this );
    groupBox->setColumnLayout(0, Qt::Vertical );
    groupBox->layout()->setSpacing( 6 );
    groupBox->layout()->setMargin( 11 );
    QHBoxLayout *groupBoxLayout = new QHBoxLayout( groupBox->layout() );

    listView = new KListView( groupBox );
    listView->setAllColumnsShowFocus( true );
    listView->addColumn( i18n( "Name" ) );
    listView->addColumn( i18n( "Type" ) );
    listView->addColumn( i18n( "Location" ) );

    groupBoxLayout->addWidget( listView );

    KButtonBox *buttonBox = new KButtonBox( groupBox, Vertical );
    addButton = buttonBox->addButton( i18n( "&Add" ), this, SLOT(slotAdd()) );
    removeButton = buttonBox->addButton( i18n( "&Remove" ), this, SLOT(slotRemove()) );
    removeButton->setEnabled( false );
    editButton = buttonBox->addButton( i18n( "&Edit" ), this, SLOT(slotEdit()) );
    editButton->setEnabled( false );
    buttonBox->layout();

    groupBoxLayout->addWidget( buttonBox );

    mainLayout->addWidget( groupBox );

    connect( listView, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()) );
    connect( listView, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(slotEdit()) );

    config = 0;

    load();
}

void ConfigPage::load()
{
    QStringList keys;

    delete config;
    config = new KConfig( "kabcrc" );

    config->setGroup( "General" );
    keys = config->readListEntry( "ResourceKeys" );

    listView->clear();

    for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
	config->setGroup( "Resource_" + (*it) );
	ConfigViewItem *item = new ConfigViewItem( listView, 
		config->readEntry( "ResourceName" ),
		config->readEntry( "ResourceType" ) );

	item->key = (*it);
	item->type = config->readEntry( "ResourceType" );
    }

    emit changed( false );
}

void ConfigPage::save()
{
    QStringList keys;

    QListViewItem *item = listView->firstChild();
    while ( item != 0 ) {
	ConfigViewItem *configItem = dynamic_cast<ConfigViewItem*>( item );
	keys.append( configItem->key );
	item = item->itemBelow();
    }

    config->setGroup( "General" );
    config->writeEntry( "ResourceKeys", keys );

    config->sync();

    emit changed( false );
}

void ConfigPage::defaults()
{
    QStringList groups = config->groupList();
    QStringList::Iterator it;
    for ( it = groups.begin(); it != groups.end(); ++it )
	config->deleteGroup( (*it) );
    
    QString key = KApplication::randomString( 10 );
    QString type = "file";

    groups.clear();
    groups << key;

    config->setGroup( "General" );
    config->writeEntry( "ResourceKeys", groups );
    
    config->setGroup( "Resource_" + key );
    config->writeEntry( "ResourceName", "Default" );
    config->writeEntry( "ResourceType", type );
    config->writeEntry( "ResourceIsReadOnly", false );
    config->writeEntry( "ResourceIsFast", true );
    config->writeEntry( "FileFormat", 0 );
    config->writeEntry( "FileName", locateLocal( "data", "kabc/std.vcf" ) );

    listView->clear();

    ConfigViewItem *item = new ConfigViewItem( listView, "Default", type );
    item->key = key;
    item->type = type;
}

void ConfigPage::slotAdd()
{
    KABC::ResourceFactory *factory = KABC::ResourceFactory::self();
    QString key = KApplication::randomString( 10 );

    QStringList types = factory->resources();
    bool ok = false;
    QString type = QInputDialog::getItem( i18n( "Resource Configuration" ),
	    i18n( "Please select type of the new resource" ), types, 0, false, &ok, this );
    if ( !ok )
	return;

    config->setGroup( "Resource_" + key );

    ResourceConfigDlg dlg( this, type, config, "ResourceConfigDlg" );

    dlg.resourceName->setText( type + "-resource" );
    dlg.resourceIsFast->setChecked( true );

    if ( dlg.exec() ) {
	config->writeEntry( "ResourceName", dlg.resourceName->text() );
	config->writeEntry( "ResourceType", type );
	config->writeEntry( "ResourceIsReadOnly", dlg.resourceIsReadOnly->isChecked() );
	config->writeEntry( "ResourceIsFast", dlg.resourceIsFast->isChecked() );

	ConfigViewItem *item = new ConfigViewItem( listView,
		dlg.resourceName->text(), type );
	item->key = key;
	item->type = type;
	emit changed( true );
    } else {
	config->deleteGroup( "Resource_" + key );
    }
}

void ConfigPage::slotRemove()
{
    QListViewItem *item = listView->currentItem();
    QString key = dynamic_cast<ConfigViewItem*>( item )->key;

    config->deleteGroup( "Resource_" + key );

    listView->takeItem( item );
    delete item;

    emit changed( true );
}

void ConfigPage::slotEdit()
{
    QListViewItem *item = listView->currentItem();
    ConfigViewItem *configItem = dynamic_cast<ConfigViewItem*>( item );
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

	configItem->setText( 0, dlg.resourceName->text() );
	configItem->setText( 1, type );
	emit changed( true );
    }
}

void ConfigPage::slotSelectionChanged()
{
    bool state = ( listView->currentItem() != 0 );

    removeButton->setEnabled( state );
    editButton->setEnabled( state );
}

KCMkabc::KCMkabc( QWidget *parent, const char *name )
    : KCModule( parent, name )
{
    QVBoxLayout *layout = new QVBoxLayout( this );
    mConfigPage = new ConfigPage( this );
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
