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
#include "StringListSelectAndReorderSet.h"
#include "kab.h"
#include <kprocess.h>
#include <qpainter.h>
#include <qclipboard.h>
#include <qpaintdevicemetrics.h> 
#include <qmessagebox.h>
#include "PrintDialog.h"
#include "debug.h"
#include "functions.h"
#include "businesscard.h"
#include <kfiledialog.h>
#include <kapp.h>


bool AddressBook::getEntry(const string& key, 
			   Section*& data)
{
  ID(bool GUARD=false);
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
  ID(bool GUARD=false);
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
//   const char* Fields[]= {
//     "name", "firstname", "additionalName",
//     "namePrefix", "fn", "comment", "org", 
//     "orgUnit", "orgSubUnit", "title", "role", 
//     "birthday", "talk", "deliveryLabel", "emails", 
//     "address", "town", "telephone", "fax", "modem", "URL" } ;
//   int NoOfFields=sizeof(Fields)/sizeof(Fields[0]);
  StringListSelectSetDialog dialog(this);
  int index;
  list<string> fields;
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
      nameOfField(*fpos, temp);
      CHECK(nameOfField(*fpos, temp));
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
	  if(*fpos=="emails")
	    {
	      if(emailAddress((*epos).second, text, false))
		{
		  data+=text;
		}
	      data+="\t";
	      continue;
	    }
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
  // ########################################################
}

