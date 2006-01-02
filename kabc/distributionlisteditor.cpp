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
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <QGroupBox>

#include <kinputdialog.h>
#include <klocale.h>
#include <kdebug.h>

#include "addressbook.h"
#include "addresseedialog.h"
#include "distributionlist.h"

#include "distributionlisteditor.h"
#include "distributionlisteditor.moc"

using namespace KABC;

EmailSelectDialog::EmailSelectDialog( const QStringList &emails, const QString &current,
                                      QWidget *parent ) :
  KDialogBase( KDialogBase::Plain, i18n("Select Email Address"), Ok, Ok,
               parent )
{
  QFrame *topFrame = plainPage();
  QBoxLayout *topLayout = new QVBoxLayout( topFrame );
  QGroupBox *box = new QGroupBox( i18n("Email Addresses") );
  mButtonGroup = new QButtonGroup( box );
  mButtonGroup->setExclusive( true );
  topLayout->addWidget( box );

  QStringList::ConstIterator it;
  for( it = emails.begin(); it != emails.end(); ++it ) {
    QRadioButton *button = new QRadioButton( *it, box );
    mButtonGroup->addButton( button );
    if ( (*it) == current ) {
      button->setChecked( true );
    }
  }
}

QString EmailSelectDialog::selected()
{
  QAbstractButton *button = mButtonGroup->checkedButton();
  if ( button ) return button->text();
  return QString();
}

QString EmailSelectDialog::getEmail( const QStringList &emails, const QString &current,
                                     QWidget *parent )
{
  EmailSelectDialog *dlg = new EmailSelectDialog( emails, current, parent );
  dlg->exec();

  QString result = dlg->selected();

  delete dlg;

  return result;
}

class EditEntryItem : public QTreeWidgetItem
{
  public:
    EditEntryItem( QTreeWidget *parent, const Addressee &addressee,
               const QString &email=QString() ) :
      QTreeWidgetItem( parent ),
      mAddressee( addressee ),
      mEmail( email )
    {
      setText( 0, addressee.realName() );
      if( email.isEmpty() ) {
        setText( 1, addressee.preferredEmail() );
        setText( 2, i18n("Yes") );
      } else {
        setText( 1, email );
        setText( 2, i18n("No") );
      }
    }

    Addressee addressee() const
    {
      return mAddressee;
    }

    QString email() const
    {
      return mEmail;
    }

  private:
    Addressee mAddressee;
    QString mEmail;
};

DistributionListEditor::DistributionListEditor( AddressBook *addressBook, QWidget *parent) :
  QWidget( parent ),
  mAddressBook( addressBook )
{
  kdDebug(5700) << "DistributionListEditor()" << endl;

  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );

  QBoxLayout *nameLayout = new QHBoxLayout( topLayout) ;

  mNameCombo = new QComboBox( this );
  nameLayout->addWidget( mNameCombo );
  connect( mNameCombo, SIGNAL( activated( int ) ), SLOT( updateEntryView() ) );

  newButton = new QPushButton( i18n("New List"), this );
  nameLayout->addWidget( newButton );
  connect( newButton, SIGNAL( clicked() ), SLOT( newList() ) );

  removeButton = new QPushButton( i18n("Remove List"), this );
  nameLayout->addWidget( removeButton );
  connect( removeButton, SIGNAL( clicked() ), SLOT( removeList() ) );

  mEntryView = new QTreeWidget( this );
  mEntryView->setColumnCount( 3 );
  QStringList labels;
  labels << i18n("Name") << i18n("Email") << i18n("Use Preferred");
  mEntryView->setHeaderLabels( labels );
  topLayout->addWidget( mEntryView );
  connect(mEntryView,SIGNAL(selectionChanged ()),this, SLOT(slotSelectionEntryViewChanged()));

  changeEmailButton = new QPushButton( i18n("Change Email"), this );
  topLayout->addWidget( changeEmailButton );
  connect( changeEmailButton, SIGNAL( clicked() ), SLOT( changeEmail() ) );

  removeEntryButton = new QPushButton( i18n("Remove Entry"), this );
  topLayout->addWidget( removeEntryButton );
  connect( removeEntryButton, SIGNAL( clicked() ), SLOT( removeEntry() ) );

  addEntryButton = new QPushButton( i18n("Add Entry"), this );
  topLayout->addWidget( addEntryButton );
  connect( addEntryButton, SIGNAL( clicked() ), SLOT( addEntry() ) );

  mAddresseeView = new QTreeWidget( this );
  mAddresseeView->setColumnCount( 2 );
  QStringList addresseeViewLabels;
  labels << i18n("Name") << i18n("Preferred Email");
  mAddresseeView->setHeaderLabels( addresseeViewLabels );
  topLayout->addWidget( mAddresseeView );

  connect(mAddresseeView,SIGNAL(itemSelectionChanged ()),this, SLOT(slotSelectionAddresseeViewChanged()));

  mManager = new DistributionListManager( mAddressBook );
  mManager->load();

  updateAddresseeView();
  updateNameCombo();
  removeButton->setEnabled(!mManager->listNames().isEmpty());
}

