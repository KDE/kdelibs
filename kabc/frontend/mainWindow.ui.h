/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qmessagebox.h>

#include <klocale.h>
#include <kdebug.h>

using namespace KABC;

class AddresseeItem : public QListViewItem
{
  public:
    AddresseeItem( QListView *parent, const Addressee &a ) :
      QListViewItem( parent, a.realName(), a.uid() ), mAddressee( a ) {}
      
    void setAddressee( const Addressee &a ) { mAddressee = a; }
    Addressee &addressee() { return mAddressee; }
    
  private:
    Addressee mAddressee;
};

void MainWindow::init()
{
  mAddressBook = new KABC::AddressBook;
  mCurrentItem = 0;

  mEmailListView->header()->hide();
  mCategoryListView->header()->hide();

  connect( mAddressBook, SIGNAL( addressBookChanged( AddressBook * ) ),
           SLOT( addressBookChanged() ) );
  connect( mAddressBook, SIGNAL( addressBookLocked( AddressBook * ) ),
           SLOT( addressBookLocked() ) );
  connect( mAddressBook, SIGNAL( addressBookUnlocked( AddressBook * ) ),
           SLOT( addressBookUnlocked() ) );
}

void MainWindow::destroy()
{
  delete mAddressBook;
}

void MainWindow::fileSave()
{
  updateAddressee( mAddresseeList->selectedItem() );

  if ( mAddressBook->fileName().isEmpty() ) {
    QString fn = QFileDialog::getSaveFileName();
    if ( fn.isEmpty() ) return;
    mAddressBook->setFileName( fn );
  }
    
  AddressBook::Ticket *ticket = mAddressBook->requestSaveTicket();
  if ( !ticket ) {
    QMessageBox::critical( this, i18n("Save Error"), i18n("Can't save file '%1'.\n"
      "File is locked by another process."), QMessageBox::Ok, QMessageBox::NoButton,
      QMessageBox::NoButton );
    return;
  }
  mAddressBook->save( ticket );
}

void MainWindow::fileOpen()
{
  QString fileName = QFileDialog::getOpenFileName();
    
  loadAddressBook( fileName );
}

void MainWindow::loadAddressBook( const QString &fileName )
{
  if ( !mAddressBook->load( fileName ) ) return;

  mAddresseeList->clear();
  mCurrentItem = 0;
  mCurrentAddress = QString::null;
  readAddressee( Addressee() );
    
  KABC::AddressBook::Iterator it;
  for( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
    new AddresseeItem( mAddresseeList, (*it) );
  }
}

void MainWindow::updateAddressee( QListViewItem *item )
{
  AddresseeItem *addresseeItem = dynamic_cast<AddresseeItem *>( item );
  if ( !addresseeItem ) return;
    
  if (mCurrentItem ) {
    writeAddress( mCurrentAddress );
    writePhone( mCurrentPhone );
    Addressee a = writeAddressee( mCurrentItem->addressee() );
    mCurrentItem->setAddressee( a );
    mAddressBook->insertAddressee( a );
  }
  mCurrentItem = addresseeItem;

  readAddressee( addresseeItem->addressee() );
  updateAddress( mAddressIdCombo->currentItem() );
  updatePhone( mPhoneIdCombo->currentItem() );
}


