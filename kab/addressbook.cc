/* -*- C++ -*-
 * This file implements the database of addresses and 
 * configuration values.
 * 
 * the KDE addressbook.
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class,
 *             Nana for debugging
 * $Revision$
 */

#include "addressbook.h"
#include "debug.h"
#include <kapp.h>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
	   }

bool AddressBook::initialized=false;
const char* AddressBook::ConfigSection="Config";
const char* AddressBook::EntrySection="Entries";
const char* AddressBook::DBFileName="addressbook.database";
const char* AddressBook::Fields[]= {
    "name", "firstname", "emails",
    "telephone", "town", "address",
    "state", "zip",
    "additionalName", "namePrefix",
    "fn", "comment", "org", "orgUnit",
    "orgSubUnit", "title", "role",
    "fax", "modem", "URL", "birthday" };
const int AddressBook::NoOfFields=sizeof(Fields)/sizeof(Fields[0]);

/* Some defaults (e.g., all settings that are lists of parameters)
 * must be added in AddressBook::restoreDefaults(..)!
 */

const char* AddressBook::Defaults[]= { 
  "Version=0.9",
  "QueryOnDelete=true",
  "QueryOnChange=true",
  "SaveOnExit=true",
  "QueryOnSave=true",
  "CreateBackup=true",
  "MailCommand=/opt/kde/bin/kmail",
  "MailSelectAddress=true",
  "TalkCommand=/opt/kde/bin/ktalk",
  "Background=background_1.jpg"
};

AddressBook::AddressBook(bool readonly_)
  : ConfigDB()
{
  register bool GUARD; GUARD=true;
  REQUIRE(DBFileName!=0);
  // ############################################################################
  /* The constructor is written to be save. It does NOT 
   * assume a connection to a X display, that is why it sends
   * all of its output to stdout. This provides the 
   * possibility to use the program as a kind of address data 
   * server.
   */
  string filename;
  string dir;
  LG(GUARD, "AddressBook constructor: called%s.\n", 
     readonly_ ? " (readonly)" : "");
  // ----- check if directory exists:
  if(initialized)
    {
      LG(GUARD, "AddressBook constructor: class already initialized.\n");
    } else {
      LG(GUARD, "AddressBook constructor: initializing class.\n");
      initialized=true;
      LG(GUARD, "AddressBook constructor: done.\n");
    }
  LG(GUARD, "AddressBook constructor: checking user directory.\n");
  dir=KApplication::localkdedir();
  dir+=(string)"/share/apps/kab/";
  if(access(dir.c_str(), F_OK)!=0)
    {
      cerr << i18n
	("The directory where user specific data for the\n"
	 "addressbook application is stored does not exist.\n"
	 "The program will try to create\n         ")
	   << dir << endl
	   << i18n("and store all local files in this directory.") << endl;
      if(mkdir(dir.c_str(), 0755)!=0)
	{
	  cerr << i18n
	    ("The directory could not be created.\n"
	     "Probably you do not have used KDE before, so\n"
	     "you do not have a local \".kde\" directory\n"
	     "in your home directory. Run the KDE filemanager\n"
	     "kfm once to automatically create it.") << endl;
	  ::exit(-1);
	} else {
	  CHECK(access(dir.c_str(), X_OK | F_OK /* R_OK | W_OK | */ )==0);
	  cerr << i18n("The directory has been created.\n") << endl;
	}
    } else {
      LG(GUARD, "AddressBook constructor: local data dir found.\n");
    }
  // ----- is it locked ?
  filename=dir+(string)DBFileName;
  if(ConfigDB::IsLocked(filename)!=0)
    {
      LG(GUARD, "AddressBook constructor: the file is locked.\n");
      if(!ConfigDB::CheckLockFile(filename))
	{
	  if(::remove((filename+".lock").c_str())==0)
	    {
	      LG(GUARD, "AddressBook constructor: stale "
		 "lockfile, removed, read+write-mode.\n");
	    } else {
	      LG(GUARD, "AddressBook constructor: opening read only.\n");
	      readonly_=true;
	    }
	} else {
	  LG(GUARD, "AddressBook constructor: lockfile is valid.\n");
	  readonly_=true;
	}
    } else {
      LG(GUARD, "AddressBook constructor: the file is not locked.\n");
    }
  // ----- set filename -> readonly?
  LG(GUARD, "AddressBook constructor: setting database filename to\n"
     "            \"%s\".\n", filename.c_str());
  if(readonly_==false)
    {
      if(!setFileName(filename, true, readonly_))
	{
	  cerr << i18n("No addressbook database file, creating new one.")
	       << endl;
	  // -----
	  if(!createNew(filename))
	    { // ----- createNew is verbose enough:
	      ::exit(-1);
	    }
	} else {
	  // The program will by default create backup when the key 
	  //   "CreateBackup"
	  // is missing in config section:
	  bool backup=true; 
	  string backupfilename=filename+".backup";
	  LG(GUARD, "AddressBook constructor: creating backup "
	     "of database in file \"%s\".\n", backupfilename.c_str());
	  KeyValueMap* keys;
	  if(!load())
	    {
	      cerr << i18n
		("Tried to load database for creating backup, but failed") 
		   << endl;
	      ::exit(-1);
	    }
	  get(ConfigSection, keys);
	  // database must be valid:
	  CHECK(get(ConfigSection, keys)); 
	  keys->get("CreateBackup", backup);
	  CHECK(keys->get("CreateBackup", backup));
	  // ----- check for stale lockfile:
	  if(ConfigDB::IsLocked(backupfilename)!=0)
	    {
	      LG(GUARD, "AddressBook constructor: the backup file is locked.\n");
	      if(ConfigDB::CheckLockFile(backupfilename))
		{
		  backup=false;
		} else {
		  if(::remove((backupfilename+".lock").c_str())==0)
		    {
		      LG(GUARD, "AddressBook constructor: "
			 "stale lockfile, removed.\n");
		    } else {
		      LG(GUARD, 
			 "AddressBook constructor: could "
			 "not remove stale lockfile.\n");
		    }
		}
	    }
	  if(backup)
	    {
	      if(setFileName(backupfilename, false, false))
		{
		  CHECK(!isRO());
		  if(!save("backup file for addressbook database"))
		    {
		      cerr << i18n("Unable to save to backup file.") << endl;
		      ::exit(-1);
		    }
		  if(!setFileName(filename, true, readonly_))
		    {
		      cerr << i18n
			("Unable to reset filename after creating backup.")
			   << endl;
		      ::exit(-1);
		    }
		} else {
		  cerr << i18n("Unable to create backup file") << endl;
		}
	    }
	  clear();
	  LG(GUARD, "AddressBook constructor: backup created.\n");
	}
      /* not needed, as already done:
       * LG(GUARD, "AddressBook constructor: "
       *  "changing mode back to read-only.\n");
       * setFileName(filename, true, true);
       * CHECK(setFileName(filename, true, true));
       * LG(GUARD, "AddressBook constructor: opened database read-only.\n");
       */
    } else { // ----- open read-only
      if(setFileName(filename, true, true))
	{
	  LG(GUARD, "AddressBook constructor: opened database, file existed.\n");
	} else {
	  if(!createNew(filename))
	    {
	      LG(GUARD, "AddressBook constructor: no database existed, "
		 "error creating new one, giving up.\n");
	      ::exit(-1);
	    } else {
	      if(setFileName(filename, true, true))
		{
		  LG(GUARD, "AddressBook constructor: created new file.\n");
		} else { // ----- this is a program failure!
		  CHECK(false); 
		}
	    }
	}
      LG(GUARD, "AddressBook constructor: opened database read-only.\n");
    }
  // -----
  CHECK(isRO()==readonly);
  LG(GUARD, "AddressBook constructor: done.\n");
  // ############################################################################
  ENSURE(entries.empty());
}

