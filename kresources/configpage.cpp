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

#include "resourcesconfigpage.h"

using namespace KRES;

class ConfigViewItem : public QCheckListItem
{
  public:
    ConfigViewItem( QListView *parent, Resource* resource ) :
      QCheckListItem( parent, resource->resourceName(), CheckBox ),
      mResource( resource ),
      mIsStandard( false )
    {
      setText( 1, mResource->type() );
      kdDebug() << "RESOURCE: " << mResource->resourceName() << " "
                << ( mResource->isActive() ? "Active" : "Not active" ) << endl;
      setOn( mResource->isActive() );
    }

    void setStandard( bool value )
    {
      setText( 2, ( value ? i18n( "yes" ) : QString( "" ) ) );
      mIsStandard = value;
    }

    bool standard() const { return mIsStandard; }
    bool readOnly() const { return mResource->readOnly(); }

    Resource *resource() { return mResource; }

  private:
    Resource* mResource;

    bool mIsStandard;
};

ResourcesConfigPage::ResourcesConfigPage( const QString &family, const QString &config,
                                          QWidget *parent, const char *name )
  : QWidget( parent, name ), mFamily( family ), mConfig( config )
{
  kdDebug(5650) << "ResourcesConfigPage::ResourcesConfigPage()" << endl;

  init();
}

ResourcesConfigPage::ResourcesConfigPage( const QString &family, QWidget *parent,
                                          const char *name )
  : QWidget( parent, name ), mFamily( family ), mConfig( QString::null )
{
  kdDebug(5650) << "ResourcesConfigPage::ResourcesConfigPage()" << endl;

  init();
}

ResourcesConfigPage::~ResourcesConfigPage()
{
  mManager->removeListener( this );
}

void ResourcesConfigPage::load()
{
  kdDebug(5650) << "ResourcesConfigPage::load()" << endl;

  delete mManager;
  mManager = new ResourceManager<Resource>( mFamily, mConfig );
  mManager->load();

  if ( !mManager )
    kdDebug(5650) << "ERROR: cannot create ResourceManager<Resource>( mFamily )" << endl;

  mListView->clear();

  if ( mManager->isEmpty() ) defaults();

  Resource *standardResource = mManager->standardResource();

  ResourceManager<Resource>::Iterator it;
  for ( it = mManager->begin(); it != mManager->end(); ++it ) {
    ConfigViewItem *item = new ConfigViewItem( mListView, *it );
    if ( *it == standardResource )
      item->setStandard( true );
  }

  mManager->addListener( this );

  if ( mListView->childCount() == 0 ) {
    defaults();
    emit changed( true );
    mManager->sync();
  } else {
    if ( !standardResource )
      KMessageBox::sorry( this, i18n( "There is no standard resource! Please select one." ) );

    emit changed( false );
  }
}