void MainWindow::readAddressee( const KABC::Addressee &a )
{
  kdDebug() << "MainWindow::readAddressee(): " << a.name() << endl;  
    
  mNameEdit->setText( a.name() );
  mUrlEdit->setText( a.url().url() );
  mAdditionalNameEdit->setText( a.additionalName() );
  mSuffixEdit->setText( a.suffix() );
  mGivenNameEdit->setText( a.givenName() );
  mPrefixEdit->setText( a.prefix() );
  mFamilyNameEdit->setText( a.familyName() );
  mFormattedNameEdit->setText( a.formattedName() );
  mNickNameEdit->setText( a.nickName() );
  mSortStringEdit->setText( a.sortString() );
  mTitleEdit->setText( a.title() );
  mRoleEdit->setText( a.role() );
  mOrganizationEdit->setText( a.organization() );
  mNoteEdit->setText( a.note() );
//  mLabelEdit->setText( a.label() );

  mEmailListView->clear();  
  QStringList emails = a.emails();
  QStringList::ConstIterator it3;
  for( it3 = emails.begin(); it3 != emails.end(); ++it3 ) {
    new QListViewItem( mEmailListView, *it3 );
  }
  
  mCategoryListView->clear();  
  QStringList categories = a.categories();
  QStringList::ConstIterator it4;
  for( it4 = categories.begin(); it4 != categories.end(); ++it4 ) {
    new QListViewItem( mCategoryListView, *it4 );
  }
  
  mCustomListView->clear();  
  QStringList customs = a.customs();
  QStringList::ConstIterator it5;
  for( it5 = customs.begin(); it5 != customs.end(); ++it5 ) {
    int posDash = (*it5).find( "-" );
    int posColon = (*it5).find( ":" );
  
    QString app = (*it5).left( posDash );
    QString cname = (*it5).mid( posDash + 1, posColon - posDash - 1 );
    QString value = (*it5).mid( posColon + 1 );
  
    new QListViewItem( mCustomListView, app, cname, value );
  }
  
  Address::List addresses = a.addresses();
  mAddressIdCombo->clear();
  Address::List::ConstIterator it;
  for( it = addresses.begin(); it != addresses.end(); ++it ) {
    mAddressIdCombo->insertItem( (*it).id() );
  }
  if ( mAddressIdCombo->count() > 0 ) mCurrentAddress = mAddressIdCombo->currentText();
  else mCurrentAddress = QString::null;
  readAddress( mCurrentAddress );

  mPhoneIdCombo->clear();
  PhoneNumber::List pl = a.phoneNumbers();
  PhoneNumber::List::ConstIterator it2;
  for( it2 = pl.begin(); it2 != pl.end(); ++it2) {
    mPhoneIdCombo->insertItem( (*it2).id() );
  }
  if ( mPhoneIdCombo->count() > 0 ) mCurrentPhone = mPhoneIdCombo->currentText();
  readPhone( mCurrentPhone );
}

KABC::Addressee MainWindow::writeAddressee( const KABC::Addressee &addressee )
{
  Addressee a( addressee );
  a.setName( mNameEdit->text() );
  if ( !mUrlEdit->text().isEmpty() ) {
    a.setUrl( KURL( mUrlEdit->text() ) );
  }
  
  a.setAdditionalName( mAdditionalNameEdit->text() );
  a.setSuffix( mSuffixEdit->text() );
  a.setGivenName( mGivenNameEdit->text() );
  a.setPrefix( mPrefixEdit->text() );
  a.setFamilyName( mFamilyNameEdit->text() );
  a.setFormattedName( mFormattedNameEdit->text() );
  a.setNickName( mNickNameEdit->text() );
  a.setSortString( mSortStringEdit->text() );
  a.setTitle( mTitleEdit->text() );
  a.setRole( mRoleEdit->text() );
  a.setOrganization( mOrganizationEdit->text() );
  a.setNote( mNoteEdit->text() );
//  a.setLabel( mLabelEdit->text() );

  kdDebug() << "MainWindow::writeAddressee()" << endl;
  a.dump();

  return a;
}

void MainWindow::newEntry()
{
  bool ok = false;
  QString name = QInputDialog::getText( i18n("New Address Book Entry"),
                                        i18n("Please enter name."),
                                        QLineEdit::Normal, QString::null, &ok,
                                        this );
  if ( !ok || name.isEmpty() ) return;
  
  Addressee a;
  a.setName( name );
  mAddressBook->insertAddressee( a );
  
  new AddresseeItem( mAddresseeList, a );
}