void AddressBook::restoreDefaults()
{
  // ############################################################################
  CHECK(configSection()!=0);
  KeyValueMap* keys=configSection()->getKeys();
  unsigned int index;
  const unsigned int Size=(sizeof(Defaults)/sizeof(Defaults[0]));
  list<string> mailParams;
  list<string> talkParams;
  // -----
  CHECK(keys!=0);
  keys->clear();
  CHECK(keys->empty());
  for(index=0; index<Size; index++)
    {
      if(!keys->insertLine(Defaults[index], true))
	{
	  cerr << i18n("Unable to insert \"") << Defaults[index] 
	       << i18n("\" into configuration section (ignored).") << endl;
	  // this is a syntax error in the "Defaults" aggregate!
	  CHECK(false); 
	}
    }
  // ----- the lists cannot be inserted by constants because
  //       they are mangled during the insertion:
  mailParams.push_back("<person>"); // kmail configuration ...
  mailParams.push_back("-s");
  mailParams.push_back("<subject>");
  if(!keys->insert("MailParameters", mailParams, true))
    {
      CHECK(false);
    }
  talkParams.push_back("--autoexit");
  talkParams.push_back("<person>");
  if(!keys->insert("TalkParameters", talkParams, true))
    {
      CHECK(false);
    }
  // ----- the file format (a int value) is inserted here:
  if(!keys->insert("FileFormat", KAB_FILE_FORMAT, true))
    {
      CHECK(false);
    }
  // ----- store the version of kab that is in use:
  if(!keys->insert("Version", KAB_VERSION, true))
    {
      CHECK(false);
    }
  // ############################################################################
}

