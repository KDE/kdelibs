/* -*- C++ -*-
 * This file implements the dialog for editing an entry.
 * 
 * the KDE addressbook.
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class,
 *             Nana for debugging
 * $Revision$
 */

#include "StringListDialog.h"
#include "editentry.h"
#include "widgets.h"
#include "kab.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qkeycode.h>
#include <kapp.h>
#include "debug.h"

EditEntryDialog::EditEntryDialog(QWidget* parent, const char* name)
  : QTabDialog(parent, name, true)
{
  ID(bool GUARD=false);
  // ############################################################################
  LG(GUARD, "EditEntryDialog constructor: creating dialog.\n");
  setCancelButton();
  // ------
  connect(this, SIGNAL(applyButtonPressed()), SLOT(okPressed()));
  connect(this, SIGNAL(cancelButtonPressed()), SLOT(cancelPressed()));
  // ------
  createTabs();
  setCaption(i18n("Edit the current entry"));
  // ############################################################################
  LG(GUARD, "EditEntryDialog constructor: done.\n");
}

void EditEntryDialog::createTabs()
{
  ID(bool GUARD=true);
  // ############################################################################
  LG(GUARD, "EditEntryDialog::createTabs: creating tabbed dialog.\n");
  QLabel* label=0;
  QWidget *name, *address, *org, *contact, *others;
  int count, x, y;
  int widestTab=0;
  int highestTab=0;
  const int Border=5;
  // ------  
  // a tab for the basics: title, name, firstname, additional and formatted name
  LG(GUARD, "EditEntryDialog::createTabs: creating name tab.\n");
  {
    QGridLayout *layout;
    int widestString=0;
    const char* labels[]= { 
      i18n("Title:"), 
      i18n("First name:"), 	    
      i18n("Additional Name:"), 
      i18n("Name:"),
      i18n("Formatted Name:") };
    QLineEdit** ledits[]= {
      &leTitle,
      &leFirstName,
      &leAddName,
      &leName,
      &leFormattedName };
    const int Size=sizeof(labels)/sizeof(labels[0]);
    CHECK(Size==sizeof(ledits)/sizeof(ledits[0]));
    name=new QWidget(this);
    layout=new QGridLayout(name, Size, 2, Border);
    layout->setColStretch(0, 1);
    layout->setColStretch(1, 2);
    for(count=0; count<Size; count++)
      {
	label=new QLabel(labels[count], name);
	// find the needed space:
	y=label->fontMetrics().width(labels[count]);
	if(widestString<y) widestString=y;
	*ledits[count]=new QLineEdit(name);
	layout->addWidget(label, count, 0);
	layout->addWidget(*ledits[count], count, 1);
      }  
    addTab(name, i18n("&Name"));
    y=(Size+1)*Border+Size*((*ledits[0])->sizeHint().height());
    if(y>highestTab) highestTab=y;
    x=3*Border+3*widestString; // see column stretch factors
    if(x>widestTab) widestTab=x;
    layout->activate();
    name->setFixedHeight(y);
    LG(GUARD, "EditEntryDialog::createTabs: finished, %ix%i pixels.\n", x, y);
  }
  // end of first tab "&Name"
  // ------
  // a tab containing the different address fields
  {
    LG(GUARD, "EditEntryDialog::createTabs: creating address tab.\n");
    QGridLayout *layout;
    int widestString=0;
    const char* labels[]= { 
      i18n("Address:"),
      i18n("Zip/postal code:"),
      i18n("Town:"),
      i18n("State:"),
      i18n("Country:") };
    QLineEdit** ledits[]= {
      &leAddress,
      &leZip,
      &leTown,
      &leState,
      &leCountry };
    const int Size=sizeof(labels)/sizeof(labels[0]);
    CHECK(Size==sizeof(ledits)/sizeof(ledits[0]));
    address=new QWidget(this);
    layout=new QGridLayout(address, Size, 2, Border);
    layout->setColStretch(0, 1);
    layout->setColStretch(1, 2);
    for(count=0; count<Size; count++)
      {
	label=new QLabel(labels[count], address);
	// find the needed space:
	y=label->fontMetrics().width(labels[count]);
	if(widestString<y) widestString=y;
	*ledits[count]=new QLineEdit(address);
	layout->addWidget(label, count, 0);
	layout->addWidget(*ledits[count], count, 1);
      }      
    addTab(address, i18n("&Address"));
    y=(Size+1)*Border+Size*((*ledits[0])->sizeHint().height());
    if(y>highestTab) highestTab=y;
    x=3*Border+3*widestString; // see column stretch factors
    if(x>widestTab) widestTab=x;
    layout->activate();
    address->setFixedHeight(y);
    LG(GUARD, "EditEntryDialog::createTabs: finished, %ix%i pixels.\n", x, y);
  }
  // end of tab "&Address"
  // ------  
  // a tab for the organizational data
  LG(GUARD, "EditEntryDialog::createTabs: creating org tab.\n");
  {
    QGridLayout *layout;
    int widestString=0;
    const char* labels[]= {
      i18n("Organization:"),
      i18n("Org Unit:"),
      i18n("Org Subunit:"),
      i18n("Role:") };
    QLineEdit** ledits[]= {
      &leOrg,
      &leOrgUnit,
      &leOrgSubUnit,
      &leRole };
    const int Size=sizeof(labels)/sizeof(labels[0]);
    CHECK(Size==sizeof(ledits)/sizeof(ledits[0]));
    org=new QWidget(this);
    layout=new QGridLayout(org, Size, 2, Border);
    layout->setColStretch(0, 1);
    layout->setColStretch(1, 2);     
    for(count=0; count<Size; count++)
      {
	label=new QLabel(labels[count], org);
	y=label->fontMetrics().width(labels[count]);
	if(widestString<y) widestString=y;
	*ledits[count]=new QLineEdit(org);
	layout->addWidget(label, count, 0);
	layout->addWidget(*ledits[count], count, 1);
      }  
    addTab(org, i18n("&Organization"));
    y=(Size+1)*Border+Size*((*ledits[0])->sizeHint().height());
    if(y>highestTab) highestTab=y;
    x=3*Border+3*widestString;
    if(x>widestTab) widestTab=x;
    layout->activate();
    org->setFixedHeight(y);
    LG(GUARD, "EditEntryDialog::createTabs: finished, %ix%i pixels.\n", x, y);
  }
  // end of "org" tab
  // ------
  // a tab for email addresses, telefon numbers and URLs
  LG(GUARD, "EditEntryDialog::createTabs: creating contact tab.\n");
  {
    QGridLayout *layout;
    int widestString=0;
    const char* labels[]= {
      i18n("Telephone:"),
      i18n("Fax:"),
      i18n("Modem:"),
      i18n("Homepage (URL):") };
    QLineEdit** ledits[]= {
      &leTelephone, &leFax, &leModem, &leURL };
    const int Size=sizeof(labels)/sizeof(labels[0]);
    CHECK(Size==sizeof(ledits)/sizeof(ledits[0]));
    contact=new QWidget(this);
    layout=new QGridLayout(contact, Size+2, 2, Border);
    layout->setColStretch(0, 1);
    layout->setColStretch(1, 2);     
    //       add email addresses button:
    pbEmails=new QPushButton(contact);
    CHECK_PTR(pbEmails);
    connect(pbEmails, SIGNAL(clicked()),
	    SLOT(editEmailAddresses()));
    pbEmails->setText(i18n("Edit email addresses"));
    layout->addWidget(pbEmails, 0, 1);
    //       add talk addresses button:
    pbTalk=new QPushButton(contact);
    connect(pbTalk, SIGNAL(clicked()),
	    SLOT(editTalkAddresses()));
    pbTalk->setText(i18n("Edit talk addresses"));
    layout->addWidget(pbTalk, 1, 1);
    //       add other widgets:
    for(count=0; count<Size; count++)
      {
	label=new QLabel(labels[count], contact);
	y=label->fontMetrics().width(labels[count]);
	if(widestString<y) widestString=y;
	*ledits[count]=new QLineEdit(contact);
	layout->addWidget(label, count+2, 0);
	layout->addWidget(*ledits[count], count+2, 1);
      }
    addTab(contact, i18n("&Contact"));
    y=(Size+3)*Border+Size*((*ledits[0])->sizeHint().height())
      +pbEmails->sizeHint().height()+pbTalk->sizeHint().height();
    if(y>highestTab) highestTab=y;
    x=3*Border+3*widestString;
    if(x>widestTab) widestTab=x;
    layout->activate();
    contact->setFixedHeight(y);
    LG(GUARD, "EditEntryDialog::createTabs: finished, %ix%i pixels.\n", x, y);
  }
  // end of tab "&Contact"
  // ------
  // a tab for everything else:
 {
   int widestString=0;
   int x;
   const int noOfRows=2;
   QGridLayout *layout;
   others=new QWidget(this);
   layout=new QGridLayout(others, noOfRows, 2, Border);
   layout->setColStretch(0, 1);
   layout->setColStretch(1, 2);     

   label=new QLabel(i18n("Comment:"), others);
   mleComment=new QMultiLineEdit(others);
   layout->addWidget(label, 0, 0);
   layout->addWidget(mleComment, 0, 1);
   y=label->fontMetrics().width(label->text());
   if(y>widestString) widestString=y;
   
   label=new QLabel(i18n("Birthday:"), others);
   dlBirthDay=new DateLabel(others, 0, QDate());
   dlBirthDay->enableChangeDialog(true);

   layout->addWidget(label, 1, 0);
   layout->addWidget(dlBirthDay, 1, 1);   
   y=label->fontMetrics().width(label->text());
   if(y>widestString) widestString=y;
   layout->setRowStretch(0, 5);
   layout->setRowStretch(1, 1);
   addTab(others, i18n("O&thers"));
   layout->activate();
   // careful: leName must be created until here!
   y=7*leName->sizeHint().height()+8*Border;
   others->setFixedHeight(y);
   if(y>highestTab) highestTab=y;
   x=3*Border+3*widestString;
   if(x>widestTab) widestTab=x;
  }
  // ------
  // set the height and width of the tab dialog
  // name->setFixedWidth(widestTab);
  // address->setFixedWidth(widestTab);
  // org->setFixedWidth(widestTab);  
  // contact->setFixedWidth(widestTab);
  // others->setFixedWidth(widestTab);
  // ############################################################################
}