void MainWindow::removeEntry()
{
  AddresseeItem *item = dynamic_cast<AddresseeItem *>(mAddresseeList->selectedItem());
  if ( item ) {
    mAddressBook->removeAddressee( item->addressee() );
    delete item;
    mCurrentItem = 0;
  }
}


void MainWindow::updateAddress( int id )
{
  if( !mCurrentItem ) return;
  
  writeAddress( mCurrentAddress );
  if ( mAddressIdCombo->count()  > 0 ) {
    mCurrentAddress = mAddressIdCombo->text( id );
  } else {
    mCurrentAddress = QString::null;
  }
  readAddress( mCurrentAddress );
}

KABC::Address MainWindow::writeAddress( const KABC::Address &address )
{
  Address a( address );
  
  a.setPostOfficeBox( mAddressPostOfficeBoxEdit->text() );
  a.setExtended( mAddressExtendedEdit->text() );
  a.setStreet( mAddressStreetEdit->text() );
  a.setLocality( mAddressLocalityEdit->text() );
  a.setRegion( mAddressRegionEdit->text() );
  a.setLabel( mAddressLabelEdit->text() );
  a.setPostalCode( mAddressPostalCodeEdit->text() );
  a.setCountry( mAddressCountryEdit->text() );
  
  int type = 0;
  if ( mAddressDomCheck->isChecked() ) type |= Address::Dom;
  if ( mAddressIntlCheck->isChecked() ) type |= Address::Intl;
  if ( mAddressParcelCheck->isChecked() ) type |= Address::Parcel;
  if ( mAddressPostalCheck->isChecked() ) type |= Address::Postal;
  if ( mAddressHomeCheck->isChecked() ) type |= Address::Home;
  if ( mAddressPrefCheck->isChecked() ) type |= Address::Pref;
  if ( mAddressWorkCheck->isChecked() ) type |= Address::Work;
  a.setType( type );
  
  return a;
}

void MainWindow::writeAddress( const QString &id )
{
  if ( !mCurrentItem ) return;
 
  if ( id.isEmpty() ) return;
 
  Address address;
  address.setId( id );
  address = writeAddress( address );
 
  mCurrentItem->addressee().insertAddress( address );
}

void MainWindow::readAddress( const KABC::Address &a )
{
  mAddressPostOfficeBoxEdit->setText( a.postOfficeBox() );
  mAddressExtendedEdit->setText( a.extended() );
  mAddressStreetEdit->setText( a.street() );
  mAddressLocalityEdit->setText( a.locality() );
  mAddressRegionEdit->setText( a.region() );
  mAddressLabelEdit->setText( a.label() );
  mAddressPostalCodeEdit->setText( a.postalCode() );
  mAddressCountryEdit->setText( a.country() );
  
  int type = a.type();
  if ( type & Address::Dom ) mAddressDomCheck->setChecked( true );
  else mAddressDomCheck->setChecked( false );
  if ( type & Address::Intl ) mAddressIntlCheck->setChecked( true );
  else mAddressIntlCheck->setChecked( false );
  if ( type & Address::Parcel ) mAddressParcelCheck->setChecked( true );
  else mAddressParcelCheck->setChecked( false );
  if ( type & Address::Postal ) mAddressPostalCheck->setChecked( true );
  else mAddressPostalCheck->setChecked( false );
  if ( type & Address::Home ) mAddressHomeCheck->setChecked( true );
  else mAddressHomeCheck->setChecked( false );
  if ( type & Address::Pref ) mAddressPrefCheck->setChecked( true );
  else mAddressPrefCheck->setChecked( false );
  if ( type & Address::Work ) mAddressWorkCheck->setChecked( true );
  else mAddressWorkCheck->setChecked( false );
}

void MainWindow::readAddress( const QString &id )
{
  if ( !mCurrentItem || id.isEmpty() ) {
    readAddress( Address() );
    return;
  }

  Address address = mCurrentItem->addressee().findAddress( id );
  readAddress( address );
}

