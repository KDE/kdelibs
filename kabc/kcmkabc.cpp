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
*/

#include <qgroupbox.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include "kcmkabc.h"

#include "resource.h"
#include "resourceconfigdlg.h"
#include "resourcefactory.h"
#include "stdaddressbook.h"

class ConfigViewItem : public QCheckListItem
{
public:
  ConfigViewItem( QListView *parent, QString name, QString type,
      QString identifier = QString::null );

  void setStandard( bool value )
  {
    setText( 2, ( value ? i18n( "yes" ) : "" ) );
    isStandard = value;
  }

  bool standard() { return isStandard; }

  QString key;
  QString type;
  bool readOnly;

private:
  bool isStandard;
};

ConfigViewItem::ConfigViewItem( QListView *parent, QString name,
    QString type, QString )
  : QCheckListItem( parent, name, CheckBox )
{
  isStandard = false;
  readOnly = false;
  setText( 1, type );
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

  mListView = new KListView( groupBox );
  mListView->setAllColumnsShowFocus( true );
  mListView->addColumn( i18n( "Name" ) );
  mListView->addColumn( i18n( "Type" ) );
  mListView->addColumn( i18n( "Standard" ) );
  mListView->addColumn( i18n( "Location" ) );

  groupBoxLayout->addWidget( mListView );

  KButtonBox *buttonBox = new KButtonBox( groupBox, Vertical );
  mAddButton = buttonBox->addButton( i18n( "&Add..." ), this, SLOT(slotAdd()) );
  mRemoveButton = buttonBox->addButton( i18n( "&Remove" ), this, SLOT(slotRemove()) );
  mRemoveButton->setEnabled( false );
  mEditButton = buttonBox->addButton( i18n( "&Edit..." ), this, SLOT(slotEdit()) );
  mEditButton->setEnabled( false );
  mConvertButton = buttonBox->addButton( i18n( "&Convert..." ), this, SLOT(slotConvert()) );
  mConvertButton->setEnabled( false );
  mStandardButton = buttonBox->addButton( i18n( "&Use as Standard..." ), this, SLOT(slotStandard()) );
  mStandardButton->setEnabled( false );
  buttonBox->layout();

  groupBoxLayout->addWidget( buttonBox );

  mainLayout->addWidget( groupBox );

  connect( mListView, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()) );

  config = 0;
  mLastItem = 0;

  load();
}

void ConfigPage::load()
{
  delete config;
  config = new KConfig( "kabcrc" );

  config->setGroup( "General" );

  QStringList keys = config->readListEntry( "ResourceKeys" );
  uint numActiveKeys = keys.count();
  keys += config->readListEntry( "PassiveResourceKeys" );

  QString standardKey = config->readEntry( "Standard" );

  mListView->clear();

  uint counter = 0;
  bool haveStandardResource = false;
  for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
    config->setGroup( "Resource_" + (*it) );
    ConfigViewItem *item = new ConfigViewItem( mListView, 
    config->readEntry( "ResourceName" ),
    config->readEntry( "ResourceType" ) );

    item->key = (*it);
    item->type = config->readEntry( "ResourceType" );
    item->readOnly = config->readBoolEntry( "ResourceIsReadOnly" );
    if ( standardKey == (*it) ) {
      item->setStandard( true );
      haveStandardResource = true;
    }

    item->setOn( counter < numActiveKeys );

    counter++;
  }

  if ( mListView->childCount() == 0 ) {
    defaults();
    config->sync();
  } else {
    if ( !haveStandardResource )
      KMessageBox::error( this, i18n( "There is no standard resource! Please select one." ) );

    emit changed( false );
  }
}