bool AddressBook::invariant()
{
  register bool GUARD; GUARD=false;
  LG(GUARD, "AddressBook::invariant: checking invariants for"
     " this AddressBook object.\n");
  // ############################################################################
  // do not call REQUIRE or ENSURE here!
  // (this would cause an infinite loop)
  // also be careful not to call functions that call
  // REQUIRE or ENSURE themselves!
  if(entrySection()!=0) // DB has already been loaded
    {
      if(entries.size()!=entrySection()->noOfSections()) return false; 
      if(entrySection()->noOfSections()!=noOfEntries()) return false;
    }
  return true;
  // ############################################################################
}
void AddressBook::changed()
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressBook::changed: Database has changed its contents.\n");
  // ############################################################################
  // You should not use REQUIRE here because - as the DB 
  // contents have changed - we cannot ensure the entries-map
  // to be consistent. It is the job of this function to 
  // create the consistency.
  updateEntriesMap();
  first();
  if(noOfEntries()==0) currentChanged();
  // ############################################################################
  LG(GUARD && (entrySection()==0 || noOfEntries()==0),
     "AddressBook::changed: Database is empty now.\n");
  ENSURE(1); // check if we have done all right
  LG(GUARD, "AddressBook::changed: done.\n");
}

void AddressBook::updateEntriesMap(string theOne)
{
  // no REQUIRE !! it is impossible to be true!
  register bool GUARD; GUARD=false;
  // ############################################################################
  LG(GUARD, "AddressBook::updateEntriesMap: updating mirror map.\n");
  Section::StringSectionMap::iterator pos;
  string key;
  Entry entry;
  Section* section=entrySection();
  // -----
  // entries.erase(); // ni in GNU C++ 2.7.2.1
  entries.erase(entries.begin(), entries.end());
  CHECK(entries.empty());
  if(section!=0) 
    {
      for(pos=section->sectionsBegin(); pos!=section->sectionsEnd(); pos++)
	{
	  //	  makeEntryFromSection(*(*pos).second, entry);
	  //	  CHECK(makeEntryFromSection(*(*pos).second, entry));
	  // configurable sorting orter has to attack here:
	  // sorting order depends on the constructed key
	  description((*pos).first, key, true);
	  CHECK(description((*pos).first, key, true));
	  // Problem: If there are two entries that are 
	  // exactly similar to each other, the key could
	  // be the same and could not be inserted into 
	  // the map because the keys are identifying.
	  // Solution:: Add the (identifying) DB entry key
	  // to the key for the mirror map (if a part of a
	  // key is identifying the whole key is).
	  key+=(*pos).first;
	  CHECK(!(*pos).first.empty());
	  entries.insert(StringStringMap::value_type(key, (*pos).first));
	}
      LG(GUARD, "AddressBook::updateEntriesMap: current-key (%s) is %s.\n", 
	 theOne.c_str(), theOne.empty() ? "empty" : "not empty");
      if(theOne.empty())
	{
	  current=entries.begin();
	  currentChanged();
	} else {
	  setCurrent(theOne);
	  CHECK(setCurrent(theOne));
	}
    } // else: DB is empty, entries should be too
  LG(GUARD, "AddressBook::updateEntriesMap: inserted %i elements in mirror map, "
     "DB has %i entries.\n", entries.size(), noOfEntries());
  // ############################################################################
  ENSURE(1); // check invariants
}

bool AddressBook::setCurrent(const string& key)
{ // set current entry using its key in the entrySection:
  // ############################################################################
  StringStringMap::iterator pos;
  // -----
  for(pos=entries.begin(); pos!=entries.end(); pos++)
    {
      if((*pos).second==key)
	{
	  current=pos;
	  currentChanged();
	  return true;
	}
    }
  CHECK(pos==entries.end());
  return false; // no such key
  // ############################################################################
}  

Section* AddressBook::configSection()
{
  REQUIRE(ConfigSection!=0);
  // ############################################################################
  Section* section;
  // -----
  if(!get(ConfigSection, section))
    {
      return 0;
    } else {
      return section;
    }
  // ############################################################################
}

Section* AddressBook::entrySection()
{
  // ############################################################################
  Section* section;
  // -----
  if(!get(EntrySection, section))
    {
      return 0;
    } else {
      return section;
    }
  // ############################################################################
}