void MainWindow::updatePhone( int id )
{
  if( !mCurrentItem ) return;
  
  writePhone( mCurrentPhone );
  if ( mPhoneIdCombo->count()  > 0 ) {
    mCurrentPhone = mPhoneIdCombo->text( id );
  } else {
    mCurrentPhone = QString::null;
  }
  readPhone( mCurrentPhone );
}

KABC::PhoneNumber MainWindow::writePhone( const KABC::PhoneNumber &phoneNumber )
{
  PhoneNumber p( phoneNumber );

  p.setNumber( mPhoneNumberEdit->text() );
  
  int type = 0;
  if ( mPhoneMsgCheck->isChecked() ) type |= PhoneNumber::Msg;
  if ( mPhoneVoiceCheck->isChecked() ) type |= PhoneNumber::Voice;
  if ( mPhoneFaxCheck->isChecked() ) type |= PhoneNumber::Fax;
  if ( mPhoneCellCheck->isChecked() ) type |= PhoneNumber::Cell;
  if ( mPhoneHomeCheck->isChecked() ) type |= PhoneNumber::Home;
  if ( mPhonePrefCheck->isChecked() ) type |= PhoneNumber::Pref;
  if ( mPhoneWorkCheck->isChecked() ) type |= PhoneNumber::Work;
  if ( mPhoneVideoCheck->isChecked() ) type |= PhoneNumber::Video;
  if ( mPhoneBbsCheck->isChecked() ) type |= PhoneNumber::Bbs;
  if ( mPhoneModemCheck->isChecked() ) type |= PhoneNumber::Modem;
  if ( mPhoneCarCheck->isChecked() ) type |= PhoneNumber::Car;
  if ( mPhoneIsdnCheck->isChecked() ) type |= PhoneNumber::Isdn;
  if ( mPhonePcsCheck->isChecked() ) type |= PhoneNumber::Pcs;
  if ( mPhonePagerCheck->isChecked() ) type |= PhoneNumber::Pager;
  p.setType( type );
  
  return p;
}

void MainWindow::writePhone( const QString &id )
{
  if ( !mCurrentItem ) return;
 
  if ( id.isEmpty() ) return;
 
  PhoneNumber p;
  p.setId( id );
  p = writePhone( p );
 
  mCurrentItem->addressee().insertPhoneNumber( p );
}

void MainWindow::readPhone( const KABC::PhoneNumber &p )
{
  mPhoneNumberEdit->setText( p.number() );
  
  int type = p.type();
  if ( type & PhoneNumber::Msg ) mPhoneMsgCheck->setChecked( true );
  else mPhoneMsgCheck->setChecked( false );
  if ( type & PhoneNumber::Voice ) mPhoneVoiceCheck->setChecked( true );
  else mPhoneVoiceCheck->setChecked( false );
  if ( type & PhoneNumber::Fax ) mPhoneFaxCheck->setChecked( true );
  else mPhoneFaxCheck->setChecked( false );
  if ( type & PhoneNumber::Cell ) mPhoneCellCheck->setChecked( true );
  else mPhoneCellCheck->setChecked( false );
  if ( type & PhoneNumber::Home ) mPhoneHomeCheck->setChecked( true );
  else mPhoneHomeCheck->setChecked( false );
  if ( type & PhoneNumber::Pref ) mPhonePrefCheck->setChecked( true );
  else mPhonePrefCheck->setChecked( false );
  if ( type & PhoneNumber::Work ) mPhoneWorkCheck->setChecked( true );
  else mPhoneWorkCheck->setChecked( false );
  if ( type & PhoneNumber::Video ) mPhoneVideoCheck->setChecked( true );
  else mPhoneVideoCheck->setChecked( false );
  if ( type & PhoneNumber::Bbs ) mPhoneBbsCheck->setChecked( true );
  else mPhoneBbsCheck->setChecked( false );
  if ( type & PhoneNumber::Modem ) mPhoneModemCheck->setChecked( true );
  else mPhoneModemCheck->setChecked( false );
  if ( type & PhoneNumber::Car ) mPhoneCarCheck->setChecked( true );
  else mPhoneCarCheck->setChecked( false );
  if ( type & PhoneNumber::Isdn ) mPhoneIsdnCheck->setChecked( true );
  else mPhoneIsdnCheck->setChecked( false );
  if ( type & PhoneNumber::Pcs ) mPhonePcsCheck->setChecked( true );
  else mPhonePcsCheck->setChecked( false );
  if ( type & PhoneNumber::Pager ) mPhonePagerCheck->setChecked( true );
  else mPhonePagerCheck->setChecked( false );
}

