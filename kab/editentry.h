// -*- C++ -*-
#ifndef EDITENTRYDIALOG_H
#define EDITENTRYDIALOG_H

/* This file implements the dialog for editing an entry.
 * Declaration
 *
 * the Configuration Database library
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@hamburg.netsurf.de>
 *                          <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class, Qt
 *             NANA (for debugging)
 * $Revision$
 */

#include <qtabdialog.h>
#include <qlineedit.h>
#include <qmultilinedit.h> 
#include <qpushbutton.h>
#include "addressbook.h"
#include "datepickerdialog.h"

class EditEntryDialog : public QTabDialog
{
  Q_OBJECT
public:
  EditEntryDialog(QWidget* parent=0, const char* name=0);
  AddressBook::Entry getEntry();
  void setEntry(const AddressBook::Entry&);
protected:
  // the "name" tab
  QLineEdit *leTitle, *leFirstName, *leAddName, 
    *leName, *leRole, *leFormattedName;
  // the "address" tab
  QLineEdit *leAddress, *leZip, *leTown, 
    *leState, *leCountry;
  // the "organization" tab
  QLineEdit *leOrg, *leOrgUnit, *leOrgSubUnit;
  // the "contact" tab
  QLineEdit // *leEmail1, *leEmail2, *leEmail3,
    *leTelephone, *leFax, *leModem, *leURL;
  // the "others" tab
  QMultiLineEdit* mleComment;
  DateLabel* dlBirthDay;
  QPushButton* pbTalk;
  QPushButton* pbEmails;
  // ------
  AddressBook::Entry entry;
  // ------
  void createTabs();
public slots:
  void okPressed();
  void cancelPressed();
  void editTalkAddresses();
  void editEmailAddresses();
};

#endif // EDITENTRYDIALOG_H