bool AddressBook::makeEntryFromSection(Section& section, Entry& entry)
{
  register bool GUARD; GUARD=false;
  LG(GUARD, "AddressBook::makeEntryFromSection: parsing section.\n");
  // ############################################################################
  KeyValueMap* map;
  list<int> birthday;
  Entry dummy;
  // ----- clear up the old entry: (change on July 19 1998):
  entry=dummy; // dummy is always completely empty
  // ----- 
  map=section.getKeys();
  // the first 6 keys are required, something went wrong if 
  // these are not set:
  if(!map->get("name", entry.name)
     || !map->get("firstname", entry.firstname)
     || !map->get("email", entry.email)
     || !map->get("telephone", entry.telephone)
     || !map->get("town", entry.town)
     || !map->get("address", entry.address))
    {
      return false;
    } else { // the entry should be OK
      map->get("additionalName", entry.additionalName);
      map->get("namePrefix", entry.namePrefix);
      map->get("fn", entry.fn);
      map->get("comment", entry.comment);
      map->get("org", entry.org);
      map->get("orgUnit", entry.orgUnit);
      map->get("orgSubUnit", entry.orgSubUnit);
      map->get("title", entry.title);
      map->get("role", entry.role);
      map->get("deliveryLabel", entry.deliveryLabel);
      map->get("email2", entry.email2);
      map->get("email3", entry.email3);
      map->get("fax", entry.fax);
      map->get("modem", entry.modem);
      map->get("URL", entry.URL);
      map->get("zip", entry.zip);
      map->get("state", entry.state);
      map->get("country", entry.country);
      if(map->get("talk", entry.talk))
	{
	  LG(GUARD, "AddressBook::makeEntryFromSection: "
	     "found %stalk address(es) (%i).\n", 
	     entry.talk.empty() ? "no " : "",
	     entry.talk.size());
	}
      if(map->get("keywords", entry.keywords))
	{
	  LG(GUARD, "AddressBook::makeEntryFromSection: "
	     "found %skeyword(s) (%i).\n",
	     entry.keywords.empty() ? "no " : "",
	     entry.keywords.size());
	}
      if(map->get("emails", entry.emails))
	{
	  LG(GUARD, "AddressBook::makeEntryFromSection: "
	     "found %semail address(es) (%i).\n",
	     entry.emails.empty() ? "no " : "",
	     entry.emails.size());
	}
      if(map->get("birthday", birthday))
	{
	  LG(GUARD, "AddressBook::makeEntryFromSection: "
	     "read a birthday int list.\n");
	  if(birthday.size()==3) // year, month, day
	    {
	      int y, m, d;
	      d=birthday.back(); birthday.pop_back();
	      m=birthday.back(); birthday.pop_back();
	      y=birthday.back(); birthday.pop_back();
	      LG(GUARD, "AddressBook::makeEntryFromSection:  "
		 "birthday values are %i-%i-%i (YMD).\n", y, m, d);
	      if(entry.birthday.setYMD(y, m, d))
		{
		  LG(GUARD, "AddressBook::makeEntryFromSection: the date is "
		     "valid, set birthday to %i-%i-%i (YMD).\n", 
		     entry.birthday.year() , entry.birthday.month(), 
		     entry.birthday.day());
		} else {
		  LG(GUARD, "AddressBook::makeEntryFromSection: birthday is not "
		     "a valid date.\n");
		} 
	    } else {
	      LG(GUARD, "AddressBook::makeEntryFromSection: more or less than 3 "
		 "integers in birthday.\n");
	    }
	}
    }
  return true;
  // ############################################################################
}

bool AddressBook::currentEntry(Entry& entry)
{
  // ############################################################################
  Entry temp;
  // -----
  if(noOfEntries()>0)
    {
      Section::StringSectionMap::iterator pos;
      entrySection()->find((*current).second, pos);
      CHECK(entrySection()->find((*current).second, pos));
      makeEntryFromSection(*(*pos).second, entry); 
      // this should work all time (it never failed for me)
      CHECK(makeEntryFromSection(*(*pos).second, entry)); 
      return true;
    } else {
      return false;
    }
  // ############################################################################
}

bool AddressBook::first()
{
  REQUIRE(1);
  // ############################################################################
  StringStringMap::iterator pos;
  // -----
  if(noOfEntries()>0)
    {
      pos=entries.begin();
      if(pos!=current)
	{
	  current=pos;
	  currentChanged();
	} else {
	  return false;
	}
      return true;
    } else {
      return false;
    }
  // ############################################################################
}

bool AddressBook::previous()
{
  REQUIRE(1);
  // ############################################################################
  if(noOfEntries()>0 && current!=entries.begin())
    {
      --current;
      currentChanged();
      return true;
    } else {
      return false;
    }
  // ############################################################################
}

bool AddressBook::next()
{
  REQUIRE(1);
  // ############################################################################
  if(noOfEntries()>0 && current!=entries.end() && current!=--entries.end())
    {
      ++current;
      currentChanged();
      return true;
    } else {
      return false;
    }
  // ############################################################################
}

bool AddressBook::last()
{
  // ############################################################################
  StringStringMap::iterator pos;
  // -----
  if(noOfEntries()>0)
    {
      pos=entries.end();
      --pos;
      if(pos!=current) 
	{
	  current=pos;
	  currentChanged();
	} else {
	  return false;
	}
      return true;
    } else {
      return false;
    }
  // ############################################################################
}

void AddressBook::currentChanged()
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  LG(GUARD, "AddressBook::currentChanged: current entry changed.\n");
  // ############################################################################
}

unsigned int AddressBook::noOfEntries()
{
  // ############################################################################
  Section* section=entrySection();
  // -----
  if(section!=0) // database has been loaded
    {
      return section->noOfSections();
    } else {
      return 0;
    }
  // ############################################################################
}

