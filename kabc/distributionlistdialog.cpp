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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qlistview.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <klineeditdlg.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "addressbook.h"
#include "addresseedialog.h"
#include "distributionlist.h"

#include "distributionlistdialog.h"
#include "distributionlistdialog.moc"

using namespace KABC;

DistributionListDialog::DistributionListDialog( AddressBook *addressBook, QWidget *parent)
    : KDialogBase( parent, "", true, i18n("Configure Distribution Lists"), Ok, Ok, true)
{
  mEditor = new DistributionListEditorWidget( addressBook, this );
  setMainWidget( mEditor );

  connect( this, SIGNAL( okClicked() ), mEditor, SLOT( save() ) );
}

DistributionListDialog::~DistributionListDialog()
{
}


EmailSelector::EmailSelector( const QStringList &emails, const QString &current,
                                      QWidget *parent ) :
  KDialogBase( KDialogBase::Plain, i18n("Select Email Address"), Ok, Ok,
               parent )
{
  QFrame *topFrame = plainPage();
  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  mButtonGroup = new QButtonGroup( 1, Horizontal, i18n("Email Addresses"),
                                   topFrame );
  topLayout->addWidget( mButtonGroup );

  QStringList::ConstIterator it;
  for( it = emails.begin(); it != emails.end(); ++it ) {
    QRadioButton *button = new QRadioButton( *it, mButtonGroup );
    if ( (*it) == current ) {
      button->setDown( true );
    }
  }
}

QString EmailSelector::selected()
{
  QButton *button = mButtonGroup->selected();
  if ( button ) return button->text();
  return QString::null;
}

QString EmailSelector::getEmail( const QStringList &emails, const QString &current,
                                     QWidget *parent )
{
  EmailSelector *dlg = new EmailSelector( emails, current, parent );
  dlg->exec();

  QString result = dlg->selected();

  delete dlg;

  return result;
}

class EntryItem : public QListViewItem
{
  public:
    EntryItem( QListView *parent, const Addressee &addressee,
               const QString &email=QString::null ) :
      QListViewItem( parent ),
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

DistributionListEditorWidget::DistributionListEditorWidget( AddressBook *addressBook, QWidget *parent) :
  QWidget( parent ),
  mAddressBook( addressBook )
{
  kdDebug(5700) << "DistributionListEditor()" << endl;

  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  QBoxLayout *nameLayout = new QHBoxLayout( topLayout) ;

  mNameCombo = new QComboBox( this );
  nameLayout->addWidget( mNameCombo );
  connect( mNameCombo, SIGNAL( activated( int ) ), SLOT( updateEntryView() ) );

  mNewButton = new QPushButton( i18n("New List..."), this );
  nameLayout->addWidget( mNewButton );
  connect( mNewButton, SIGNAL( clicked() ), SLOT( newList() ) );

  mEditButton = new QPushButton( i18n("Rename List..."), this );
  nameLayout->addWidget( mEditButton );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( editList() ) );

  mRemoveButton = new QPushButton( i18n("Remove List..."), this );
  nameLayout->addWidget( mRemoveButton );
  connect( mRemoveButton, SIGNAL( clicked() ), SLOT( removeList() ) );

  QGridLayout *gridLayout = new QGridLayout( topLayout, 3, 3 );
  gridLayout->setColStretch(1, 1);

  QLabel *listLabel = new QLabel( i18n("Available addresses:"), this );
  gridLayout->addWidget( listLabel, 0, 0 );

  mListLabel = new QLabel( this );
  gridLayout->addMultiCellWidget( mListLabel, 0, 0, 1, 2 );

  mAddresseeView = new QListView( this );
  mAddresseeView->addColumn( i18n("Name") );
  mAddresseeView->addColumn( i18n("Preferred Email") );
  gridLayout->addWidget( mAddresseeView, 1, 0 );
  connect( mAddresseeView, SIGNAL( selectionChanged() ),
           SLOT( slotSelectionAddresseeViewChanged() ) );
  connect( mAddresseeView, SIGNAL( doubleClicked( QListViewItem * ) ),
           SLOT( addEntry() ) );

  mAddEntryButton = new QPushButton( i18n("Add Entry"), this );
  mAddEntryButton->setEnabled(false);
  gridLayout->addWidget( mAddEntryButton, 2, 0 );
  connect( mAddEntryButton, SIGNAL( clicked() ), SLOT( addEntry() ) );

  mEntryView = new QListView( this );
  mEntryView->addColumn( i18n("Name") );
  mEntryView->addColumn( i18n("Email") );
  mEntryView->addColumn( i18n("Use Preferred") );
  mEntryView->setEnabled(false);
  gridLayout->addMultiCellWidget( mEntryView, 1, 1, 1, 2 );
  connect( mEntryView, SIGNAL( selectionChanged() ),
           SLOT( slotSelectionEntryViewChanged() ) );

  mChangeEmailButton = new QPushButton( i18n("Change Email..."), this );
  gridLayout->addWidget( mChangeEmailButton, 2, 1 );
  connect( mChangeEmailButton, SIGNAL( clicked() ), SLOT( changeEmail() ) );

  mRemoveEntryButton = new QPushButton( i18n("Remove Entry"), this );
  gridLayout->addWidget( mRemoveEntryButton, 2, 2 );
  connect( mRemoveEntryButton, SIGNAL( clicked() ), SLOT( removeEntry() ) );

  mManager = new DistributionListManager( mAddressBook );
  mManager->load();

  updateAddresseeView();
  updateNameCombo();
}