void ConfigPage::save()
{
  QStringList activeKeys;
  QStringList passiveKeys;
  QString standardKey;

  config->setGroup( "General" );

  QListViewItem *item = mListView->firstChild();
  while ( item != 0 ) {
    ConfigViewItem *configItem = dynamic_cast<ConfigViewItem*>( item );

    // check if standard resource
    if ( configItem->standard() && !configItem->readOnly && configItem->isOn() )
      standardKey = configItem->key;

    // check if active or passive resource
    if ( ( (QCheckListItem*)item )->isOn() )
      activeKeys.append( configItem->key );
    else
      passiveKeys.append( configItem->key );

    item = item->itemBelow();
  }

  config->writeEntry( "ResourceKeys", activeKeys );
  config->writeEntry( "PassiveResourceKeys", passiveKeys );
  config->writeEntry( "Standard", standardKey );

  config->sync();

  if ( standardKey.isEmpty() )
    KMessageBox::error( this, i18n( "There is no valid standard resource! Please select one which is neither read-only nor inactive." ) );

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
  config->writeEntry( "Standard", key );
    
  config->setGroup( "Resource_" + key );
  config->writeEntry( "ResourceName", "Default" );
  config->writeEntry( "ResourceType", type );
  config->writeEntry( "ResourceIsReadOnly", false );
  config->writeEntry( "ResourceIsFast", true );
  config->writeEntry( "FileFormat", 0 );
  config->writeEntry( "FileName", KABC::StdAddressBook::fileName() );

  mListView->clear();

  ConfigViewItem *item = new ConfigViewItem( mListView, "Default", type );
  item->key = key;
  item->type = type;
  item->setStandard(true);
  item->setOn( true );

  mLastItem = item;

  emit changed( true );
}

void ConfigPage::slotAdd()
{
  KABC::ResourceFactory *factory = KABC::ResourceFactory::self();
  QString key = KApplication::randomString( 10 );

  QStringList types = factory->resources();
  bool ok = false;
  QString type = QInputDialog::getItem( i18n( "Resource Configuration" ),
	    i18n( "Please select type of the new resource:" ), types, 0, false, &ok, this );
  if ( !ok )
    return;

  config->setGroup( "Resource_" + key );

  ResourceConfigDlg dlg( this, type, config, "ResourceConfigDlg" );

  dlg.setResourceName( type + "-resource" );
  dlg.setFast( true );

  if ( dlg.exec() ) {
    config->writeEntry( "ResourceName", dlg.resourceName() );
    config->writeEntry( "ResourceType", type );
    config->writeEntry( "ResourceIsReadOnly", dlg.readOnly() );
    config->writeEntry( "ResourceIsFast", dlg.fast() );

    ConfigViewItem *item = new ConfigViewItem( mListView, dlg.resourceName(), type );
    item->key = key;
    item->type = type;
    item->readOnly = dlg.readOnly();
    item->setOn( true );

    mLastItem = item;

    // if there are only read-only resources we'll set this resource
    // as standard resource
    if ( !item->readOnly ) {
      bool onlyReadOnly = true;
      QListViewItem *it = mListView->firstChild();
      while ( it != 0 ) {
        ConfigViewItem *confIt = dynamic_cast<ConfigViewItem*>( it );
        if ( !confIt->readOnly && confIt != item )
          onlyReadOnly = false;

        it = it->itemBelow();
      }

      if ( onlyReadOnly )
        item->setStandard( true );
    }

    emit changed( true );
  } else {
    config->deleteGroup( "Resource_" + key );
  }
}

void ConfigPage::slotRemove()
{
  QListViewItem *item = mListView->currentItem();
  ConfigViewItem *confItem = dynamic_cast<ConfigViewItem*>( item );

  if ( !confItem )
    return;

  if ( confItem->standard() ) {
    KMessageBox::error( this, i18n( "You can't remove your standard resource!. Please select a new standard resource first." ) );
    return;
  }

  config->deleteGroup( "Resource_" + confItem->key );

  if ( item == mLastItem )
    mLastItem = 0;

  mListView->takeItem( item );
  delete item;

  emit changed( true );
}

void ConfigPage::slotEdit()
{
  QListViewItem *item = mListView->currentItem();
  ConfigViewItem *configItem = dynamic_cast<ConfigViewItem*>( item );
  if ( !configItem )
    return;

  QString key = configItem->key;
  QString type = configItem->type;

  config->setGroup( "Resource_" + key );

  ResourceConfigDlg dlg( this, type, config, "ResourceConfigDlg" );

  dlg.setResourceName( config->readEntry( "ResourceName" ) );
  dlg.setReadOnly( config->readBoolEntry( "ResourceIsReadOnly" ) );
  dlg.setFast( config->readBoolEntry( "ResourceIsFast" ) );

  if ( dlg.exec() ) {
    config->writeEntry( "ResourceName", dlg.resourceName() );
    config->writeEntry( "ResourceType", type );
    config->writeEntry( "ResourceIsReadOnly", dlg.readOnly() );
    config->writeEntry( "ResourceIsFast", dlg.fast() );

    configItem->setText( 0, dlg.resourceName() );
    configItem->setText( 1, type );
    configItem->readOnly = dlg.readOnly();

    if ( configItem->standard() && configItem->readOnly ) {
      KMessageBox::error( this, i18n( "You cannot use a read-only resource as standard!" ) );
      configItem->setStandard( false );
    }

    emit changed( true );
  }
}

