/* -*- C++ -*-
 * This file implements the application programming interface
 * for using kab's addressbook files within other programs.
 * Parse it with kdoc to get the API documentation.
 * 
 * the KDE addressbook.
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class,
 *             Nana for debugging
 * $Revision$
 */

#include "editentry.h"
#include <qpushbutton.h>
#include <qframe.h>
#include <qmessagebox.h>
#include "kabapi.h"
#include <debug.h>

KabAPI::KabAPI(QWidget* parent)
  : QDialog(parent, 0, true),
    widget(0)
{
  ID(bool GUARD=true);
  LG(GUARD, "KabAPI constructor: called.\n");
  // ############################################################################
  /* NOTE: the AddressWidget-object is NOT created here, the
   *       pointer to it is initially set to zero.
   */
  // ----- create subwidgets:
  buttonOK=new QPushButton(this);
  CHECK(buttonOK!=0);
  buttonOK->setText(i18n("OK"));
  buttonCancel=new QPushButton(this);
  CHECK(buttonCancel!=0);
  buttonCancel->setText(i18n("Cancel"));
  frameLine=new QFrame(this);
  CHECK(frameLine!=0);
  frameLine->setFrameStyle(QFrame::HLine | QFrame::Raised);
  // ----- create connections:
  connect(buttonOK, SIGNAL(clicked()), SLOT(accept()));
  connect(buttonCancel, SIGNAL(clicked()), SLOT(reject()));
  LG(GUARD, "KabAPI constructor: done.\n");
  // ############################################################################  
}

void KabAPI::initializeGeometry()
{
  REQUIRE(widget!=0);
  ID(bool GUARD=true);
  LG(GUARD, "KabAPI::initializeGeometry: called.\n");
  // ############################################################################
  const int Grid=3;
  const int ButtonWidth=QMAX(buttonOK->sizeHint().width(),
			     buttonCancel->sizeHint().width());
  const int ButtonHeight=buttonOK->sizeHint().height();
  int cx=widget->width();
  int cy=0;
  // ----- the addresswidget:
  widget->move(0, cy);
  cy+=widget->height()+Grid;
  // ----- the horizontal line:
  frameLine->setGeometry(Grid, cy, cx-2*Grid, Grid);
  cy+=2*Grid;
  // ----- the buttons:
  buttonOK->setGeometry(Grid, cy, ButtonWidth, ButtonHeight);
  buttonCancel->setGeometry(cx-Grid-ButtonWidth, cy, ButtonWidth, ButtonHeight);
  cy+=Grid+ButtonHeight;
  // ----- 
  setFixedSize(cx, cy);
  LG(GUARD, "KabAPI::initializeGeometry: done.\n");
  // ############################################################################  
}
  
KabAPI::ErrorCode KabAPI::init(bool readonly)
{
  // ############################################################################  
  widget=new AddressWidget(this, 0, readonly);
  if(widget!=0)
    {
      connect(widget, SIGNAL(sizeChanged()), SLOT(initializeGeometry()));
      initializeGeometry();
      return NoError;
    } else {
      return InternError;
    }
  // ############################################################################  
}

KabAPI::ErrorCode KabAPI::getEntry(AddressBook::Entry& entry_, string& key)
{
  REQUIRE(widget!=0);
  // ############################################################################  
  AddressBook::Entry entry;
  // -----
  if(widget->currentEntry(entry))
    {
      entry_=entry;
      key=widget->currentEntry();
      CHECK(!key.empty());
      return NoError;
    } else {
      return NoEntry;
    } 
  // ############################################################################  
}

KabAPI::ErrorCode KabAPI::add(const AddressBook::Entry& entry, string& key,
			      bool edit_)
{
  REQUIRE(widget!=0);
  ID(bool GUARD=true);
  // ############################################################################  
  string dummy;
  AddressBook::Entry changed=entry;
  // -----
  if(widget->isRO())
    {
      return PermDenied;
    }
  if(edit_)
    {
      if(edit(changed)!=NoError)
	{
	  LG(GUARD, "KabAPI::add: edit dialog rejected.\n");
	  return Rejected;
	}
    }
  if(!widget->AddressBook::add(changed, dummy))
    {
      QMessageBox::information
	(this, i18n("Sorry"), i18n("Your new entry could not be added."));
      return InternError;
    }       
  key=dummy;
  return NoError;
  // ############################################################################  
}