DistributionListEditorWidget::~DistributionListEditorWidget()
{
  kdDebug(5700) << "~DistributionListEditor()" << endl;

  delete mManager;
}

void DistributionListEditorWidget::save()
{
  mManager->save();
}

void DistributionListEditorWidget::slotSelectionEntryViewChanged()
{
  EntryItem *entryItem = static_cast<EntryItem *>( mEntryView->selectedItem() );
  bool state=entryItem;

  mChangeEmailButton->setEnabled(state);
  mRemoveEntryButton->setEnabled(state);
}

void DistributionListEditorWidget::newList()
{
  KLineEditDlg dlg(i18n("Please enter name:"), QString::null, this);
  dlg.setCaption(i18n("New Distribution List"));
  if (!dlg.exec()) return;

  new DistributionList( mManager, dlg.text() );

  mNameCombo->clear();
  mNameCombo->insertStringList( mManager->listNames() );
  mNameCombo->setCurrentItem( mNameCombo->count() - 1 );

  updateEntryView();
  slotSelectionAddresseeViewChanged();
}

void DistributionListEditorWidget::editList()
{
  QString oldName = mNameCombo->currentText();
  
  KLineEditDlg dlg(i18n("Please change name:"), oldName, this);
  dlg.setCaption(i18n("Distribution List"));
  if (!dlg.exec()) return;

  DistributionList *list = mManager->list( oldName );
  list->setName( dlg.text() );

  mNameCombo->clear();
  mNameCombo->insertStringList( mManager->listNames() );
  mNameCombo->setCurrentItem( mNameCombo->count() - 1 );

  updateEntryView();
  slotSelectionAddresseeViewChanged();
}

void DistributionListEditorWidget::removeList()
{
  int result = KMessageBox::warningContinueCancel( this,
      i18n("Delete distibution list '%1'?") .arg( mNameCombo->currentText() ),
      QString::null, i18n("Delete") );

  if ( result != KMessageBox::Continue ) return;

  delete mManager->list( mNameCombo->currentText() );
  mNameCombo->removeItem( mNameCombo->currentItem() );

  updateEntryView();
  slotSelectionAddresseeViewChanged();
}

void DistributionListEditorWidget::addEntry()
{
  AddresseeItem *addresseeItem =
      static_cast<AddresseeItem *>( mAddresseeView->selectedItem() );

  if( !addresseeItem ) {
    kdDebug(5700) << "DLE::addEntry(): No addressee selected." << endl;
    return;
  }

  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) {
    kdDebug(5700) << "DLE::addEntry(): No dist list '" << mNameCombo->currentText() << "'" << endl;
    return;
  }

  list->insertEntry( addresseeItem->addressee() );
  updateEntryView();
  slotSelectionAddresseeViewChanged();
}

void DistributionListEditorWidget::removeEntry()
{
  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) return;

  EntryItem *entryItem =
      static_cast<EntryItem *>( mEntryView->selectedItem() );
  if ( !entryItem ) return;

  list->removeEntry( entryItem->addressee(), entryItem->email() );
  delete entryItem;
}

void DistributionListEditorWidget::changeEmail()
{
  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) return;

  EntryItem *entryItem =
      static_cast<EntryItem *>( mEntryView->selectedItem() );
  if ( !entryItem ) return;

  QString email = EmailSelector::getEmail( entryItem->addressee().emails(),
                                           entryItem->email(), this );
  list->removeEntry( entryItem->addressee(), entryItem->email() );
  list->insertEntry( entryItem->addressee(), email );

  updateEntryView();
}

void DistributionListEditorWidget::updateEntryView()
{
  if ( mNameCombo->currentText().isEmpty() ) {
    mListLabel->setText( i18n("Selected addressees:") );
  } else {
    mListLabel->setText( i18n("Selected addresses in '%1':")
                         .arg( mNameCombo->currentText() ) );
  }

  mEntryView->clear();

  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) {
    mEditButton->setEnabled(false);
    mRemoveButton->setEnabled(false);
    mChangeEmailButton->setEnabled(false);
    mRemoveEntryButton->setEnabled(false);
    mAddresseeView->setEnabled(false);
    mEntryView->setEnabled(false);
    return;
  } else {
    mEditButton->setEnabled(true);
    mRemoveButton->setEnabled(true);
    mAddresseeView->setEnabled(true);
    mEntryView->setEnabled(true);
  }

  DistributionList::Entry::List entries = list->entries();
  DistributionList::Entry::List::ConstIterator it;
  for( it = entries.begin(); it != entries.end(); ++it ) {
    new EntryItem( mEntryView, (*it).addressee, (*it).email );
  }

  EntryItem *entryItem = static_cast<EntryItem *>( mEntryView->selectedItem() );
  bool state=entryItem;

  mChangeEmailButton->setEnabled(state);
  mRemoveEntryButton->setEnabled(state);
}

void DistributionListEditorWidget::updateAddresseeView()
{
  mAddresseeView->clear();

  AddressBook::Iterator it;
  for( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
    new AddresseeItem( mAddresseeView, *it );
  }
}

void DistributionListEditorWidget::updateNameCombo()
{
  mNameCombo->insertStringList( mManager->listNames() );

  updateEntryView();
}

void DistributionListEditorWidget::slotSelectionAddresseeViewChanged()
{
  AddresseeItem *addresseeItem =
      static_cast<AddresseeItem *>( mAddresseeView->selectedItem() );
  bool state=addresseeItem;
  mAddEntryButton->setEnabled( state && !mNameCombo->currentText().isEmpty());
}
