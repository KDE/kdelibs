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
#include <qclipboard.h>
#include <qpaintdevicemetrics.h> 
#include <qmessagebox.h>
#include "PrintDialog.h"
#include "debug.h"
#include "functions.h"
#include <kfiledialog.h>
#include <kapp.h>


bool AddressBook::getEntry(const string& key, 
			   Section*& data)
{
  ID(bool GUARD=true);
  LG(GUARD, "AddressBook::getEntry[as a map]: called.\n");
  // ########################################################
  Section* entries;
  Section* entry;
  // -----
  if(noOfEntries()==0)
    {
      LG(GUARD, "AddressBook::getEntry[as a map]: "
	 "no entries.\n");
      return false;
    }
  entries=entrySection();
  CHECK(entries!=0);
  if(entries->find(key, entry))
    {
      LG(GUARD, "AddressBook::getEntry[as a map]: "
	 "entry %s found.\n", key.c_str());
      data=entry;
      LG(GUARD, "AddressBook::getEntry[as a map]: done.\n");
      return true;
    } else {
      LG(GUARD, "AddressBook::getEntry[as a map]: "
	 "no such entry.\n");
      return false;
    }
  // ########################################################
}

bool AddressBook::getEntry(const string& key, Entry& ref)
{
  ID(bool GUARD=true);
  LG(GUARD, "AddressBook::getEntry: called.\n");
  // ########################################################
  Section* section;
  // -----
  if(!getEntry(key, section))
    {
      LG(GUARD, "AddressBook::getEntry: no such entry %s.\n",
	 key.c_str());
      return false;
    } else {
      if(!makeEntryFromSection(*section, ref))
	{ // ----- may simply not happen:
	  CHECK(false);
	}
      LG(GUARD, "AddressBook::getEntry: done.\n");
      return true;
    }
  // ########################################################
}

void AddressWidget::copy()
{
  // ########################################################
  //       an array containing the keys for all fields:
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
  list<string> selection;
  list<string>::iterator fpos; // selects a field
  list<int> indizes;
  list<int>::iterator ipos;
  Section* section;
  KeyValueMap* keys;
  StringStringMap::iterator epos; // selects an entry
  string data, text, temp;
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
  dialog.getSelection(indizes);
  if(indizes.size()==0)
    {
      emit(setStatus(i18n("Nothing selected.")));
      qApp->beep();
      return;
    } else { // put selected keys in fields:
      fields.erase(fields.begin(), fields.end());
      for(ipos=indizes.begin(); ipos!=indizes.end(); ipos++)
	{
	  CHECK(*ipos>-1 && *ipos<NoOfFields);
	  fields.push_back(Fields[*ipos]);
	}
      CHECK(fields.size()==indizes.size());
    }
  // ----- add the headers:
  for(fpos=fields.begin(); fpos!=fields.end(); fpos++)
    {
      temp="";
      if(!nameOfField(*fpos, temp))
	{
	  CHECK(false);
	}
      data+=temp+"\t";
    }
  data+="\n";
  // ----- add all entries:
  for(epos=entries.begin(); epos!=entries.end(); epos++)
    { //       for all entries in current sorting order do:
      // ----- get the entry as an key-value-map
      getEntry((*epos).second, section);
      CHECK(getEntry((*epos).second, section));
      keys=section->getKeys();
      for(fpos=fields.begin(); fpos!=fields.end(); fpos++)
	{ //       for all selected fields do:
	  if(keys->get(*fpos /* the field key */, text))
	    {
	      data+=text;
	    }
	  data+="\t";
	}
      data+="\n";
    }
  // ----- copy data to the clipboard:
  QApplication::clipboard()->setText(data.c_str());
  QMessageBox::information
    (this, i18n("kab"), 
     i18n("Sorry: copying is not implemented."));
  // ########################################################
}

#include "widgets.moc"