void MainWindow::readPhone( const QString &id )
{
  if ( !mCurrentItem || id.isEmpty() ) {
    readPhone( PhoneNumber() );
    return;
  }

  PhoneNumber p = mCurrentItem->addressee().findPhoneNumber( id );
  readPhone( p );
}


void MainWindow::newAddress()
{
  if( !mCurrentItem ) return;
  
  Address address;
  mCurrentItem->addressee().insertAddress( address );
  
  mAddressIdCombo->insertItem( address.id() );
}

void MainWindow::removeAddress()
{
  if ( !mCurrentItem ) return;
  
  QString id = mAddressIdCombo->currentText();
  if ( id.isEmpty() ) return;
 	 
  Address address;
  address.setId( id );
  mCurrentItem->addressee().removeAddress( address );
  
  readAddressee( mCurrentItem->addressee() );
}


void MainWindow::dumpAddressBook()
{
  mAddressBook->dump();
}


void MainWindow::newEmail()
{
  if ( !mCurrentItem ) return;
  
  bool ok = false;
  QString name = QInputDialog::getText( i18n("New Email Address"),
                                        i18n("Please enter email address."),
                                        QLineEdit::Normal, QString::null, &ok,
                                        this );
  if ( !ok || name.isEmpty() ) return;
  
  new QListViewItem( mEmailListView, name );
  mCurrentItem->addressee().insertEmail( name );
}

void MainWindow::editEmail()
{
  if ( !mCurrentItem ) return;
  
  QListViewItem *item = mEmailListView->selectedItem();
  if( !item ) return;

  QString oldName = item->text( 0 );

  bool ok = false;
  QString name = QInputDialog::getText( i18n("Edit Email Address"),
                                        i18n("Please enter new email address."),
                                        QLineEdit::Normal, oldName, &ok,
                                        this );
  if ( !ok || name.isEmpty() ) return;

  item->setText( 0, name );
  mCurrentItem->addressee().removeEmail( oldName );
  mCurrentItem->addressee().insertEmail( name );
}

void MainWindow::removeEmail()
{
  if ( !mCurrentItem ) return;
  
  QListViewItem *item = mEmailListView->selectedItem();
  if( !item ) return;

  mCurrentItem->addressee().removeEmail( item->text( 0 ) );
  delete item;  
}

void MainWindow::newPhoneNumber()
{
  if ( !mCurrentItem ) return;
  
  PhoneNumber p;
  mCurrentItem->addressee().insertPhoneNumber( p );
  
  mPhoneIdCombo->insertItem( p.id() );
}

void MainWindow::removePhoneNumber()
{
  if ( !mCurrentItem ) return;
  
  QString id = mPhoneIdCombo->currentText();
  if ( id.isEmpty() ) return;
  
  PhoneNumber p;
  p.setId( id );
  mCurrentItem->addressee().removePhoneNumber( p );
  
  readAddressee( mCurrentItem->addressee() );
}