bool AddressBook::add(const Entry& entry, string& key)
{
  register bool GUARD; GUARD=false;
  LG(!key.empty(), "AddressBook::add: reference <key> is not empty!.\n");
  // ############################################################################
  LG(GUARD, "AddressBook::add[entry]: adding entry.\n");
  list<int> birthday;
  // ----- prepare birthday list:
  if(entry.birthday.isValid())
    { // construct a list from the date:
      birthday.push_back(entry.birthday.year());
      birthday.push_back(entry.birthday.month());
      birthday.push_back(entry.birthday.day());
      LG(GUARD, "AddressBook::add[entry]: birthday is valid, using it.\n");
    }
  // ----- find new name for the new entry:
  string name=nextAvailEntryKey();
  string path=(string)EntrySection+(string)"/"+name;
  if(!createSection(path))
    { //       this may not happen:
     cerr << i18n("Unable to add a new entry with available key.") << endl;
      ::exit(-1);
    } else {
      KeyValueMap* map;
      if(!get(path, map))
	{ //       again: this may not happen:
	  cerr << i18n("Cannot find previously created section.") << endl;
	  ::exit(-1);
	}
      if(   !map->insert("name", entry.name)
	 || !map->insert("firstname", entry.firstname)
	 || !map->insert("additionalName", entry.additionalName)
	 || !map->insert("namePrefix", entry.namePrefix)
	 || !map->insert("fn", entry.fn)
	 || !map->insert("comment", entry.comment)
	 || !map->insert("org", entry.org)
	 || !map->insert("orgUnit", entry.orgUnit)
	 || !map->insert("orgSubUnit", entry.orgSubUnit)
	 || !map->insert("title", entry.title)
	 || !map->insert("role", entry.role)
	 || !map->insert("deliveryLabel", entry.deliveryLabel)
	 || !map->insert("email", entry.email)
	 || !map->insert("email2", entry.email2)
	 || !map->insert("email3", entry.email3)
	 || !map->insert("address", entry.address)
 	 || !map->insert("town", entry.town)
	 || !map->insert("telephone", entry.telephone)
	 || !map->insert("fax", entry.fax)
	 || !map->insert("modem", entry.modem)
	 || !map->insert("URL", entry.URL)
	 || !map->insert("birthday", birthday)
	 || !map->insert("talk", entry.talk)
	 || !map->insert("emails", entry.emails)
	 || !map->insert("keywords", entry.keywords)
	 || !map->insert("state", entry.state)
	 || !map->insert("zip", entry.zip))
	{ //       errors again may not happen:
	  cerr << i18n("Unable to create key-value-map for entry.") << endl;
	  ::exit(-1);
	}
    }
  // add the element to the mirror map (this also updates the current entry):
  updateEntriesMap(name); 
  key=name;
  return true;
  // ############################################################################
}

bool AddressBook::add(string& key)
{
  // ############################################################################
  Entry temp; // all parts are empty
  return add(temp, key);
  // ############################################################################
}

bool AddressBook::change(const Entry& contents)
{
  // ############################################################################
  return change(currentEntry(), contents);
  // ############################################################################
}

bool AddressBook::change(const string& key, const Entry& contents)
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressBook::change: changing entry.\n");
  // ############################################################################
  list<int> birthday;
  // -----
  if(readonly)
    {
      L("AddressBook::change: DB is readonly!\n");
      return false;
    }
  // -----
  if(contents.birthday.isValid())
    {
      birthday.push_back(contents.birthday.year());
      birthday.push_back(contents.birthday.month());
      birthday.push_back(contents.birthday.day());
      LG(GUARD, "AddressBook::change: birthday is valid, using it.\n");
    }      
  Section* section;
  if(!entrySection()->find(key, section))
    {
      LG(GUARD, "AddressBook::change: no such entry.\n");
      return false;
    }
  KeyValueMap* keys=section->getKeys();
  CHECK(keys!=0);
  if(!keys->insert("name", contents.name, true)
     || !keys->insert("firstname", contents.firstname, true)
     || !keys->insert("additionalName", contents.additionalName, true)
     || !keys->insert("namePrefix", contents.namePrefix, true)
     || !keys->insert("fn", contents.fn, true)
     || !keys->insert("comment", contents.comment, true)
     || !keys->insert("org", contents.org, true)
     || !keys->insert("orgUnit", contents.orgUnit, true)
     || !keys->insert("orgSubUnit", contents.orgSubUnit, true)
     || !keys->insert("title", contents.title, true)
     || !keys->insert("role", contents.role, true)
     || !keys->insert("deliveryLabel", contents.deliveryLabel, true)
     || !keys->insert("email", contents.email, true)
     || !keys->insert("email2", contents.email2, true)
     || !keys->insert("email3", contents.email3, true)
     || !keys->insert("address", contents.address, true)
     || !keys->insert("town", contents.town, true)
     || !keys->insert("telephone", contents.telephone, true)
     || !keys->insert("fax", contents.fax, true)
     || !keys->insert("modem", contents.modem, true)
     || !keys->insert("URL", contents.URL, true)
     || !keys->insert("comment", contents.comment, true)
     || !keys->insert("talk", contents.talk, true)
     || !keys->insert("emails", contents.emails, true)
     || !keys->insert("keywords", contents.keywords, true)
     || !keys->insert("zip", contents.zip, true)
     || !keys->insert("state", contents.state, true)
     || !keys->insert("country", contents.country, true)	
     || (birthday.empty() ? !keys->insert("birthday", "", true)
	 : !keys->insert("birthday", birthday, true)))
    {
      LG(GUARD, "AddressBook::change: failed to set values.\n");
      // kill the program -> may not happen in debug version!
      CHECK(0); 
      return false;
    } else {
      LG(GUARD, "AddressBook::change:  done, updating using current-key %s.\n",
	 currentEntry().c_str());
      updateEntriesMap((*current).second);
    }
  return true;
  // ############################################################################
} 