void AddressWidget::exportHTML()
{
  ID(bool GUARD=true);
  LG(GUARD, "AddressWidget::exportHTML: called.\n");
  // ########################################################
  const string background=card->getBackground();;
  const string title=i18n("KDE addressbook overview");
  string header=
    (string)"<html>\n<head>\n"
    +(string)"<title>"+(string)title+(string)"</title>\n"
    +(string)"</head>\n"
    +(string)"<body background=\""+background+(string)"\">\n"
    +(string)"<h1>"+title+(string)"</h1>";
  string footer="</body>\n</html>";
  string logo;
  string kdelabel;
  string alignment="center";
  string body;
  string home;
  string file;
  string temp;
  QString dummy; // string objects crash on 0 pointers
  list<int> indizes;
  list<int>::iterator ipos;
  list<string> fields; // the fields in the table
  list<string>::iterator fieldPos;
  StringListSAndRSetDialog pDialog(this);
  Section* entry;
  StringStringMap::iterator pos;
  KeyValueMap* keys;
  int i;
  // ----- preparation:
  if(noOfEntries()==0)
    {
      emit(setStatus(i18n("No entries.")));
      qApp->beep();
      return;
    }
  // ----- create the table:
  body+=(string)"<"+alignment+(string)">"
    +(string)"<table border>\n";
  //       select what fields to add to the table:
  // fields.erase(fields.begin(), fields.end());
  for(i=0; i<NoOfFields; i++)
    {
      if(!nameOfField(Fields[i], temp))
	{
	  CHECK(false);
	}
      fields.push_back(temp.c_str());
    }
  fields.push_front(i18n("Name-Email-Link (recommended!)"));
  pDialog.selector()->setValues(fields);
  pDialog.setCaption(i18n("Select table columns"));
  if(pDialog.exec())
    {
      if(!pDialog.selector()->getSelection(indizes))
	{
	  emit(setStatus(i18n("Nothing to export.")));
	  qApp->beep();
	  return;
	}	
      fields.erase(fields.begin(), fields.end());
      CHECK(fields.size()==0);
      for(ipos=indizes.begin(); ipos!=indizes.end(); ipos++)
	{
	  if(*ipos==0) // the name-email-link
	    {
	      // WORK_TO_DO: query settings for name style here
	      fields.push_back("name-email-link");
	    } else {
	      CHECK(*ipos<=NoOfFields && *ipos>0);
	      // remove the leading pseudo-field
	      fields.push_back(Fields[*ipos-1]);
	    }
	}
    } else {
      emit(setStatus(i18n("Rejected.")));
      qApp->beep();
      return;
    }
  //       create table headers:
  body+="<tr>\n";
  for(fieldPos=fields.begin(); 
      fieldPos!=fields.end();
      fieldPos++)
    {
      if(*fieldPos=="name-email-link")
	{
	  temp=i18n("Email link");
	} else {
	  if(!nameOfField(*fieldPos, temp))
	    {
	      L("AddressWidget::exportHTML: "
		"could not get name for field %s.\n",
		(*fieldPos).c_str());
	      temp="(unknown field name)";
	    }
	}
      body+=(string)"<th>"+temp+"\n";
    }
  body+="</tr>\n";
  //       create table, linewise:
  for(pos=entries.begin(); pos!=entries.end(); pos++)
    {
      getEntry((*pos).second, entry); // the section
      CHECK(getEntry((*pos).second, entry));
      keys=entry->getKeys();
      CHECK(keys!=0);
      body+="<tr>\n";
      for(fieldPos=fields.begin(); 
	  fieldPos!=fields.end();
	  fieldPos++)
	{
	  if(*fieldPos=="birthday")
	    {
	      Entry dummy;
	      getEntry((*pos).second, dummy);
	      CHECK(getEntry((*pos).second, dummy));
	      if(dummy.birthday.isValid())
		{
		  temp=dummy.birthday.toString();
		} else {
		  temp="";
		}
	      //       insert a non-breaking space - mozilla 
	      //       displays this better (hint from Thomas 
	      //       Stinner <thomas@roedgen.pop-siegen.de>
	      if(temp.empty()) temp="&nbsp;";
	      body+=(string)"<td>"+temp+"\n";
	      continue;
	    }
	  if(*fieldPos=="name-email-link")
	    {
	      Entry dummy;
	      getEntry((*pos).second, dummy);
	      CHECK(getEntry((*pos).second, dummy));
	      string mail;
	      string name;
	      emailAddress((*pos).second, mail, false);
	      literalName((*pos).second, name);
	      if(mail.empty())
		{
		  body+=(string)"<td>"+name+(string)"\n";
		} else {
		  body+=(string)"<td> <a href=mailto:\""+
		    mail+(string)"\">"+name+(string)"</a>\n";
		}
	      continue;
	    }
	  if(!keys->get(*fieldPos, temp))
	    {
	      L("AddressWidget::exportHTML: "
		"could not get data for key %s.\n",
		(*fieldPos).c_str());
	      temp="";
	    }
	  if(temp.empty()) temp="&nbsp;";
	  body+=(string)"<td>"+temp+"\n";
	}
      body+="</tr>\n";
    }
  body+="</table>\n"
    +(string)"</"+alignment+(string)">";
  // ----- get a filename:
  if(!getHomeDirectory(home))
    {
      QMessageBox::information
	(this, i18n("Sorry"),
	 i18n("Could not find the users home directory."));
      emit(setStatus(i18n("Intern error!"))); 
      qApp->beep();
      return;
    }
  dummy=KFileDialog::getOpenFileName
    (home.c_str(), "*html", this);
  if(!dummy.isEmpty())
    {
      file=dummy;
      LG(GUARD, "AddressWidget::exportHTML: "
	 "filename is %s.\n", file.c_str());
    } else {
      emit(setStatus(i18n("Cancelled.")));
      qApp->beep();
      return;
    }
  // ----- create HTML file:
  ofstream stream(file.c_str());
  if(!stream.good())
    {
      QMessageBox::information
	(this, i18n("Error"),
	 i18n("Could not open the file to create the HTML "
	      "table."));
    }
  LG(GUARD, "AddressWidget::exportHTML: writing the file.\n");
  //        htmlizeString is n.i., but may already be called:
  if(htmlizeString(header, temp)) header=temp;
  if(htmlizeString(body, temp)) body=temp;
  if(htmlizeString(footer, temp)) footer=temp;
  stream << header << endl 
	 << body << endl 
	 << footer << endl;
  LG(GUARD, "AddressWidget::exportHTML: done.\n");
  // ########################################################
}
 
#include "widgets.moc"

