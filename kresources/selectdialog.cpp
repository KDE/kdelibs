/*
    This file is part of libkresources.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kbuttonbox.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qgroupbox.h>
#include <qlayout.h>

#include "resource.h"

#include "selectdialog.h"

using namespace KRES;

SelectDialog::SelectDialog( QPtrList<Resource> list, QWidget *parent,
                            const char *name )
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
  buttonBox->addButton( KStdGuiItem::ok(), this, SLOT( accept() ) );
  buttonBox->addButton( KStdGuiItem::cancel(), this, SLOT( reject() ) );
  buttonBox->layout();

  mainLayout->addWidget( buttonBox );

  // setup listbox
  uint counter = 0;
  for ( uint i = 0; i < list.count(); ++i ) {
    Resource *resource = list.at( i );
    if ( resource && !resource->readOnly() ) {
      mResourceMap.insert( counter, resource );
      mResourceId->insertItem( resource->resourceName() );
      counter++;
    }
  }

  mResourceId->setCurrentItem( 0 );
  connect( mResourceId, SIGNAL(returnPressed(QListBoxItem*)),
           SLOT(accept()) );
  connect( mResourceId, SIGNAL( executed( QListBoxItem* ) ),
           SLOT( accept() ) );
}

Resource *SelectDialog::resource()
{
  if ( mResourceId->currentItem() != -1 )
    return mResourceMap[ mResourceId->currentItem() ];
  else
    return 0;
}

Resource *SelectDialog::getResource( QPtrList<Resource> list, QWidget *parent )
{
  if ( list.count() == 0 ) {
    KMessageBox::error( parent, i18n( "There is no resource available!" ) );
    return 0;
  }

  if ( list.count() == 1 ) return list.first();

  // the following lines will return a writeable resource if only _one_ writeable
  // resource exists
  Resource *found = 0;
  Resource *it = list.first();
  while ( it ) {
    if ( !it->readOnly() ) {
      if ( found ) {
        found = 0;
	break;
      } else
        found = it;
    }
    it = list.next();
  }

  if ( found )
    return found;

  SelectDialog dlg( list, parent );
  if ( dlg.exec() == KDialog::Accepted ) return dlg.resource();
  else return 0;
}