bool AddressBook::remove()
{
  // ############################################################################
  return remove(currentEntry());
  // ############################################################################
}

bool AddressBook::remove(const string& key)
{
  // ############################################################################
  Section::StringSectionMap::iterator pos;
  // -----
  if(!entrySection()->find(key, pos))
    { // ----- no such entry:
      return false;
    }
  if(entrySection()->remove((*pos).first))
    {
      updateEntriesMap();
      return true;
    } else {
      return false;
    } 
  // ############################################################################
}
   
string AddressBook::nextAvailEntryKey()
{ // should be improved 
  // (is O(n), but by now it re-uses deleted entry keys)
  // ############################################################################
  unsigned int key=0;
  char name[1024];
  KeyValueMap* dummy;
  // -----
  do {
    sprintf(name, "%s/%u", EntrySection, ++key);
  } while(get(name, dummy));
  sprintf(name, "%u", key);
  return name;
  // ############################################################################
}

bool AddressBook::setCurrent(int index)
{
  // ############################################################################
  if(entries.size()>(unsigned)index && index>=0)
    {
      current=entries.begin();
      advance(current, index);
      currentChanged();
      return true;
    } else {
      return false;
    }
  // ############################################################################
}

bool AddressBook::isFirstEntry()
{
  // ############################################################################
  if(noOfEntries()!=0 && current==entries.begin())	
    {
      return true;
    } else {
      return false;
    }
  // ############################################################################
}

bool AddressBook::isLastEntry()
{
  // ############################################################################
  StringStringMap::iterator pos=current;
  // -----
  if(noOfEntries()!=0)
    {
      // must not be if DB is not empty:
      CHECK(current!=entries.end()); 
      pos++;
      if(pos==entries.end())
	{
	  return true;
	} else {
	  return false;
	}
    } else {
      return false;
    }
  // ############################################################################
}

string AddressBook::getName(const string& key)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  string result;
  Entry entry;
  Section* section;
  const string path=EntrySection+(string)"/"+key;
  // -----
  LG(GUARD, "AddressBook::getName: searching for section \"%s\" (key is %s).\n", 
     path.c_str(), key.c_str());
  // this method may not be called with wrong keys:
  get(path, section);  CHECK(get(path, section));
  makeEntryFromSection(*section, entry);
  LG(GUARD, "AddressBook::getName: got section and converted it to an Entry "
     "object.\n");
  if(!entry.fn.empty())
    { // use the formatted name if it is defined:
      result=entry.fn;
    } else {
      // construct the name from firstname and name:
      result=entry.firstname;
      // add a space if needed:
      if(!result.empty()) result+=" ";
      // add name:
      result+=entry.name;
      // use email-address if this is empty:
      if(result.empty() && !entry.emails.empty())
	{
	  result=entry.emails.front();
	}
      // if this is still empty use telefon number in 
      // brackets:
      if(result.empty())
	{
	  if(!entry.telephone.empty())
	    {
	      result=(string)"("+entry.telephone+(string)")";
	    } else {
	      // complain if all this is still empty:
	      result=i18n("entry without name");
	    }
	}
    }
  LG(GUARD, "AddressBook::getName: result is %s.\n", result.c_str());
  return result;
  // ############################################################################
}

bool AddressBook::createNew(string filename)
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressBook::createNew: creating new database.\n");
  // ############################################################################
  KeyValueMap* section;
  // -----
  if(!setFileName(filename, false))
    {
      cerr << i18n("Unable to create new database file (probably permission "
		   "denied), exiting.") << endl;
      return false;
    } else {
      if(!createSection(ConfigSection))
	{
	  cerr << i18n("Cannot create configuration section, exiting.") << endl;
	  return false;
	}
      if(!createSection(EntrySection))
	{
	  cerr << i18n("Cannot create entries section, exiting.") << endl;
	  return false;
	}
      if(!get(ConfigSection, section))
	{
	  cerr << i18n("Unable to find previously created section, exiting.") 
	       << endl;
	  return false;
	}
      // ----- sets the initial state of the settings:
      restoreDefaults();
      if(!save())
	{
	  cerr << i18n("Cannot save newly created database.")
	       << endl;
	  return false;
	}
      if(!clear())
	{
	  cerr << i18n("Cannot erase database contents.") << endl;
	  return false;
	}
    }
  LG(GUARD, "AddressBook::createNew: done.\n");
  return true;
  // ############################################################################
}