DistributionListEditor::~DistributionListEditor()
{
  kdDebug(5700) << "~DistributionListEditor()" << endl;

  mManager->save();
  delete mManager;
}

void DistributionListEditor::slotSelectionEntryViewChanged()
{
   QList<QTreeWidgetItem*> selected = mEntryView->selectedItems();
   bool state = ( selected.count() != 0 );

    changeEmailButton->setEnabled(state);
    removeEntryButton->setEnabled(state);
}

void DistributionListEditor::newList()
{
  bool ok = false;
  QString name = KInputDialog::getText( i18n("New Distribution List"),
                                        i18n("Please enter name:"),
                                        QString(), &ok, this );
  if ( !ok )
    return;

  new DistributionList( mManager, name );

  mNameCombo->insertItem( name );
  removeButton->setEnabled(true);
  updateEntryView();
}

void DistributionListEditor::removeList()
{
  mManager->remove( mManager->list( mNameCombo->currentText() ) );
  mNameCombo->removeItem( mNameCombo->currentItem() );
  removeButton->setEnabled(!mManager->listNames().isEmpty());
  addEntryButton->setEnabled( !mNameCombo->currentText().isEmpty());
  updateEntryView();
}

void DistributionListEditor::addEntry()
{
  QList<QTreeWidgetItem*> selected = mAddresseeView->selectedItems();
  if( selected.count() == 0 ) {
    kdDebug(5700) << "DLE::addEntry(): No addressee selected." << endl;
    return;
  }
  AddresseeItem *addresseeItem = static_cast<AddresseeItem *>( selected.at( 0 ) );


  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) {
    kdDebug(5700) << "DLE::addEntry(): No dist list '" << mNameCombo->currentText() << "'" << endl;
    return;
  }

  list->insertEntry( addresseeItem->addressee() );
  updateEntryView();
  slotSelectionAddresseeViewChanged();
}

void DistributionListEditor::removeEntry()
{
  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) return;

  QList<QTreeWidgetItem*> selected = mEntryView->selectedItems();
  if ( selected.count() == 0 ) return;
  EditEntryItem *entryItem =
      static_cast<EditEntryItem *>( selected.at( 0 ) );
  list->removeEntry( entryItem->addressee(), entryItem->email() );
  delete entryItem;
}

void DistributionListEditor::changeEmail()
{
  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) return;

  QList<QTreeWidgetItem*> selected = mEntryView->selectedItems();
  if( selected.count() == 0 ) return;

  EditEntryItem *entryItem =
      static_cast<EditEntryItem *>( selected.at( 0 ) );

  QString email = EmailSelectDialog::getEmail( entryItem->addressee().emails(),
                                               entryItem->email(), this );
  list->removeEntry( entryItem->addressee(), entryItem->email() );
  list->insertEntry( entryItem->addressee(), email );

  updateEntryView();
}

void DistributionListEditor::updateEntryView()
{
  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) return;

  mEntryView->clear();
  DistributionList::Entry::List entries = list->entries();
  DistributionList::Entry::List::ConstIterator it;
  for( it = entries.begin(); it != entries.end(); ++it ) {
    new EditEntryItem( mEntryView, (*it).addressee, (*it).email );
  }
   QList<QTreeWidgetItem*> selected = mEntryView->selectedItems();
   bool state = ( selected.count() != 0 );

   changeEmailButton->setEnabled(state);
   removeEntryButton->setEnabled(state);
}

void DistributionListEditor::updateAddresseeView()
{
  mAddresseeView->clear();

  AddressBook::Iterator it;
  for( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
    new AddresseeItem( mAddresseeView, *it );
  }
}

void DistributionListEditor::updateNameCombo()
{
  mNameCombo->insertStringList( mManager->listNames() );

  updateEntryView();
}

void DistributionListEditor::slotSelectionAddresseeViewChanged()
{
  QList<QTreeWidgetItem*> selected = mAddresseeView->selectedItems();
  bool state = (selected.count() != 0);
    addEntryButton->setEnabled( state && !mNameCombo->currentText().isEmpty());
}