void ResourcesConfigPage::save()
{
  QListViewItem *item = mListView->firstChild();
  while ( item ) {
    ConfigViewItem *configItem = static_cast<ConfigViewItem*>( item );

    // check if standard resource
    if ( configItem->standard() && !configItem->readOnly() &&
         configItem->isOn() )
      mManager->setStandardResource( configItem->resource() );

    // check if active or passive resource
    configItem->resource()->setActive( configItem->isOn() );

    item = item->nextSibling();
  }
  mManager->sync();

  if ( !mManager->standardResource() )
    KMessageBox::sorry( this, i18n( "There is no valid standard resource! Please select one which is neither read-only nor inactive." ) );

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
  if( !resource ) {
    KMessageBox::error( this, i18n("Unable to create resource of type '%1'.")
                              .arg( type ) );
    return;
  }

  resource->setResourceName( type + "-resource" );

  ResourceConfigDlg dlg( this, mFamily, resource, "ResourceConfigDlg" );

  if ( dlg.exec() ) {
    mManager->add( resource );

    ConfigViewItem *item = new ConfigViewItem( mListView, resource );

    mLastItem = item;

    // if there are only read-only resources we'll set this resource
    // as standard resource
    if ( !resource->readOnly() ) {
      bool onlyReadOnly = true;
      QListViewItem *it = mListView->firstChild();
      while ( it != 0 ) {
        ConfigViewItem *confIt = static_cast<ConfigViewItem*>( it );
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
  ConfigViewItem *confItem = static_cast<ConfigViewItem*>( item );

  if ( !confItem )
    return;

  if ( confItem->standard() ) {
    KMessageBox::sorry( this, i18n( "You cannot remove your standard resource! Please select a new standard resource first." ) );
    return;
  }

  mManager->remove( confItem->resource() );

  if ( item == mLastItem )
    mLastItem = 0;

  mListView->takeItem( item );
  delete item;

  emit changed( true );
}

void ResourcesConfigPage::slotEdit()
{
  QListViewItem *item = mListView->currentItem();
  ConfigViewItem *configItem = static_cast<ConfigViewItem*>( item );
  if ( !configItem )
    return;

  Resource *resource = configItem->resource();

  ResourceConfigDlg dlg( this, mFamily, resource, "ResourceConfigDlg" );

  if ( dlg.exec() ) {
    configItem->setText( 0, resource->resourceName() );
    configItem->setText( 1, resource->type() );

    if ( configItem->standard() && configItem->readOnly() ) {
      KMessageBox::sorry( this, i18n( "You cannot use a read-only resource as standard!" ) );
      configItem->setStandard( false );
    }

    mManager->resourceChanged( resource );
    emit changed( true );
  }
}

void ResourcesConfigPage::slotStandard()
{
  ConfigViewItem *item = static_cast<ConfigViewItem*>( mListView->currentItem() );
  if ( !item )
    return;

  if ( item->readOnly() ) {
    KMessageBox::sorry( this, i18n( "You cannot use a read-only resource as standard!" ) );
    return;
  }

  if ( !item->isOn() ) {
    KMessageBox::sorry( this, i18n( "You cannot use an inactive resource as standard!" ) );
    return;
  }

  QListViewItem *it = mListView->firstChild();
  while ( it != 0 ) {
    ConfigViewItem *configItem = static_cast<ConfigViewItem*>( it );
    if ( configItem->standard() )
      configItem->setStandard( false );
    it = it->itemBelow();
  }

  item->setStandard( true );
  mManager->setStandardResource( item->resource() );
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
  kdDebug(5650) << "ResourcesConfigPage::resourceAdded( " << resource->resourceName() << " )" << endl;
  ConfigViewItem *item = new ConfigViewItem( mListView, resource );

  // FIXME: this sucks. This should be in the config file,
  // or application-dependent, in which case it's always Off
  item->setOn( false );

  mLastItem = item;

  emit changed( true );
}

void ResourcesConfigPage::resourceModified( Resource* resource )
{
  kdDebug(5650) << "ResourcesConfigPage::resourceModified( " << resource->resourceName() << " )" << endl;
}

void ResourcesConfigPage::resourceDeleted( Resource* resource )
{
  kdDebug(5650) << "ResourcesConfigPage::resourceDeleted( " << resource->resourceName() << " )" << endl;
}

void ResourcesConfigPage::slotItemClicked( QListViewItem *item )
{
  ConfigViewItem *configItem = static_cast<ConfigViewItem *>( item );
  if ( !configItem ) return;

  if ( configItem->standard() && !configItem->isOn() ) {
    KMessageBox::sorry( this, i18n( "You can't deactivate the standard resource, choose another standard resource first" ) );
    configItem->setOn( true );
    return;
  }

  if ( configItem->isOn() != configItem->resource()->isActive() ) {
    emit changed( true );
  }
}

void ResourcesConfigPage::init()
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

  connect( mListView, SIGNAL( selectionChanged() ),
           SLOT( slotSelectionChanged() ) );
  connect( mListView, SIGNAL( clicked( QListViewItem * ) ),
           SLOT( slotItemClicked( QListViewItem * ) ) );

  mManager = 0;
  mLastItem = 0;

  load();
}

#include "resourcesconfigpage.moc"