void MainWindow::newCategory()
{
  if ( !mCurrentItem ) return;
  
  bool ok = false;
  QString name = QInputDialog::getText( i18n("New Category"),
                                        i18n("Please enter category name."),
                                        QLineEdit::Normal, QString::null, &ok,
                                        this );
  if ( !ok || name.isEmpty() ) return;
  
  new QListViewItem( mCategoryListView, name );
  mCurrentItem->addressee().insertCategory( name );
}

void MainWindow::editCategory()
{
  if ( !mCurrentItem ) return;
  
  QListViewItem *item = mCategoryListView->selectedItem();
  if( !item ) return;

  QString oldName = item->text( 0 );

  bool ok = false;
  QString name = QInputDialog::getText( i18n("Edit Category"),
                                        i18n("Please enter new category name."),
                                        QLineEdit::Normal, oldName, &ok,
                                        this );
  if ( !ok || name.isEmpty() ) return;

  item->setText( 0, name );
  mCurrentItem->addressee().removeCategory( oldName );
  mCurrentItem->addressee().insertCategory( name );
}

void MainWindow::removeCategory()
{
  if ( !mCurrentItem ) return;
  
  QListViewItem *item = mCategoryListView->selectedItem();
  if( !item ) return;

  mCurrentItem->addressee().removeCategory( item->text( 0 ) );
  delete item;  
}


void MainWindow::editCustom()
{
  if ( !mCurrentItem ) return;
  
  QListViewItem *item = mCustomListView->selectedItem();
  if( !item ) return;

  QString oldName = item->text( 0 ) + "-" + item->text( 1 ) + ":" +
                    item->text( 2 );

  bool ok = false;
  QString name = QInputDialog::getText( i18n("New Custom Entry"),
                                        i18n("Please enter custom entry.\n"
                                             "Format: APP-NAME:VALUE"),
                                        QLineEdit::Normal, oldName, &ok,
                                        this );
  if ( !ok || name.isEmpty() ) return;

  int posDash = name.find( "-" );
  int posColon = name.find( ":" );
  
  QString app = name.left( posDash );
  QString cname = name.mid( posDash + 1, posColon - posDash - 1 );
  QString value = name.mid( posColon + 1 );
  
  item->setText( 0, app );
  item->setText( 1, cname );
  item->setText( 2, value );
  
  mCurrentItem->addressee().removeCustom( app, cname );
  mCurrentItem->addressee().insertCustom( app, cname, value );
}

void MainWindow::newCustom()
{
  if ( !mCurrentItem ) return;
  
  bool ok = false;
  QString name = QInputDialog::getText( i18n("New Custom Entry"),
                                        i18n("Please enter custom entry.\n"
                                             "Format: APP-NAME:VALUE"),
                                        QLineEdit::Normal, QString::null, &ok,
                                        this );
  if ( !ok || name.isEmpty() ) return;
  
  int posDash = name.find( "-" );
  int posColon = name.find( ":" );
  
  QString app = name.left( posDash );
  QString cname = name.mid( posDash + 1, posColon - posDash - 1 );
  QString value = name.mid( posColon + 1 );
  
  new QListViewItem( mCustomListView, app, cname, value );

  mCurrentItem->addressee().insertCustom( app, cname, value );
}

void MainWindow::removeCustom()
{
  if ( !mCurrentItem ) return;
  
  QListViewItem *item = mCustomListView->selectedItem();
  if( !item ) return;

  mCurrentItem->addressee().removeCustom( item->text( 0 ), item->text( 1 ) );
  delete item;
}

void MainWindow::addressBookChanged()
{
  QMessageBox::warning( this, i18n("Address Book Changed"),
                        i18n("The address book has changed on disk"),
                        i18n("Reload") );
  loadAddressBook( mAddressBook->fileName() );
}

void MainWindow::addressBookLocked()
{
  kdDebug() << "AddressBook locked()" << endl;
}

void MainWindow::addressBookUnlocked()
{
  kdDebug() << "AddressBook unlocked()" << endl;
}
