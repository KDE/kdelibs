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

#include <klocale.h>
#include <kbuttonbox.h>
#include <klistbox.h>

#include <qgroupbox.h>
#include <qlayout.h>

#include <kglobalsettings.h>
#include <kiconloader.h>
#include <qpushbutton.h>

#include "resource.h"
#include "addressbook.h"

#include "resourceselectdialog.h"

using namespace KABC;

ResourceSelectDialog::ResourceSelectDialog( AddressBook *ab, QWidget *parent, const char *name )
    : KDialog( parent, name, true )
{
  setCaption( i18n( "Resource Selection" ) );
  resize( 300, 200 );

  QVBoxLayout *mainLayout = new QVBoxLayout( this );
  mainLayout->setMargin( marginHint() );

  QGroupBox *groupBox = new QGroupBox( 2, Qt::Horizontal,  this );
  groupBox->setTitle( i18n( "Resources" ) );

  mResourceId = new KListBox( groupBox );

  mainLayout->addWidget( groupBox );

  mainLayout->addSpacing( 10 );

  KButtonBox *buttonBox = new KButtonBox( this );

  buttonBox->addStretch();
  QPushButton* ok = buttonBox->addButton( i18n( "&OK" ), this, SLOT( accept() ) );
  QPushButton* cancel = buttonBox->addButton( i18n( "&Cancel" ), this, SLOT( reject() ) );
  if (KGlobalSettings::showIconsOnPushButtons()) {
    ok->setIconSet( SmallIconSet("button_ok") );
    cancel->setIconSet( SmallIconSet("button_cancel") );
  }

  buttonBox->layout();

  mainLayout->addWidget( buttonBox );

  // setup listbox
  uint counter = 0;
  QPtrList<Resource> list = ab->resources();
  for ( uint i = 0; i < list.count(); ++i ) {
    Resource *resource = list.at( i );
    if ( resource && !resource->readOnly() ) {
      mResourceMap.insert( counter, resource );
      mResourceId->insertItem( resource->resourceName() );
      counter++;
    }
  }

  mResourceId->setCurrentItem( 0 );
}

Resource *ResourceSelectDialog::resource()
{
  if ( mResourceId->currentItem() != -1 )
    return mResourceMap[ mResourceId->currentItem() ];
  else
    return 0;
}

Resource *ResourceSelectDialog::getResource( AddressBook *ab, QWidget *parent )
{
  QPtrList<Resource> resources = ab->resources();
  if ( resources.count() == 1 ) return resources.first();

  Resource *found = 0;
  Resource *r = resources.first();
  while( r ) {
    if ( !r->readOnly() ) {
      if ( found ) {
        found = 0;
        break;
      } else {
        found = r;
      }
    }
    r = resources.next();
  }
  if ( found ) return found;

  ResourceSelectDialog dlg( ab, parent );
  if ( dlg.exec() == KDialog::Accepted ) return dlg.resource();
  else return 0;
}

#include "resourceselectdialog.moc"
