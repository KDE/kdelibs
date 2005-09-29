/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <qlayout.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qregexp.h>

#include <klocale.h>
#include <kdebug.h>

#include "stdaddressbook.h"

#include "addresseedialog.h"
#include "addresseedialog.moc"

using namespace KABC;

AddresseeItem::AddresseeItem( QListView *parent, const Addressee &addressee ) :
  QListViewItem( parent ),
  mAddressee( addressee )
{
  setText( Name, addressee.realName() );
  setText( Email, addressee.preferredEmail() );
}

QString AddresseeItem::key( int column, bool ) const
{
  if (column == Email) {
    QString value = text(Email);
    QRegExp emailRe("<\\S*>");
    int match = emailRe.search(value);
    if (match > -1)
      value = value.mid(match + 1, emailRe.matchedLength() - 2);

    return value.lower();
  }

  return text(column).lower();
}

AddresseeDialog::AddresseeDialog( QWidget *parent, bool multiple ) :
  KDialogBase( KDialogBase::Plain, i18n("Select Addressee"),
               Ok|Cancel, Ok, parent ), mMultiple( multiple )
{
  QWidget *topWidget = plainPage();

  QBoxLayout *topLayout = new QHBoxLayout( topWidget );
  QBoxLayout *listLayout = new QVBoxLayout;
  topLayout->addLayout( listLayout );

  mAddresseeList = new KListView( topWidget );
  mAddresseeList->addColumn( i18n("Name") );
  mAddresseeList->addColumn( i18n("Email") );
  mAddresseeList->setAllColumnsShowFocus( true );
  mAddresseeList->setFullWidth( true );
  listLayout->addWidget( mAddresseeList );
  connect( mAddresseeList, SIGNAL( doubleClicked( QListViewItem * ) ),
           SLOT( slotOk() ) );
  connect( mAddresseeList, SIGNAL( selectionChanged( QListViewItem * ) ),
           SLOT( updateEdit( QListViewItem * ) ) );

  mAddresseeEdit = new KLineEdit( topWidget );
  mAddresseeEdit->setCompletionMode( KGlobalSettings::CompletionAuto );
  connect( mAddresseeEdit->completionObject(), SIGNAL( match( const QString & ) ),
           SLOT( selectItem( const QString & ) ) );
  mAddresseeEdit->setFocus();
  mAddresseeEdit->completionObject()->setIgnoreCase( true );
  listLayout->addWidget( mAddresseeEdit );

  setInitialSize( QSize( 450, 300 ) );

  if ( mMultiple ) {
    QBoxLayout *selectedLayout = new QVBoxLayout;
    topLayout->addLayout( selectedLayout );
    topLayout->setSpacing( spacingHint() );

    QGroupBox *selectedGroup = new QGroupBox( 1, Horizontal, i18n("Selected"),
                                              topWidget );
    selectedLayout->addWidget( selectedGroup );

    mSelectedList = new KListView( selectedGroup );
    mSelectedList->addColumn( i18n("Name") );
    mSelectedList->addColumn( i18n("Email") );
    mSelectedList->setAllColumnsShowFocus( true );
    mSelectedList->setFullWidth( true );
    connect( mSelectedList, SIGNAL( doubleClicked( QListViewItem * ) ),
             SLOT( removeSelected() ) );

    QPushButton *unselectButton = new QPushButton( i18n("Unselect"), selectedGroup );
    connect ( unselectButton, SIGNAL( clicked() ), SLOT( removeSelected() ) );

    connect( mAddresseeList, SIGNAL( clicked( QListViewItem * ) ),
             SLOT( addSelected( QListViewItem * ) ) );

    setInitialSize( QSize( 650, 350 ) );
  }

  mAddressBook = StdAddressBook::self( true );
  connect( mAddressBook, SIGNAL( addressBookChanged( AddressBook* ) ),
           SLOT( addressBookChanged() ) );
  connect( mAddressBook, SIGNAL( loadingFinished( Resource* ) ),
           SLOT( addressBookChanged() ) );

  loadAddressBook();
}