string AddressBook::currentEntry()
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  if(noOfEntries()==0)
    {
      LG(GUARD, "AddressBook::currentEntry: no entries.\n");
      return "";
    }
  LG(GUARD, "AddressBook::currentEntry: current entry is %s.\n", 
     (*current).second.c_str());
  return (*current).second;
  // ############################################################################
}

bool AddressBook::getEntries(list<AddressBook::Entry>& entries)
{
  register bool GUARD; GUARD=false;
  REQUIRE(entries.empty());
  LG(GUARD, "AddressBook::getEntries: called.\n");
  // ############################################################################  
  Section::StringSectionMap::iterator pos; 
  Section* section=entrySection();
  Entry entry;
  bool error=false;
  // -----
  if(section==0)
    { // ----- database has been cleared completely before:
      LG(GUARD, "AddressBook::getEntries: no entries, empty database.\n");
      return false;
    }
  for(pos=section->sectionsBegin(); pos!=section->sectionsEnd(); pos++)
    { // ----- for all entries:
      if(makeEntryFromSection(*((*pos).second), entry))
	{
	  entries.push_back(entry);
	} else {
	  L("AddressBook::getEntries: error parsing entry %s.\n",
	    (*pos).first.c_str());
	  CHECK(false);
	  error=true;
	}
    }
  LG(GUARD, "AddressBook::getEntries: done, %i entries%s.\n",
     entries.size(), error ? ", errors occured" : "");
  CHECK(entries.size()==noOfEntries());
  return !error;
  // ############################################################################  
}

