/* -*- C++ -*-
 * This file contains stuff that is in development for 
 * faster compilation. Usually it is nearly empty.
 * 
 * the KDE addressbook.
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class,
 *             Nana for debugging
 * $Revision$
 */

#include "stl_headers.h"
#include "StringListSelectSetDialog.h"
#include "kab.h"
#include <kprocess.h>
#include <qpainter.h>
#include <qpdevmet.h> 
#include <qmsgbox.h>
#include "PrintDialog.h"
#include "debug.h"
#include "functions.h"
#include <kfiledialog.h>
#include <kapp.h>

void AddressWidget::copy()
{
  // ########################################################
  // an array containing the keys for all fields:
  const char* Fields[]= {
    "name", "firstname", "additionalName",
    "namePrefix", "fn", "comment", "org", 
    "orgUnit", "orgSubUnit", "title", "role", 
    "birthday", "talk", "deliveryLabel", "emails", 
    "address", "town", "telephone", "fax", "modem", "URL" } ;
  int NoOfFields=sizeof(Fields)/sizeof(Fields[0]);
  StringListSelectSetDialog dialog(this);
  int index;
  list<string> fields;
  list<int> indizes;
  // list<string>::iterator pos;
  string data;
  string text;
  // ----- select fields to copy:
  for(index=0; index<NoOfFields; index++)
    { // translate field keys into texts:
      if(!nameOfField(Fields[index], text))
	{
	  CHECK(false); // should not happen
	}
      fields.push_back(text);
    }
  dialog.setValues(fields);
  dialog.setCaption(i18n("kab: Select fields to copy"));
  if(!dialog.exec())
    {
      emit(setStatus(i18n("Rejected.")));
      qApp->beep();
      return;
    }
  dialog.getSelection(fields);
  if(fields.size()==0)
    {
      emit(setStatus(i18n("Nothing selected.")));
      qApp->beep();
      return;
    }
  QMessageBox::information
    (this, i18n("kab"), 
     i18n("Sorry: copying is not implemented."));
  // ----- add the headers:
  
  // ----- add all entries:
  // ########################################################
}

#include "widgets.moc"

