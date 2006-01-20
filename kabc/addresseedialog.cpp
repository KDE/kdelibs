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

#include <QGroupBox>
#include <QLayout>
#include <QPushButton>
#include <QRegExp>

#include <kdebug.h>
#include <klocale.h>

#include "stdaddressbook.h"

#include "addresseedialog.h"

using namespace KABC;

AddresseeItem::AddresseeItem( QTreeWidget *parent, const Addressee &addressee ) :
  QTreeWidgetItem( parent ),
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
    int match = emailRe.indexIn(value);
    if (match > -1)
      value = value.mid(match + 1, emailRe.matchedLength() - 2);

    return value.toLower();
  }

  return text(column).toLower();
}

AddresseeDialog::AddresseeDialog( QWidget *parent, bool multiple ) :
  KDialogBase( KDialogBase::Plain, i18n("Select Addressee"),
               Ok|Cancel, Ok, parent ), mMultiple( multiple )
{
  QWidget *topWidget = plainPage();

  QBoxLayout *topLayout = new QHBoxLayout( topWidget );
  QBoxLayout *listLayout = new QVBoxLayout;
  topLayout->addLayout( listLayout );

  mAddresseeList = new QTreeWidget( topWidget );
  mAddresseeList->setColumnCount( 2 );
  QStringList headerTitles;
  headerTitles << i18n("Name") << i18n("Email");
  mAddresseeList->setHeaderItem( new QTreeWidgetItem( headerTitles ) );
  listLayout->addWidget( mAddresseeList );
  connect( mAddresseeList, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
           SLOT( slotOk() ) );
  connect( mAddresseeList, SIGNAL( itemSelectionChanged() ),
           SLOT( updateEdit() ) );

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

    QGroupBox *selectedGroup = new QGroupBox( i18n("Selected"), topWidget );
    QHBoxLayout *groupLayout = new QHBoxLayout;
    selectedGroup->setLayout( groupLayout );
    selectedLayout->addWidget( selectedGroup );

    mSelectedList = new QTreeWidget( selectedGroup );
    groupLayout->addWidget( mSelectedList );
    mSelectedList->setColumnCount( 2 );
    QStringList headerTitles;
    headerTitles << i18n("Name") << i18n("Email");
    mSelectedList->setHeaderItem( new QTreeWidgetItem( headerTitles ) );

    connect( mSelectedList, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
             SLOT( removeSelected() ) );

    QPushButton *unselectButton = new QPushButton( i18n("Unselect"), selectedGroup );
    selectedLayout->addWidget( unselectButton );
    connect ( unselectButton, SIGNAL( clicked() ), SLOT( removeSelected() ) );

    connect( mAddresseeList, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ),
             SLOT( addSelected( QTreeWidgetItem * ) ) );

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

void AddresseeDialog::addCompletionItem( const QString &str, QTreeWidgetItem *item )
{
  if ( str.isEmpty() ) return;

  mItemDict.insert( str, item );
  mAddresseeEdit->completionObject()->addItem( str );
}

void AddresseeDialog::selectItem( const QString &str )
{
  if ( str.isEmpty() ) return;

  QTreeWidgetItem *item = mItemDict.value( str, 0 );
  if ( item ) {
    mAddresseeList->blockSignals( true );
    mAddresseeList->setItemSelected( item, true );
    mAddresseeList->scrollToItem( item );
    mAddresseeList->blockSignals( false );
  }
}

void AddresseeDialog::updateEdit()
{
  QList<QTreeWidgetItem*> selected = mAddresseeList->selectedItems();
  if ( selected.count() == 0 ) return;
  QTreeWidgetItem *item = selected.at( 0 );
  mAddresseeEdit->setText( item->text( 0 ) );
  mAddresseeEdit->setSelection( 0, item->text( 0 ).length() );
}

void AddresseeDialog::addSelected( QTreeWidgetItem *item )
{
  AddresseeItem *addrItem = dynamic_cast<AddresseeItem *>( item );
  if ( !addrItem ) return;

  Addressee a = addrItem->addressee();

  QTreeWidgetItem *selectedItem = mSelectedDict.value( a.uid(), 0 );
  if ( !selectedItem ) {
    selectedItem = new AddresseeItem( mSelectedList, a );
    mSelectedDict.insert( a.uid(), selectedItem );
  }
}

void AddresseeDialog::removeSelected()
{
  QList<QTreeWidgetItem*> selected = mAddresseeList->selectedItems();
  if ( selected.count() == 0 ) return;
  AddresseeItem *addrItem = dynamic_cast<AddresseeItem *>( selected.at( 0 ) );
  if ( !addrItem ) return;

  mSelectedDict.remove( addrItem->addressee().uid() );
  delete addrItem;
}

Addressee AddresseeDialog::addressee()
{
  AddresseeItem *aItem = 0;

  if ( mMultiple )
    aItem = dynamic_cast<AddresseeItem *>( mSelectedList->topLevelItem( 0 ) );
  else {
    QList<QTreeWidgetItem*> selected = mAddresseeList->selectedItems();
    if ( selected.count() != 0 )
      aItem = dynamic_cast<AddresseeItem *>( selected.at( 0 ) );
  }

  if (aItem) return aItem->addressee();
  return Addressee();
}

Addressee::List AddresseeDialog::addressees()
{
  Addressee::List al;
  AddresseeItem *aItem = 0;

  if ( mMultiple ) {
    for ( int i = 0; i < mSelectedList->topLevelItemCount(); i++ ) {
      aItem = dynamic_cast<AddresseeItem *>( mSelectedList->topLevelItem( i ) );
      if ( aItem )
        al.append( aItem->addressee() );
    }
  }
  else
  {
    QList<QTreeWidgetItem*> selected = mAddresseeList->selectedItems();
    if ( selected.count() != 0 )
      aItem = dynamic_cast<AddresseeItem *>( selected.at( 0 ) );
    if (aItem)
      al.append( aItem->addressee() );
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

#include "addresseedialog.moc"