bool AddressBook::nameOfField(const string& field, string& name)
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressBook::nameOfField: called.\n");
  // ############################################################################  
  // this is only initialized once:
  static map<string, string, less<string> > *names=0;
  map<string, string, less<string> >::iterator pos;
  // ----- initialize the "names" map:
  if(names==0)
    { // ----- names will never get deleted during runtime
      LG(GUARD, "AddressBook::nameOfField: first call, need"
	 " to initialize name translations.\n");
      names=new map<string, string, less<string> >;
      if(!names->insert
	 (map<string, string, less<string> >::value_type
	  ("name", i18n("Name"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("firstname", i18n("First name"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("additionalName", i18n("Additional name"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("namePrefix", i18n("Name prefix"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("fn", i18n("Formatted name"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("comment", i18n("Comment"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("org", i18n("Organization"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("orgUnit", i18n("Unit"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("orgSubUnit", i18n("Subunit"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("title", i18n("Title"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("role", i18n("Role"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("birthday", i18n("Birthday"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("talk", i18n("Talk addresses"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("deliveryLabel", i18n("Delivery label"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("email", i18n("Email address"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("email2", i18n("Second email address"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("email3", i18n("Third email address"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("emails", i18n("Email addresses"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("address", i18n("Address"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("town", i18n("City/Town"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("telephone", i18n("Telephone"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("fax", i18n("Fax number"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("modem", i18n("Modem number"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("URL", i18n("Homepage URL"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("zip", i18n("Zip/postal code"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("state", i18n("State/province"))).second
	 || 
	 !names->insert
	 (map<string, string, less<string> >::value_type
	  ("country", i18n("Country"))).second
	 )
	{ // ----- all errors here are typos:
	  CHECK(false);
	}
      LG(GUARD, "AddressBook::nameOfField: translation done.\n");
    }
  // ----- now do the real transition:
  CHECK(names!=0);
  pos=names->find(field);
  if(pos==names->end())
    {
      LG(GUARD, "AddressBook::nameOfField: unknown field.\n");
      return false;
    } else {
      name=(*pos).second;
      LG(GUARD, "AddressBook::nameOfField: name for field %s is %s.\n", 
	 field.c_str(), name.c_str());
      return true;
    }
  // ############################################################################  
}

bool AddressBook::literalName(const string& key, string& text, bool reverse,
			      bool initials)
{
  register bool GUARD; GUARD=false;
  LG(GUARD, "AddressBook::literalName: called.\n");
  // ############################################################################  
  // ----- this method will return either (in this order)
  //       ° the formatted name, if it is set,
  //       ° a combination (see header) of the name fields
  //       for purposes where an entry should be displayed
  //       identified by its name.
  Entry entry;
  string name;
  string firstname, addname, nameprefix;
  // -----
  if(!getEntry(key, entry))
    {
      LG(GUARD, "AddressBook::literalName: no such entry.\n");
      return false;
    }
  // ----- is the formatted name set?
  if(!entry.fn.empty())
    {
      text=entry.fn;
      return true;
    }
  // ----- prepare text parts:
  if(initials)
    {
      const string WhiteSpaces=" \t\n";
      string::size_type pos;
      // -----
      pos=entry.firstname.find_first_not_of(WhiteSpaces);
      firstname=pos==string::npos 
	? (string)"" : entry.firstname.substr(pos, 1)+(string)".";
      pos=entry.namePrefix.find_first_not_of(WhiteSpaces);
      addname=pos==string::npos 
	? (string)"" : entry.additionalName.substr(pos, 1)+(string)".";
      nameprefix=entry.namePrefix;
    } else {
      firstname=entry.firstname;
      addname=entry.additionalName;
      nameprefix=entry.namePrefix;
    }
  // ----- assemble the string:
  if(reverse)
    { // name, firstname - add. name - name prefix
      name=entry.name;
      if(!name.empty())
	{
	  name+=", ";
	}
      if(!firstname.empty())
	{
	  name+=firstname;
	}
      if(!addname.empty())
	{
	  if(!name.empty())
	    {
	      name+=" ";
	    }
	  name+=addname;
	}
      if(!nameprefix.empty())
	{
	  if(!name.empty())
	    {
	      name+=" ";
	    }
	  name+=nameprefix;
	}      
    } else {
      // firstname - add. name - name prefix - name
      name=firstname;
      if(!addname.empty())
	{
	  if(!name.empty())
	    {
	      name+=" ";
	    }
	  name+=addname;
	}
      if(!nameprefix.empty())
	{
	  if(!name.empty())
	    {
	      name+=" ";
	    }
	  name+=nameprefix;
	}
      if(!entry.name.empty())
	{
	  if(!name.empty())
	    {
	      name+=" ";
	    }
	  name+=entry.name;
	}
    }
  text=name;
  LG(GUARD, "AddressBook::literalName: done, name is %s.\n", text.c_str());
  return true;
  // ############################################################################  
}

bool AddressBook::description(const string& key, string& text, bool reverse,
			      bool initials)
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressBook::description: called.\n");
  // ############################################################################  
  string temp;
  Entry entry;
  // -----
  if(!literalName(key, temp, reverse, initials))
    {
      LG(GUARD, "AddressBook::description: no such entry.\n");
      return false;
    }
  // -----
  if(!temp.empty())
    {
      text=temp;
      LG(GUARD, "AddressBook::description: done, description is %s.\n", 
	 text.c_str());
      return true;
    }
  getEntry(key, entry); CHECK(getEntry(key, entry));
  if(!entry.emails.empty())
    {
      text=entry.emails.front();
      LG(GUARD, "AddressBook::description: done, description"
	 " is %s (email address).\n", text.c_str());
      return true;
    }
  if(!entry.talk.empty())
    {
      text=entry.talk.front();
      LG(GUARD, "AddressBook::description: done, description"
	 " is %s (talk address).\n", text.c_str());
      return true;
    }
  // ----- give up:
  text=i18n("(unnamed entry)");
  LG(GUARD, "AddressBook::description: done, description is %s.\n", 
     text.c_str());
  return true;
  // ############################################################################  
}

bool AddressBook::birthDay(const string& key, QDate& date)
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "AddressBook::birthDay: called.\n");
  // ############################################################################  
  Entry entry;
  // -----
  if(!getEntry(key, entry))
    {
      LG(GUARD, "AddressBook::birthDay: no such entry.\n");
      return false;
    }
  if(!entry.birthday.isValid())
    {
      LG(GUARD, "AddressBook::birthDay: no birthday.\n");
      return false;
    } else {
      date=entry.birthday;
      LG(GUARD, "AddressBook::birthDay: done.\n");
      return true;
    }
  // ############################################################################  
}

bool AddressBook::getEntry(const string& key, Section*& data)
{
  register bool GUARD; GUARD=false;
  LG(GUARD, "AddressBook::getEntry[as a map]: called.\n");
  // ############################################################################
  Section* entries;
  Section* entry;
  // -----
  if(noOfEntries()==0)
    {
      LG(GUARD, "AddressBook::getEntry[as a map]: no entries.\n");
      return false;
    }
  entries=entrySection();
  CHECK(entries!=0);
  if(entries->find(key, entry))
    {
      LG(GUARD, "AddressBook::getEntry[as a map]: entry %s found.\n", 
	 key.c_str());
      data=entry;
      LG(GUARD, "AddressBook::getEntry[as a map]: done.\n");
      return true;
    } else {
      LG(GUARD, "AddressBook::getEntry[as a map]: no such entry.\n");
      return false;
    }
  // ############################################################################
}

bool AddressBook::getEntry(const string& key, Entry& ref)
{
  register bool GUARD; GUARD=false;
  LG(GUARD, "AddressBook::getEntry: called.\n");
  // ############################################################################
  Section* section;
  // -----
  if(!getEntry(key, section))
    {
      LG(GUARD, "AddressBook::getEntry: no such entry %s.\n", key.c_str());
      return false;
    } else {
      if(!makeEntryFromSection(*section, ref))
	{ // ----- may simply not happen:
	  CHECK(false);
	}
      LG(GUARD, "AddressBook::getEntry: done.\n");
      return true;
    }
  // ############################################################################
}