KabAPI::ErrorCode KabAPI::edit(AddressBook::Entry& entry)
{
  REQUIRE(widget!=0);
  ID(bool GUARD=true);
  LG(GUARD, "KabAPI::edit[foreign entry]: called.\n");
  // ############################################################################  
  EditEntryDialog dialog(this);
  // -----
  dialog.setEntry(entry);
  if(dialog.exec())
    {
      LG(GUARD, "KabAPI::add: dialog finished with accept().\n");
      entry=dialog.getEntry();
    } else {
      LG(GUARD, "KabAPI::add: dialog finished with reject().\n");
      return Rejected;
    }  
  LG(GUARD, "KabAPI::edit[foreign entry]: finished.\n");
  return NoError;
  // ############################################################################  
}

KabAPI::ErrorCode KabAPI::edit(const string& key)
{
  REQUIRE(widget!=0);
  ID(bool GUARD=true);
  LG(GUARD, "KabAPI::edit: called.\n");
  // ############################################################################  
  AddressBook::Entry entry;
  // -----
  if(widget->isRO())
    {
      LG(GUARD, "KabAPI::edit: database is readonly.\n");
      return PermDenied;
    }
  if(widget->getEntry(key, entry))
    {
      if(edit(entry)==NoError)
	{
	  if(!widget->change(key, entry))
	     {
	       L("KabAPI::edit: Changing the entry failed!");
	       CHECK(false);
	     }
	} else {
	  LG(GUARD, "KabAPI::edit: rejected.\n");
	  return Rejected;
	}
    } else {
      LG(GUARD, "KabAPI::edit: no such entry.\n");
      return NoEntry;
    }
  LG(GUARD, "KabAPI::edit: finished.\n");
  return NoError;
  // ############################################################################  
}

KabAPI::ErrorCode KabAPI::remove(const string& key)
{
  REQUIRE(widget!=0);
  ID(bool GUARD=true);
  LG(GUARD, "KabAPI::remove: called.\n");
  // ############################################################################  
  if(widget->isRO())
    {
      return PermDenied;
    }
  if(widget->AddressBook::remove(key))
    {
      return NoError;
    } else {
      return NoEntry;
    }
  // ############################################################################  
}

KabAPI::ErrorCode KabAPI::getEntryByName(const string&, 
					 list<AddressBook::Entry>&, const int)
{
  // ############################################################################  
  return NotImplemented;
  // ############################################################################  
}

KabAPI::ErrorCode KabAPI::getEntryByName(const AddressBook::Entry&,
					 list<AddressBook::Entry>&, const int)
{
  // ############################################################################  
  return NotImplemented;
  // ############################################################################  
}

KabAPI::ErrorCode KabAPI::getEntries(list<AddressBook::Entry>& entries)
{
  ID(bool GUARD=true);
  REQUIRE(entries.empty());
  REQUIRE(widget!=0);
  LG(GUARD, "KabAPI::getEntries: called.\n");
  // ############################################################################  
  if(widget->noOfEntries()==0)
    { // ----- database is valid, but empty:
      LG(GUARD, "KabAPI::getEntries: no entries.\n");
      return NoEntry;
    }
  if(!widget->getEntries(entries))
    {
      L("KabAPI::getEntries: intern error.\n");
      return InternError;
    } else {
      LG(GUARD, "KabAPI::getEntries: done.\n");
      return NoError;
    }
  // ############################################################################  
}

KabAPI::ErrorCode KabAPI::sendEmail(const string& address, const string& subject)
{
  REQUIRE(widget!=0);
  // ############################################################################
  if(widget->sendEmail(address, subject))
    {
      return NoError;
    } else {
      return InternError;
    }
  // ############################################################################
}

// #############################################################################
// MOC OUTPUT FILES:
#include "kabapi.moc"
// #############################################################################