AddresseeDialog::~AddresseeDialog()
{
}

void AddresseeDialog::loadAddressBook()
{
  mAddresseeList->clear();
  mItemDict.clear();
  mAddresseeEdit->completionObject()->clear();

  AddressBook::Iterator it;
  for( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
    AddresseeItem *item = new AddresseeItem( mAddresseeList, (*it) );
    addCompletionItem( (*it).realName(), item );
    addCompletionItem( (*it).preferredEmail(), item );
  }
}

void AddresseeDialog::addCompletionItem( const QString &str, QListViewItem *item )
{
  if ( str.isEmpty() ) return;

  mItemDict.insert( str, item );
  mAddresseeEdit->completionObject()->addItem( str );
}

void AddresseeDialog::selectItem( const QString &str )
{
  if ( str.isEmpty() ) return;

  QListViewItem *item = mItemDict.find( str );
  if ( item ) {
    mAddresseeList->blockSignals( true );
    mAddresseeList->setSelected( item, true );
    mAddresseeList->ensureItemVisible( item );
    mAddresseeList->blockSignals( false );
  }
}

void AddresseeDialog::updateEdit( QListViewItem *item )
{
  mAddresseeEdit->setText( item->text( 0 ) );
  mAddresseeEdit->setSelection( 0, item->text( 0 ).length() );
}

void AddresseeDialog::addSelected( QListViewItem *item )
{
  AddresseeItem *addrItem = dynamic_cast<AddresseeItem *>( item );
  if ( !addrItem ) return;

  Addressee a = addrItem->addressee();

  QListViewItem *selectedItem = mSelectedDict.find( a.uid() );
  if ( !selectedItem ) {
    selectedItem = new AddresseeItem( mSelectedList, a );
    mSelectedDict.insert( a.uid(), selectedItem );
  }
}

void AddresseeDialog::removeSelected()
{
  QListViewItem *item = mSelectedList->selectedItem();
  AddresseeItem *addrItem = dynamic_cast<AddresseeItem *>( item );
  if ( !addrItem ) return;

  mSelectedDict.remove( addrItem->addressee().uid() );
  delete addrItem;
}

Addressee AddresseeDialog::addressee()
{
  AddresseeItem *aItem = 0;

  if ( mMultiple )
    aItem = dynamic_cast<AddresseeItem *>( mSelectedList->firstChild() );
  else
    aItem = dynamic_cast<AddresseeItem *>( mAddresseeList->selectedItem() );

  if (aItem) return aItem->addressee();
  return Addressee();
}

Addressee::List AddresseeDialog::addressees()
{
  Addressee::List al;
  AddresseeItem *aItem = 0;

  if ( mMultiple ) {
    QListViewItem *item = mSelectedList->firstChild();
    while( item ) {
      aItem = dynamic_cast<AddresseeItem *>( item );
      if ( aItem ) al.append( aItem->addressee() );
      item = item->nextSibling();
    }
  }
  else
  {
    aItem = dynamic_cast<AddresseeItem *>( mAddresseeList->selectedItem() );
    if (aItem) al.append( aItem->addressee() );
  }

  return al;
}

Addressee AddresseeDialog::getAddressee( QWidget *parent )
{
  AddresseeDialog *dlg = new AddresseeDialog( parent );
  Addressee addressee;
  int result = dlg->exec();

  if ( result == QDialog::Accepted ) {
    addressee =  dlg->addressee();
  }

  delete dlg;
  return addressee;
}

Addressee::List AddresseeDialog::getAddressees( QWidget *parent )
{
  AddresseeDialog *dlg = new AddresseeDialog( parent, true );
  Addressee::List addressees;
  int result = dlg->exec();
  if ( result == QDialog::Accepted ) {
    addressees =  dlg->addressees();
  }

  delete dlg;
  return addressees;
}

void AddresseeDialog::addressBookChanged()
{
  loadAddressBook();
}