void EditEntryDialog::okPressed()
{
  // ############################################################################
  // these two arrays contain lineedits and corresponding string objects:
  string* entries[]= {
    &entry.title,
    &entry.firstname,
    &entry.additionalName,
    &entry.name,
    &entry.fn,
    &entry.address,
    &entry.zip,
    &entry.town,
    &entry.state,
    &entry.country,
    &entry.org,
    &entry.orgUnit,
    &entry.orgSubUnit,
    &entry.role,
    &entry.telephone,
    &entry.fax,
    &entry.modem,
    &entry.URL };
  QLineEdit* ledits[]= {
    leTitle,
    leFirstName,
    leAddName,
    leName,
    leFormattedName,
    leAddress,
    leZip,
    leTown,
    leState,
    leCountry,
    leOrg,
    leOrgUnit,
    leOrgSubUnit,
    leRole,
    leTelephone,
    leFax,
    leModem,
    leURL };
  const int Size=sizeof(entries)/sizeof(entries[0]);
  int count;
  // -----
  CHECK(Size==sizeof(ledits)/sizeof(ledits[0]));
  for(count=0; count<Size; count++)
    {
      *entries[count]=ledits[count]->text();
    }
  entry.comment=mleComment->text();
  entry.birthday=dlBirthDay->getDate();
  emit(accept());
  // ############################################################################
}

