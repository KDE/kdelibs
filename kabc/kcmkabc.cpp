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
#include <klistbox.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
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

ConfigPage::ConfigPage( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
//    resize( 328, 241 ); 
    setCaption( i18n( "Resource Configuration" ) );

    QVBoxLayout *mainLayout = new QVBoxLayout( this );

    QGroupBox *groupBox = new QGroupBox( i18n( "Resources" ), this );
    groupBox->setColumnLayout(0, Qt::Vertical );
    groupBox->layout()->setSpacing( 6 );
    groupBox->layout()->setMargin( 11 );
    QHBoxLayout *groupBoxLayout = new QHBoxLayout( groupBox->layout() );

    listBox = new KListBox( groupBox );

    groupBoxLayout->addWidget( listBox );

    KButtonBox *buttonBox = new KButtonBox( groupBox, Vertical );
    addButton = buttonBox->addButton( i18n( "&Add" ), this, SLOT(slotAdd()) );
    removeButton = buttonBox->addButton( i18n( "&Remove" ), this, SLOT(slotRemove()) );
    removeButton->setEnabled( false );
    editButton = buttonBox->addButton( i18n( "&Edit" ), this, SLOT(slotEdit()) );
    editButton->setEnabled( false );
    buttonBox->layout();

    groupBoxLayout->addWidget( buttonBox );

    mainLayout->addWidget( groupBox );

    connect( listBox, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()) );
    connect( listBox, SIGNAL(doubleClicked(QListBoxItem*)), this, SLOT(slotEdit()) );

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

    listBox->clear();

    for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
	config->setGroup( "Resource_" + (*it) );
	ConfigBoxItem *item = new ConfigBoxItem( config->readEntry( "ResourceName" ) +
	" (" + config->readEntry( "ResourceType" ) + ")" );

	item->key = (*it);
	item->type = config->readEntry( "ResourceType" );

	listBox->insertItem( item );
    }

    emit changed( false );
}

void ConfigPage::save()
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

    listBox->clear();

    ConfigBoxItem *item = new ConfigBoxItem( "Default (" + type + ")" );
    item->key = key;
    item->type = type;
    listBox->insertItem( item );
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

	ConfigBoxItem *item = new ConfigBoxItem( dlg.resourceName->text() + " (" + type + ")" );
	item->key = key;
	item->type = type;
	listBox->insertItem( item );
	emit changed( true );
    } else {
	config->deleteGroup( "Resource_" + key );
    }
}

void ConfigPage::slotRemove()
{
    QListBoxItem *item = listBox->item( listBox->currentItem() );
    QString key = dynamic_cast<ConfigBoxItem*>( item )->key;

    config->deleteGroup( "Resource_" + key );

    listBox->removeItem( listBox->currentItem() );
    emit changed( true );
}

void ConfigPage::slotEdit()
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

	configItem->setText( dlg.resourceName->text() + " (" + type + ")" );
	emit changed( true );
    }
}

void ConfigPage::slotSelectionChanged()
{
    bool state = ( listBox->currentItem() != -1 );

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
