/*
    This file is part of libkresources.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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
#include <klistview.h>
#include <kbuttonbox.h>

#include "resource.h"
#include "resourceconfigdlg.h"
#include "resourcemanager.h"

#include "resourcesconfigpage.h"

using namespace KRES;

class ConfigViewItem : public QCheckListItem
{
public:
  ConfigViewItem( QListView *parent, Resource* resource, QString identifier = QString::null );

  void setStandard( bool value )
  {
    setText( 2, ( value ? i18n( "yes" ) : QString( "" ) ) );
    mIsStandard = value;
  }

  bool standard() { return mIsStandard; }
  bool readOnly() { return mResource->readOnly(); }

  Resource* mResource;

private:
  bool mIsStandard;
};

ConfigViewItem::ConfigViewItem( QListView *parent, Resource* resource, QString )
  : QCheckListItem( parent, resource->resourceName(), CheckBox )
{
  mResource = resource;
  mIsStandard = false;
  setText( 1, resource->type() );
}


ResourcesConfigPage::ResourcesConfigPage( const QString& resourceFamily, QWidget *parent, const char *name )
  : QWidget( parent, name ), mFamily( resourceFamily )
{
  kdDebug() << "ResourcesConfigPage::ResourcesConfigPage()" << endl;
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

  groupBoxLayout->addWidget( mListView );

  KButtonBox *buttonBox = new KButtonBox( groupBox, Vertical );
  mAddButton = buttonBox->addButton( i18n( "&Add..." ), this, SLOT(slotAdd()) );
  mRemoveButton = buttonBox->addButton( i18n( "&Remove" ), this, SLOT(slotRemove()) );
  mRemoveButton->setEnabled( false );
  mEditButton = buttonBox->addButton( i18n( "&Edit..." ), this, SLOT(slotEdit()) );
  mEditButton->setEnabled( false );
  mStandardButton = buttonBox->addButton( i18n( "&Use as Standard" ), this, SLOT(slotStandard()) );
  mStandardButton->setEnabled( false );
  buttonBox->layout();

  groupBoxLayout->addWidget( buttonBox );

  mainLayout->addWidget( groupBox );

  connect( mListView, SIGNAL(selectionChanged()), this, SLOT(slotSelectionChanged()) );

  mManager = 0;
  mLastItem = 0;

  load();
}

ResourcesConfigPage::~ResourcesConfigPage()
{
  mManager->removeListener( this );
}

void ResourcesConfigPage::load()
{
  kdDebug() << "ResourcesConfigPage::load()" << endl;

  delete mManager;
  mManager = new ResourceManager<Resource>( mFamily );
  if ( !mManager )
    kdDebug() << "ERROR: cannot create ResourceManager<Resource>( mFamily )" << endl;

  mListView->clear();
  QPtrList<Resource> activeResources = mManager->resources( true );
  QPtrList<Resource> passiveResources = mManager->resources( false );

  if ( activeResources.isEmpty() && passiveResources.isEmpty() ) {
    defaults();
    activeResources = mManager->resources( true );
    passiveResources = mManager->resources( false );
  }

  Resource *standardResource = mManager->standardResource();

  Resource *resource;
  for ( resource = activeResources.first(); resource; resource = activeResources.next() ) {
    ConfigViewItem *item = new ConfigViewItem( mListView, resource );
    item->setOn( true );
    if ( resource == standardResource )
      item->setStandard( true );
  }

  for ( resource = passiveResources.first(); resource; resource = passiveResources.next() ) {
    ConfigViewItem *item = new ConfigViewItem( mListView, resource );
    item->setOn( false );
    if ( resource == standardResource )
      item->setStandard( true );
  }

  mManager->addListener( this );

  if ( mListView->childCount() == 0 ) {
    defaults();
    emit changed( true );
    mManager->sync();
  } else {
    if ( !standardResource )
      KMessageBox::error( this, i18n( "There is no standard resource! Please select one." ) );

    emit changed( false );
  }
}

void ResourcesConfigPage::save()
{
  QListViewItem *item = mListView->firstChild();
  while ( item != 0 ) {
    ConfigViewItem *configItem = dynamic_cast<ConfigViewItem*>( item );

    // check if standard resource
    if ( configItem->standard() && !configItem->readOnly() && configItem->isOn() )
      mManager->setStandardResource( configItem->mResource );

    // check if active or passive resource
    mManager->setActive( configItem->mResource, ( (QCheckListItem*)item )->isOn() );

    item = item->itemBelow();
  }
  mManager->sync();

  if ( !mManager->standardResource() )
    KMessageBox::error( this, i18n( "There is no valid standard resource! Please select one which is neither read-only nor inactive." ) );

  emit changed( false );
}

void ResourcesConfigPage::defaults()
{
}

void ResourcesConfigPage::slotAdd()
{
  QStringList types = mManager->resourceTypeNames();
  bool ok = false;
  QString type = QInputDialog::getItem( i18n( "Resource Configuration" ),
	    i18n( "Please select type of the new resource:" ), types, 0, false, &ok, this );
  if ( !ok )
    return;

  // Create new resource
  Resource *resource = mManager->createResource( type );
  resource->setResourceName( type + "-resource" );

  ResourceConfigDlg dlg( this, mFamily, resource, "ResourceConfigDlg" );

  if ( dlg.exec() ) {
    mManager->add( resource );

    ConfigViewItem *item = new ConfigViewItem( mListView, resource );
    item->setOn( true );

    mLastItem = item;

    // if there are only read-only resources we'll set this resource
    // as standard resource
    if ( !resource->readOnly() ) {
      bool onlyReadOnly = true;
      QListViewItem *it = mListView->firstChild();
      while ( it != 0 ) {
        ConfigViewItem *confIt = dynamic_cast<ConfigViewItem*>( it );
        if ( !confIt->readOnly() && confIt != item )
          onlyReadOnly = false;

        it = it->itemBelow();
      }

      if ( onlyReadOnly )
        item->setStandard( true );
    }

    emit changed( true );
  } else {
    delete resource;
    resource = 0;
  }
}

void ResourcesConfigPage::slotRemove()
{
  QListViewItem *item = mListView->currentItem();
  ConfigViewItem *confItem = dynamic_cast<ConfigViewItem*>( item );

  if ( !confItem )
    return;

  if ( confItem->standard() ) {
    KMessageBox::error( this, i18n( "You cannot remove your standard resource! Please select a new standard resource first." ) );
    return;
  }

  mManager->remove( confItem->mResource );
  delete confItem->mResource;

  if ( item == mLastItem )
    mLastItem = 0;

  mListView->takeItem( item );
  delete item;

  emit changed( true );
}

void ResourcesConfigPage::slotEdit()
{
  QListViewItem *item = mListView->currentItem();
  ConfigViewItem *configItem = dynamic_cast<ConfigViewItem*>( item );
  if ( !configItem )
    return;

  Resource *resource = configItem->mResource;

  ResourceConfigDlg dlg( this, mFamily, resource, "ResourceConfigDlg" );

  if ( dlg.exec() ) {
    configItem->setText( 0, resource->resourceName() );
    configItem->setText( 1, resource->type() );

    if ( configItem->standard() && configItem->readOnly() ) {
      KMessageBox::error( this, i18n( "You cannot use a read-only resource as standard!" ) );
      configItem->setStandard( false );
    }

    mManager->resourceChanged( resource );
    emit changed( true );
  }
}

void ResourcesConfigPage::slotStandard()
{
  ConfigViewItem *item = dynamic_cast<ConfigViewItem*>( mListView->currentItem() );
  if ( !item )
    return;

  if ( item->readOnly() ) {
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
  mManager->setStandardResource( item->mResource );
}

void ResourcesConfigPage::slotSelectionChanged()
{
  bool state = ( mListView->currentItem() != 0 );

  mRemoveButton->setEnabled( state );
  mEditButton->setEnabled( state );
  mStandardButton->setEnabled( state );
}

void ResourcesConfigPage::resourceAdded( Resource* resource )
{
  kdDebug() << "ResourcesConfigPage::resourceAdded( " << resource->resourceName() << " )" << endl;
  ConfigViewItem *item = new ConfigViewItem( mListView, resource );

  // FIXME: this sucks. This should be in the config file,
  // or application-dependent, in which case it's always Off
  item->setOn( false );

  mLastItem = item;

  emit changed( true );
}

void ResourcesConfigPage::resourceModified( Resource* resource )
{
  kdDebug() << "ResourcesConfigPage::resourceModified( " << resource->resourceName() << " )" << endl;
}

void ResourcesConfigPage::resourceDeleted( Resource* resource )
{
  kdDebug() << "ResourcesConfigPage::resourceDeleted( " << resource->resourceName() << " )" << endl;
}

#include "resourcesconfigpage.moc"