void EditEntryDialog::cancelPressed()
{
  // ############################################################################
  emit(reject());
  // ############################################################################
}

AddressBook::Entry EditEntryDialog::getEntry()
{
  // ############################################################################
  return entry;
  // ############################################################################
}

void EditEntryDialog::setEntry(const AddressBook::Entry& data)
{
  // ############################################################################
  entry=data;
  // these two arrays contain lineedits and corresponding string objects:
  const string* entries[]= {
    &data.title,
    &data.firstname,
    &data.additionalName,
    &data.name,
    &data.fn,
    &data.address,
    &data.town,
    &data.org,
    &data.orgUnit,
    &data.orgSubUnit,
    &data.role,
    &data.telephone,
    &data.fax,
    &data.modem,
    &data.URL,
    &data.zip,
    &data.state,
    &data.country };
  QLineEdit* ledits[]= {
    leTitle,
    leFirstName,
    leAddName,
    leName,
    leFormattedName,
    leAddress,
    leTown,
    leOrg,
    leOrgUnit,
    leOrgSubUnit,
    leRole,
    leTelephone,
    leFax,
    leModem,
    leURL,
    leZip,
    leState,
    leCountry };
  const int Size=sizeof(entries)/sizeof(entries[0]);
  int count;
  // -----
  CHECK(Size==sizeof(ledits)/sizeof(ledits[0]));
  for(count=0; count<Size; count++)
    {
      ledits[count]->setText((*entries[count]).c_str());
    } 
  mleComment->setText(data.comment.c_str());
  dlBirthDay->setDate(data.birthday);
  // ############################################################################
}

void EditEntryDialog::editTalkAddresses()
{
  ID(bool GUARD=false);
  LG(GUARD, "EditEntryDialog::editTalkAddresses: called.\n");
  // ############################################################################
  StringListDialog dialog(this);
  // -----
  dialog.setCaption(i18n("Edit talk addresses"));
  dialog.set(entry.talk);
  if(dialog.exec())
    {
      entry.talk=dialog.get();
    } else {
      qApp->beep();
    }
  // ############################################################################
}

void EditEntryDialog::editEmailAddresses()
{
  ID(bool GUARD=false);
  LG(GUARD, "EditEntryDialog::editEmailAddresses: called.\n");
  // ############################################################################
  StringListDialog dialog(this);
  // -----
  dialog.setCaption(i18n("Edit email addresses"));
  dialog.set(entry.emails);
  if(dialog.exec())
    {
      entry.emails=dialog.get();
    } else {
      qApp->beep();
    }
  // ############################################################################
}

// #############################################################################
// MOC OUTPUT FILES:
#include "editentry.moc"
// #############################################################################