void ConfigPage::slotConvert()
{
  QListViewItem *item = mListView->currentItem();
  QListViewItem *oldItem = item;
  ConfigViewItem *oldConfigItem = dynamic_cast<ConfigViewItem*>( item );
  if ( !oldConfigItem )
    return;

  bool isStandard = oldConfigItem->standard();

  KABC::ResourceFactory *factory = KABC::ResourceFactory::self();
  KABC::AddressBook ab;

  // ask for target resource
  int numItems = mListView->childCount();
  slotAdd();
  if ( numItems == mListView->childCount() || mLastItem == 0 ) // user clicked cancel button
    return;

  // create old resource
  QString oldKey = oldConfigItem->key;
  QString oldType = oldConfigItem->type;

  config->setGroup( "Resource_" + oldKey );
  KABC::Resource *oldResource = factory->resource( oldType, &ab, config );
  if ( !oldResource ) {
    KMessageBox::error( this, i18n( "Unable to create resource from type '%1'." ).arg( oldType ) );
    mListView->takeItem( mLastItem );
    delete mLastItem;
    mLastItem = 0;
    return;
  }

  if ( !ab.addResource( oldResource ) ) {
    KMessageBox::error( this, i18n( "Unable to add resource '%1' to address book." ).arg( oldResource->name() ) );
    delete oldResource;
    mListView->takeItem( mLastItem );
    delete mLastItem;
    mLastItem = 0;
    return;
  }

  // load addressees from old resource
  ab.load();

  // remove old resource
  ab.removeResource( oldResource );

  // create new Resource
  QString newKey, newType;

  ConfigViewItem *newConfigItem = dynamic_cast<ConfigViewItem*>( mLastItem );
  newConfigItem->setStandard( isStandard );
  newType = newConfigItem->type;
  newKey = newConfigItem->key;

  config->setGroup( "Resource_" + newKey );
  KABC::Resource *newResource = factory->resource( newType, &ab, config );
  if ( !newResource ) {
    KMessageBox::error( this, i18n( "Unable to create resource from type '%1'." ).arg( newType ) );
    mListView->takeItem( mLastItem );
    delete mLastItem;
    mLastItem = 0;
    return;
  }

  if ( !ab.addResource( newResource ) ) {
    KMessageBox::error( this, i18n( "Unable to add resource '%1' to address book." ).arg( newResource->name() ) );
    delete newResource;
    mListView->takeItem( mLastItem );
    delete mLastItem;
    mLastItem = 0;
    return;
  }

  KABC::AddressBook::Iterator it;
  for ( it = ab.begin(); it != ab.end(); ++it ) {
    (*it).setResource( newResource );
    (*it).setChanged( true );
  }

  KABC::Ticket *ticket = ab.requestSaveTicket( newResource );
  ab.save( ticket );

  // remove old resource
  config->deleteGroup( "Resource_" + oldKey );
  mListView->takeItem( oldItem );
  delete oldItem;

  emit changed( true );
  return;
}

void ConfigPage::slotStandard()
{
  ConfigViewItem *item = dynamic_cast<ConfigViewItem*>( mListView->currentItem() );
  if ( !item )
    return;

  if ( item->readOnly ) {
    KMessageBox::error( this, i18n( "You cannot use a read-only resource as standard!" ) );
    return;
  }

  if ( !item->isOn() ) {
    KMessageBox::error( this, i18n( "You cannot use an inactive resource as standard!" ) );
    return;
  }

  QListViewItem *it = mListView->firstChild();
  while ( it != 0 ) {
    ConfigViewItem *configItem = dynamic_cast<ConfigViewItem*>( it );
    if ( configItem->standard() )
      configItem->setStandard( false );
    it = it->itemBelow();
  }

  item->setStandard( true );
}

void ConfigPage::slotSelectionChanged()
{
  bool state = ( mListView->currentItem() != 0 );

  mRemoveButton->setEnabled( state );
  mEditButton->setEnabled( state );
  mConvertButton->setEnabled( state );
  mStandardButton->setEnabled( state );
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
  KCModule *create_kabc( QWidget *parent, const char * ) {
    return new KCMkabc( parent, "kcmkabc" );
  }
}

#include "kcmkabc.moc"
