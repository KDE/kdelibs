/* -*- C++ -*-
   This file implements the basic personal information management class
   used in the KDE addressbook.

   the KDE addressbook

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2001, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org $
   $ License: GPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Id$	 
*/

#include "addressbook.h"
#include "qconfigDB.h"

#include <qfileinfo.h>
#include <qstringlist.h>

#include <kapplication.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

extern "C" {
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
}

// ----- some defines:
#ifdef KAB_KDEBUG_AREA
#undef KAB_KDEBUG_AREA
#endif

#define KAB_KDEBUG_AREA 800

#ifdef STD_USERFILENAME
#undef STD_USERFILENAME
#endif
#define STD_USERFILENAME "kab/addressbook.kab"

#ifdef STD_CONFIGFILENAME
#undef STD_CONFIGFILENAME
#endif
#define STD_CONFIGFILENAME "kab/kab.config"

#ifdef ENTRY_SECTION
#undef ENTRY_SECTION
#endif
#define ENTRY_SECTION "entries"

// the name of the file-local configuration section
#ifdef LOCAL_CONFIG_SECTION 
#undef LOCAL_CONFIG_SECTION
#endif
#define LOCAL_CONFIG_SECTION "config"

// the name of the subsection for each entry
#ifdef ADDRESS_SUBSECTION 
#undef ADDRESS_SUBSECTION
#endif
#define ADDRESS_SUBSECTION "addresses"

#ifdef KAB_TEMPLATEFILE
#undef KAB_TEMPLATEFILE
#endif
#define KAB_TEMPLATEFILE "kab/template.kab"

#ifdef KAB_CONFIGTEMPLATE
#undef KAB_CONFIGTEMPLATE
#endif
#define KAB_CONFIGTEMPLATE "kab/template.config"

#ifdef KAB_CATEGORY_KEY
#undef KAB_CATEGORY_KEY
#endif
#define KAB_CATEGORY_KEY "categories"

const char* AddressBook::Entry::Address::Fields[]= {
  "headline", "position", 
  "org", "orgunit", "orgsubunit",
  "deliverylabel", "address", "zip", "town", "country", "state" };
const int AddressBook::Entry::Address::NoOfFields
=sizeof(AddressBook::Entry::Address::Fields)
/sizeof(AddressBook::Entry::Address::Fields[0]);

const char* AddressBook::Entry::Fields[]= {
  "title", "rank", "fn", "nameprefix", "firstname", "middlename", "lastname",
  "birthday", "comment", "talk", "emails", "keywords", "telephone",
  "urls", "user1", "user2", "user3", "user4", "custom", "categories"  };
const int AddressBook::Entry::NoOfFields
=sizeof(AddressBook::Entry::Fields)/sizeof(AddressBook::Entry::Fields[0]);

struct QStringLess
  : public binary_function<const QString&, const QString&, bool>
{
  /** The function operator, inline. */
  bool operator()(const QString& x, const QString& y) const
  {
    return x < y; // make one Qt operator fit exactly
  }
};

// ----- the derived map class:
class StringKabKeyMap : public map<QString, KabKey, QStringLess>
{ /* Same as map, but a class for compilation reasons. This way we do not need
   * to include the QStringLess class into the addressbook header file. */
};

// ----- another derived map class:
class KeyNameMap : public map<const char*, const QString, less<const char*> >
{ // same thing
};

KeyNameMap* AddressBook::Entry::fields;
KeyNameMap* AddressBook::Entry::Address::fields;

bool
KabKey::operator == (const KabKey& key) const
{
  // ###########################################################################
  return key.getKey()==getKey();
  // ###########################################################################
}

void
KabKey::setKey(const QCString& text)
{
  // ###########################################################################
  key=text;
  // ###########################################################################
}

QCString
KabKey::getKey() const
{
  // ###########################################################################
  return key;
  // ###########################################################################
}



AddressBook::Entry::Address::Address()
{
}

bool AddressBook::Entry::Address::nameOfField(const char* key, QString& value)
{
  KeyNameMap::iterator pos;
  // -----
  if(fields==0)
    { // this is executed exactly one time per application instance,
      // as fields is static 
      int counter=0;
      fields=new KeyNameMap;
      Q_CHECK_PTR(fields);
      if(!fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("Headline"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	 (Fields[counter++], i18n("Position"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	 (Fields[counter++], i18n("Organisation"))).second 
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	 (Fields[counter++], i18n("Department"))).second 
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	 (Fields[counter++], i18n("Sub-Department"))).second 
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	 (Fields[counter++], i18n("Delivery Label"))).second 
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	 (Fields[counter++], i18n("Address"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("Zipcode"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("City"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("Country"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("As in addresses", "State"))).second)
	{
	  kdDebug(KAB_KDEBUG_AREA) 
	    << "AddressBook::Entry::Address::nameOfField (while "
	    << " creating field-name map): TYPO, correct this." 
	    << endl;
	} else {
	  kdDebug(KAB_KDEBUG_AREA) 
	    << "AddressBook::Entry::Address::nameOfField: "
	    << "inserted field names." << endl;
	}
#if ! defined NDEBUG
      QString name;
      kdDebug(KAB_KDEBUG_AREA) 
	<< "AddressBook::Entry::Address::nameOfField:" << endl
	<< "Created key-fieldname-map. Defined fields are:" 
	<< endl; 
      for(counter=0; counter<AddressBook::Entry::Address::NoOfFields;
	  ++counter)
	{
	  pos=fields->find(Fields[counter]);
	  if(pos==fields->end())
	    {
	      kdDebug(KAB_KDEBUG_AREA) << "  UNDEFINED" << endl;
	    } else {
	      kdDebug(KAB_KDEBUG_AREA) 
		<< "  " << Fields[counter] << " ("
		<< (*pos).second.utf8() << ")" << endl;
	    }
	}
#endif
    }
  // ----- now finally do the lookup:
  pos=fields->find(key);
  if(pos==fields->end())
    {
      return false;
    } else {
      value=(*pos).second;
      return true;
    }
}

bool AddressBook::Entry::nameOfField(const char* key, QString& value)
{
  KeyNameMap::iterator pos;
  // -----
  if(fields==0)
    { // this is executed exactly one time per application instance,
      // as fields is static 
      int counter=0;
      fields=new KeyNameMap;
      Q_CHECK_PTR(fields);
      if(!fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("Title"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	 (Fields[counter++], i18n("Rank"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	 (Fields[counter++], i18n("Formatted Name"))).second 
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	 (Fields[counter++], i18n("Name Prefix"))).second 
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	 (Fields[counter++], i18n("First Name"))).second 
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	 (Fields[counter++], i18n("Middle Name"))).second 
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	 (Fields[counter++], i18n("Last Name"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("Birthday"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("Comment"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("Talk Addresses"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("Email Addresses"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("Keywords"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("Telephone Number"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("URLs"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("User Field 1"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("User Field 2"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("User Field 3"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("User Field 4"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("Custom"))).second
	 ||
	 !fields->insert
	 (map<const char*, const QString, less<const char*> >::value_type
	  (Fields[counter++], i18n("Categories"))).second)
	{
	  kdDebug(KAB_KDEBUG_AREA) 
	    << "AddressBook::Entry::Address::nameOfField (while "
	    << " creating field-name map): TYPO, correct this." << endl;
	} else {
	  kdDebug(KAB_KDEBUG_AREA) 
	    << "AddressBook::Entry::Address::nameOfField: "
	    << "inserted field names." << endl;
	}
#if ! defined NDEBUG
      QString name;
      kdDebug(KAB_KDEBUG_AREA) 
	<< "AddressBook::Entry::nameOfField:" << endl
	<< "Created key-fieldname-map. Defined fields are:" << endl; 
      for(counter=0; counter<AddressBook::Entry::Address::NoOfFields;
	  ++counter)
	{
	  pos=fields->find(Fields[counter]);
	  if(pos==fields->end())
	    {
	      kdDebug(KAB_KDEBUG_AREA) << "  UNDEFINED" << endl;
	    } else {
	      kdDebug(KAB_KDEBUG_AREA) 
		<< "  " << Fields[counter] << " ("
		<< (*pos).second.utf8() << ")" << endl;
	    }
	}
#endif
    }
  // ----- now finally do the lookup:
  pos=fields->find(key);
  if(pos==fields->end())
    {
      return false;
    } else {
      value=(*pos).second;
      return true;
    }
}
      
AddressBook::ErrorCode
AddressBook::Entry::getAddress(int index, Address& address) const
{
  // ###########################################################################
  list<Address>::const_iterator pos;
  // -----
  if(index>=0 && (unsigned)index<addresses.size())
    {
      pos=addresses.begin();
      advance(pos, index);
      address=*pos;
      return AddressBook::NoError;
    } else {
      return AddressBook::OutOfRange;
    }
  // ###########################################################################
}

int AddressBook::Entry::noOfAddresses() const
{
  return addresses.size();
}

AddressBook::AddressBook(QWidget* parent, const char* name, bool loadit)
  : QFrame(parent, name),
    config(new QConfigDB(this)),
    data(new QConfigDB(this)),
    entries(new StringKabKeyMap),
    state(NoFile)
{
  register bool GUARD; GUARD=true;
  // ###########################################################################
  QString dir, filename;
  bool createBackup=true;
  KeyValueMap *keys;
  // ----- do memory checks (do not rely on exception handling):
  if(config==0 || data==0 || entries==0)
    {
      KMessageBox::error(this,
	 i18n("Cannot initialize local variables."),
	 i18n("Out of Memory"));
      kapp->quit(); // It is critical, but will possibly never happen.
    }
  connect(data, SIGNAL(fileChanged()), SLOT(dataFileChanged()));
  connect(data, SIGNAL(changed(QConfigDB*)),
	  SLOT(reloaded(QConfigDB*)));
  connect(config, SIGNAL(fileChanged()), SLOT(configFileChanged()));
  // ----- set style:

  filename = locate( "data", STD_CONFIGFILENAME);
  if (filename.isEmpty())
  {
     filename = locateLocal( "data", STD_CONFIGFILENAME );
     // config does not exist yet
      if(createConfigFile()!=NoError)
	{
	  KMessageBox::sorry(this,
	     i18n("Your local kab configuration file "
		  "\"%1\" "
		  "could not be created. kab will probably not "
		  "work correctly without it.\n"
		  "Make sure you have not removed write permission "
		  "from your local KDE directory (Usually ~/.kde).").arg(filename));
	  state=PermDenied;
	}
  }
  loadConfigFile();
  // ----- now get some configuration settings:
  if(config->get("config", keys))
    {
      keys->get("CreateBackupOnStartup", createBackup);
    }
  // ----- check and possibly create user standard file:
  filename = locate( "data", STD_USERFILENAME );

  if(filename.isEmpty()) // if it does not exist
    {
      filename = locateLocal( "data", STD_USERFILENAME);
      if(createNew(filename)!=NoError) // ...and we cannot create it
	{
	  KMessageBox::sorry(this,
	     i18n("Your standard kab database file "
		  "\"%1\" "
		  "could not be created. kab will probably not "
		  "work correctly without it.\n"
		  "Make sure you have not removed write permission "
		  "from your local KDE directory (Usually ~/.kde).").arg(filename));
	  state=PermDenied;
	} else {
	  KMessageBox::information
	    (this,
	     i18n("kab has created your standard addressbook in\n\"%1\"")
	     .arg(filename));
	}
    }
  // ----- load the user standard file:
  if(loadit)
    {
      if(load(filename)!=NoError)
	{ // ----- the standard file could not be loaded
	  state=PermDenied;
	} else {
	  if(createBackup)
	    {
	      // ----- create the backup file:
	      QString temp=data->fileName();
	      if(data->setFileName(temp+".backup", false, false))
		{
		  if(!data->save())
		    {
		      KMessageBox::information
			(this,
			 i18n("Cannot create backup file (permission denied)."),
			 i18n("File Error"));
		    }
		} else {
		  KMessageBox::error
		    (this,
		     i18n("Cannot open backup file for "
			  "writing (permission denied)."),
		     i18n("File Error"));
		}
	      // ----- reset the filename:
	      if(!data->setFileName(temp, true, true))
		{
		  KMessageBox::error
		    (this,
		     i18n("Critical error:\n"
			  "Permissions changed in local directory!"),
		     i18n("File Error"));
		  closeFile(false);
		  state=PermDenied;
		} else {
		  state=NoError;
		}
	    }
	}
    }
  // -----
  data->watch(true);
  // ###########################################################################
}

AddressBook::~AddressBook()
{
  // ###########################################################################
  delete data;
  delete config;
  delete entries;
  // ###########################################################################
}

QConfigDB* AddressBook::getConfig()
{
  // ###########################################################################
  return config;
  // ###########################################################################
}

AddressBook::ErrorCode AddressBook::getState()
{
  // ###########################################################################
  return state;
  // ###########################################################################
}

AddressBook::ErrorCode AddressBook::load(const QString& filename)
{
  // ----- Remark: Close the file if it could not be loaded!
  // ###########################################################################
  ErrorCode rc=NoError;
  QFileInfo newfile, oldfile;
  // -----
  QString fname = (filename.isEmpty()) ? data->fileName() : filename ;
  if(fname.isEmpty()) // there was never a filename set:
    {
      state=NoFile;
      return NoFile;
    }
  // -----
  newfile.setFile(fname);
  oldfile.setFile(data->fileName());
  if(isSameFile(fname, data->fileName()))
    { // ----- possibly deleted file:
      if(data->load())
	{
	  emit(setStatus(i18n("File reloaded.")));
	  state=NoError;
	} else {
	  switch
	    (KMessageBox::questionYesNo
	     (this,
	      i18n("The currently loaded file "
                   "\"%1\" "
	           "cannot be reloaded. kab may close or save it.\n"
	           "Save it if you accidentally deleted your data file.\n"
	           "Close it if you intended to do so.\n"
	           "Your file will be closed by default.")
		 .arg(oldfile.absFilePath()),
	      i18n("File Error"),
	      i18n("&Close"), i18n("&Save")))
	    {
	    case 1: // save
	      if(!data->save(i18n("(Safety copy on file error)").ascii(), true))
		{
		  KMessageBox::information(this,
		     i18n("Cannot save the file; will close it now."),
		     i18n("File Error"));
		  closeFile(false);
		  state=NoFile;
		  rc=PermDenied;
		} else {
		  state=NoError;
		  rc=NoError;
		}
	      break; // no error if we could save the file
	    default: // close
	      closeFile(false);
	      state=NoFile;
	      rc=NoSuchFile;
	      break;
	    }
	}
    } else { // ----- set new filename
      if(data->setFileName(fname, true, true))
	{
	  if(data->load())
	    {
	      emit(changed());
	      emit(setStatus(i18n("File opened.")));
	      state=NoError;
	    } else {
	      KMessageBox::information(this,
		 i18n("Could not load the file."),
		 i18n("File Error"));
	      closeFile(false);
	      emit(setStatus(i18n("No such file.")));
	      rc=NoSuchFile;
	    }
	} else {
	  if(KMessageBox::questionYesNo
	     (this,
	      i18n("The file \"%1\" cannot be found. "
	           "Create a new one?").arg(fname), 
	      i18n("No Such File"),
	      i18n("OK"), i18n("Cancel"))==0)
	    {
	      if(createNew(fname)==NoError)
		{
		  emit(setStatus(i18n("New file.")));
		} else { // ----- do not close here, stick with the old file:
		  emit(setStatus(i18n("Cancelled.")));
		}
	    }
	}
    }
  // -----
  if(rc==NoError)
    {
      data->watch(true);
      updateMirrorMap();
    }
  // -----
  return rc;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::getListOfNames(QStringList* strings, bool reverse, bool initials)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) << "AddressBook::getListOfNames: called.\n";
  StringKabKeyMap::iterator pos;
  QString desc;
  ErrorCode rc=NoError;
  ErrorCode temp;
  // ----- erase the list contents:
  strings->clear();
  // ----- ...and fill it:
  for(pos=entries->begin(); pos!=entries->end(); ++pos)
    {
      temp=literalName((*pos).second, desc, reverse, initials);
      if(temp!=AddressBook::NoError)
	{
	  desc=i18n("(Internal error in kab)");
	  rc=InternError;
	}
      if(desc.isEmpty())
	{
	  desc=i18n("(empty entry)");
	}
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	  "AddressBook::getListOfNames: adding " << desc << endl;
      strings->append(desc);
    }
  // ----- any problems?
  kdDebug(GUARD, KAB_KDEBUG_AREA)
      << "AddressBook::getListOfNames: done, "
      << strings->count()
      << " entries.\n";
  return rc;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::literalName(const KabKey& key, QString& text, bool rev, bool init)
{
  // ###########################################################################
  Entry entry;
  ErrorCode rc;
  // ----- get the entry:
  rc=getEntry(key, entry);
  if(rc!=NoError)
    {
      return rc;
    }
  // -----
  return literalName(entry, text, rev, init);
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::literalName(const Entry& entry, QString& text, bool rev, bool init)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) << "AddressBook::literalName: called.\n";
  QString firstname, middlename, lastname, nameprefix;
  // ----- is the formatted name set?
  if(!entry.fn.isEmpty())
    {
      text=entry.fn;
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "AddressBook::literalName: done (fn).\n";
      return NoError;
    }
  // ----- prepare the strings:
  firstname=entry.firstname.simplifyWhiteSpace();
  middlename=entry.middlename.simplifyWhiteSpace();
  lastname=entry.lastname.simplifyWhiteSpace();
  nameprefix=entry.nameprefix.simplifyWhiteSpace();
  // ----- create the initials:
  if(init)
    {
      if(!firstname.isEmpty()) firstname=firstname.mid(0, 1)+'.';
      if(!middlename.isEmpty()) middlename=middlename.mid(0, 1)+'.';
      // if(!lastname.isEmpty()) lastname=lastname.mid(0, 1)+'.';
    }
  // ----- assemble the string:
  text="";
  if(rev)
    { // name, firstname - add. name - name prefix
      if(!lastname.isEmpty())
	{
	  text=lastname;
	}
      if(!firstname.isEmpty() || !middlename.isEmpty() || !nameprefix.isEmpty())
	{
	  text+=',';
	}
      if(!firstname.isEmpty())
	{
	  if(!text.isEmpty())
	    {
	      text+=' ';
	    }
	  text+=firstname;
	}
      if(!middlename.isEmpty())
	{
	  if(!text.isEmpty())
	    {
	      text+=' ';
	    }
	  text+=middlename;
	}
      if(!nameprefix.isEmpty())
	{
	  if(!text.isEmpty())
	    {
	      text+=' ';
	    }
	  text+=nameprefix;
	}
    } else {
      // firstname - add. name - name prefix - name
      text=firstname;
      if(!middlename.isEmpty())
	{
	  if(!text.isEmpty())
	    {
	      text+=' ';
	    }
	  text+=middlename;
	}
      if(!nameprefix.isEmpty())
	{
	  if(!text.isEmpty())
	    {
	      text+=' ';
	    }
	  text+=nameprefix;
	}
      if(!lastname.isEmpty())
	{
	  if(!text.isEmpty())
	    {
	      text+=' ';
	    }
	  text+=lastname;
	}
    }
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA) << "AddressBook::literalName: done: "
				  << text << ".\n";
  return NoError;
  // ###########################################################################
}

unsigned int
AddressBook::noOfEntries()
{
  // ###########################################################################
  return entries->size();
  // ###########################################################################
}

void
AddressBook::dataFileChanged()
{
  // ###########################################################################
  data->watch(false); // will be restarted after successful load
  load();
  // ###########################################################################
}

void
AddressBook::configFileChanged()
{
  register bool GUARD; GUARD=true;
  // ###########################################################################
  if(!config->load())
    {
      KMessageBox::error(this,
	 i18n("Cannot reload configuration file!"),
	 i18n("File Error"));
    } else {
	kdDebug(GUARD, KAB_KDEBUG_AREA) << "AddressBook::configFileChanged: "
	    "config file reloaded.\n";
      emit(setStatus(i18n("Configuration file reloaded.")));
    }
  // ###########################################################################
}

void
AddressBook::reloaded(QConfigDB* db)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  if(db==data)
      {
	  kdDebug(GUARD, KAB_KDEBUG_AREA) << "AddressBook::reloaded: file has been "
	      "reloaded.\n";
      updateMirrorMap(); // WORK_TO_DO: what's up with the return value?
      changed();
    }
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::save(const QString& filename, bool force)
{
  // ###########################################################################
  if(filename.isEmpty())
    {
      if(data->save(0, force))
	{
	  emit(setStatus(i18n("File saved.")));
	  return NoError;
	} else {
	  return PermDenied;
	}
    } else {
      if(data->setFileName(filename, false, false))
	{
	  if(data->save(0, true))
	    {
	      emit(newFile(filename));
	      return NoError;
	    } else {
	      return PermDenied;
	    }
	} else {
	  return PermDenied;
	}
    }
  // ###########################################################################
}

bool
AddressBook::isSameFile(const QString& a, const QString& b)
{
  // ###########################################################################
  QFileInfo filea(a), fileb(b);
  // -----
  return filea.absFilePath()==fileb.absFilePath();
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::closeFile(bool saveit)
{
  // ###########################################################################
  if(saveit)
    {
      if(save()!=NoError)
	{
	  emit(setStatus(i18n("Permission denied.")));
	  return PermDenied;
	}
    }
  data->clear();
  // data->reset(); WORK_TO_DO: File name is not reset by now.
  emit(setStatus(i18n("File closed.")));
  return NoError;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::getEntry(const KabKey& key, Entry& entry)
{
  // ###########################################################################
  Section *section;
  // -----
  if(getEntry(key, section)==NoError)
    {
      return makeEntryFromSection(section, entry);
    } else {
      return NoSuchEntry;
    }
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::getEntry(const KabKey& key, Section*& section)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) << "AddressBook::getEntry: searching entry "
      "with key " << key.getKey().data() << endl;
  StringKabKeyMap::iterator pos;
  // -----
  for(pos=entries->begin(); pos!=entries->end(); ++pos)
    {
      if((*pos).second==key)
	{
	    kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		"AddressBook::getEntry: key exists." << endl;
	  break;
	}
    }
  if(pos==entries->end())
    {
	kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	    "AddressBook::getEntry: no such entry.\n";
      return NoSuchEntry;
    } else {
      if(data->get((QCString)ENTRY_SECTION+'/'+key.getKey(), section))
	{
	    kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		"AddressBook::getEntry: done." << endl;
	  return NoError;
	} else {
	  return InternError;
	}
    }
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::getEntries(list<Entry>& thelist)
{
  // ###########################################################################
  StringKabKeyMap::iterator pos;
  Entry entry;
  ErrorCode rc;
  // -----
  kdDebug(!thelist.empty(), KAB_KDEBUG_AREA)
    << "AddressBook::getEntries: warning - non-empty value list!" << endl;
  thelist.erase(thelist.begin(), thelist.end());
  for(pos=entries->begin(); pos!=entries->end(); ++pos)
    {
      rc=getEntry((*pos).second, entry);
      if(rc==NoError)
	{
	  thelist.push_back(entry);
	} else {
	  return InternError;
	}
    }
  // -----
  return NoError;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::getKey(int index, KabKey& key)
{
  // ###########################################################################
  StringKabKeyMap::iterator pos;
  // -----
  if((unsigned)index<entries->size())
    {
      pos=entries->begin();
      advance(pos, index);
      key=(*pos).second;
      return NoError;
    } else {
      return NoSuchEntry;
    }
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::getIndex(const KabKey& key, int& index)
{
  register bool GUARD; GUARD=true;
  // ###########################################################################
  StringKabKeyMap::iterator pos;
  // -----
  index=0;
  for(pos=entries->begin(); pos!=entries->end(); ++pos)
    {
      // kdDebug(KAB_KDEBUG_AREA) << (*pos).second.getKey().data() << " <--> " <<
      // key.getKey().data() << endl;
      if((*pos).second==key) break;
      ++index;
    }
  kdDebug(pos==entries->end(), KAB_KDEBUG_AREA) <<
      "AddressBook::getIndex: no such key." << endl;
  if(pos==entries->end())
    {
      return NoSuchEntry;
    } else {
      return NoError;
    }
  // ###########################################################################
}

Section*
AddressBook::entrySection()
{
  // ###########################################################################
  Section* section;
  // -----
  if(!data->get(ENTRY_SECTION, section))
    {
      return 0;
    } else {
      return section;
    }
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::add(const Entry& entry, KabKey& key, bool update)
{
  bool GUARD; GUARD=true;
  kdDebug(GUARD, KAB_KDEBUG_AREA) << "AddressBook::add: called." << endl;
  // ###########################################################################
  Section* theEntries=entrySection();
  Section* newEntry;
  KabKey nextKey;
  ErrorCode locked;
  ErrorCode rc;
  // -----
  if(theEntries==0)
    {
      kdDebug(KAB_KDEBUG_AREA) << "AddressBook::add: no entries section." 
			       << endl;
      return NoFile;
    }
  newEntry=new Section;
  if(newEntry==0)
    {
      KMessageBox::error(this,
	 i18n("Cannot initialize local variables."),
	 i18n("Out of Memory"));
      kapp->quit(); // It is critical, but will possibly never happen.
      return InternError; // shut the compiler up...
    }
  // ----- lock the file:
  locked=lock();
  switch(locked)
    {
    case PermDenied:
      kdDebug(GUARD, KAB_KDEBUG_AREA) 
	<<  "AddressBook::add: permission denied." << endl;
      return PermDenied; // cannot get r/w mode
    case Locked:
      kdDebug(GUARD, KAB_KDEBUG_AREA) 
	<<  "AddressBook::add: db is already in r/w mode." << endl;
      break;
    case NoError:
      kdDebug(GUARD, KAB_KDEBUG_AREA) 
	<<  "AddressBook::add: got writing permissions." << endl;
      break;
    default:
      kdDebug(GUARD, KAB_KDEBUG_AREA) 
	<<  "AddressBook::add: unknown response, exiting." << endl;
      return InternError;
    }
  // -----
  if(makeSectionFromEntry(entry, *newEntry)==NoError)
    {
      nextKey=nextAvailEntryKey();
      if(!theEntries->add(nextKey.getKey(), newEntry))
	{
	    kdDebug(KAB_KDEBUG_AREA) 
	      << "AddressBook::add: Cannot insert section.\n";
	    rc=InternError;
	} else {
	  key=nextKey;
	  emit(changed());
	  rc=NoError;
	}
      if(update) updateMirrorMap();
    } else {
      rc=InternError;
    }
  if(locked!=Locked)
    { // ----- unlock the file here:
      kdDebug(GUARD, KAB_KDEBUG_AREA)
	<<  "AddressBook::add: dropped writing permissions." << endl;
      locked=unlock();
    }
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "AddressBook::add: done." << endl;
  if(locked!=NoError)
    {
      return locked;
    }
  if(rc!=NoError)
    {
      return rc;
    }
  return NoError;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::change(const KabKey& key, const Entry& entry)
{
  // ###########################################################################
  Section* theEntries=entrySection();
  Section* oldEntry;
  ErrorCode locked;
  ErrorCode rc;
  // -----
  if(theEntries==0)
    {
      return NoFile;
    }
  // ----- lock the file:
  locked=lock();
  if(locked==PermDenied)
    {
      return PermDenied; // cannot get r/w mode
    }
  // -----
  if(!theEntries->find(key.getKey(), oldEntry))
    {
      rc=NoSuchEntry;
    } else {
      oldEntry->clear();
      rc=makeSectionFromEntry(entry, *oldEntry);
      emit(changed());
    }
  // -----
  if(locked!=PermDenied)
    { // ----- unlock the file here:
      locked=unlock();
    }
  if(locked==NoError)
    {
      return rc;
    } else {
      return locked;
    }
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::remove(const KabKey& key)
{
  // ###########################################################################
  Section *theEntries=entrySection();
  ErrorCode locked;
  ErrorCode rc;
  // -----
  if(theEntries==0)
    {
      return NoFile;
    }
  // ----- lock the file:
  locked=lock();
  if(locked==PermDenied)
    {
      return PermDenied; // cannot get r/w mode
    }
  // -----
  if(theEntries->remove(key.getKey()))
    {
      rc=NoError;
      emit(changed());
    } else {
      rc=NoSuchEntry;
    }
  // -----
  if(locked!=PermDenied)
    { // ----- unlock the file here:
      locked=unlock();
    }
  if(locked==NoError)
    {
      return rc;
    } else {
      return locked;
    }
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::lock()
{
  // ###########################################################################
  if(!data->isRO()) return Locked;
  if(data->setFileName(data->fileName(), false, false))
    {
      return NoError;
    } else {
      KMessageBox::information(this,
	 i18n("The file you wanted to change could not be locked.\n"
	      "It is probably in use by another application or read-only."),
		 i18n("File Error"));
      return PermDenied;
    }
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::unlock()
{
  // ###########################################################################
  if(data->isRO()) return PermDenied;
  if(data->setFileName(data->fileName(), true, true))
    {
      return NoError;
    } else {
      return InternError;
    }
  // ###########################################################################
}

KabKey
AddressBook::nextAvailEntryKey()
{
  // ###########################################################################
  int max=0;
  int temp;
  Section::StringSectionMap::iterator pos;
  Section *section=entrySection();
  KabKey key;
  QCString dummy;
  bool good=true;
  // -----
  if(section!=0)
    {
      for(pos=section->sectionsBegin(); pos!=section->sectionsEnd(); ++pos)
	{
	  temp=0;
	  temp=(*pos).first.toInt(&good);
	  if(!good)
	    {
	      kdDebug(KAB_KDEBUG_AREA) 
		<< "AddressBook::nextAvailEntryKey: non-integer entry " 
		<< endl;
	    }
	  if(temp>max)
	    {
	      max=temp;
	    }
	}
    }
  // -----
  dummy.setNum(++max);
  key.setKey(dummy);
  // CHECK(key.getKey().toInt(&good)==max);
  return key;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::updateMirrorMap()
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) 
    << "AddressBook::updateMirrorMap: updating mirror map.\n";
  QString key;
  Entry entry;
  ErrorCode ec;
  KabKey kk;
  Section *section=entrySection();
  Section::StringSectionMap::iterator pos;
  // -----
  entries->erase(entries->begin(), entries->end());
  if(section==0)
    {
	kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "AddressBook::updateMirrorMap: done, "
	  "no file loaded." << endl;
      return NoError;
    }
  for(pos=section->sectionsBegin(); pos!=section->sectionsEnd(); ++pos)
    {
      if(makeEntryFromSection((*pos).second, entry)!=NoError)
	{
	  // return InternError; // it is saver to continue without a key
	}
      key="";
      ec=literalName(entry, key, true, false);
      if(key.isEmpty() || ec!=NoError)
	{
	  key=i18n("(empty entry)");
	}
      key+=(*pos).first; // append the section name to make the key unique
      kk.setKey((*pos).first);
      entries->insert(StringKabKeyMap::value_type(key, kk));
    }
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "AddressBook::updateMirrorMap: done."
				  << endl;
  return NoError;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::makeEntryFromSection(Section* section, Entry& entry)
{
  // ###########################################################################
  Section *addresses;
  Section *addressSection;
  Section::StringSectionMap::iterator pos;
  KeyValueMap *keys;
  Entry temp;
  Entry::Address address;
  Entry::Address addressDummy;
  int count;
  // ----- create the aggregats:
  const QCString StringKeys[]= {
    "title",
    "rank",
    "fn",
    "nameprefix",
    "firstname",
    "middlename",
    "lastname",
    "comment",
    "user1",
    "user2",
    "user3",
    "user4"
  };
  QString* StringValues[]= {
    &temp.title,
    &temp.rank,
    &temp.fn,
    &temp.nameprefix,
    &temp.firstname,
    &temp.middlename,
    &temp.lastname,
    &temp.comment,
    &temp.user1,
    &temp.user2,
    &temp.user3,
    &temp.user4
  };
  const int StringKeySize=sizeof(StringKeys)/sizeof(StringKeys[0]);
  const QCString StringListKeys[]= {
    "talk",
    "emails",
    "keywords",
    "telephone",
    "URLs",
    "custom",
    "categories"
  };
  QStringList* StringListValues[]= {
    &temp.talk,
    &temp.emails,
    &temp.keywords,
    &temp.telephone,
    &temp.URLs,
    &temp.custom,
    &temp.categories
  };
  const int StringListKeySize=sizeof(StringListKeys)/sizeof(StringListKeys[0]);
  // ----- first parse "addresses" subsection:
  if(!section->find(ADDRESS_SUBSECTION, addresses))
    {
      return InternError; // no subsection called "addresses"
    }
  for(pos=addresses->sectionsBegin(); pos!=addresses->sectionsEnd(); ++pos)
    {
      if(!addresses->find((*pos).first, addressSection))
	{
	  return InternError; // no section we have an iterator for?
	}
      keys=addressSection->getKeys();
      address=addressDummy; // clean it up
      if(makeAddressFromMap(keys, address)==AddressBook::NoError)
	{
	  // ----- add the address to the list of addresses:
	  temp.addresses.push_back(address);
	} else {
	  kdDebug(KAB_KDEBUG_AREA)
	    << "AddressBook::makeEntryFromSection: cannot find all fields "
	    << "in an address subsection." << endl;
	}
    }
  // ----- now parse all other fields directly:
  keys=section->getKeys();
  for(count=0; count<StringKeySize; ++count)
    {
      if(!keys->get(StringKeys[count], *StringValues[count]))
	{
	  /* Spits out lots of warnings:
	    kdDebug(KAB_KDEBUG_AREA)
	    << "AddressBook::makeEntryFromSection: error: could not get "
	    << "value for key " << (const char*)StringKeys[count]
	    << "." << endl;
	  */
	}
    }
  for(count=0; count<StringListKeySize; ++count)
    {
      if(!keys->get(StringListKeys[count], *StringListValues[count]))
	{
	  /* Spits out lots of warnings:
	     kdDebug(KAB_KDEBUG_AREA)
	     << "AddressBook::makeEntryFromSection: error: could not get "
	     << "value for key " << (const char*)StringListKeys[count]
	     << "." << endl;
	  */
	}
    }
  // ----- finally get the birthday:
  keys->get("birthday", temp.birthday); // this may return false (no date)
  // -----
  entry=temp;
  return NoError;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::makeAddressFromMap(KeyValueMap* keys, Entry::Address& address)
{
  // ###########################################################################
  const QCString Keys[]= {
    "headline",
    "position",
    "org",
    "orgunit",
    "orgsubunit",
      // "role",
    "deliverylabel",
    "address",
    "zip",
    "town",
    "country",
    "state"
  };
  QString* strings[]= {
    &address.headline,
    &address.position,
    &address.org,
    &address.orgUnit,
    &address.orgSubUnit,
      // &address.role,
    &address.deliveryLabel,
    &address.address,
    &address.zip,
    &address.town,
    &address.country,
    &address.state
  };
  const int Size=sizeof(Keys)/sizeof(Keys[0]);
  int count;
  // -----
  for(count=0; count<Size; ++count)
    {
      keys->get(Keys[count], *strings[count]);
    }
  return NoError;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::makeSectionFromEntry(const Entry& entry, Section& section)
{
  // ###########################################################################
  list<Entry::Address>::const_iterator addPos;
  Section *addresses=0;
  Section *address=0;
  QCString key; // used for creating address subsection keys
  int count=0; // counts the addresses
  KeyValueMap *keys;
  // ----- prepare the section object:
  section.clear();
  // ----- first create "addresses" subsection:
  if(!section.add(ADDRESS_SUBSECTION))
    {
      kdDebug(KAB_KDEBUG_AREA) 
	<< "AddressBook::makeSectionFromEntry: cannot create " << "subsection." 
	<< " " << endl;
      return InternError;
    }
  if(!section.find(ADDRESS_SUBSECTION, addresses))
    {
      kdDebug(KAB_KDEBUG_AREA) 
	<< "AddressBook::makeSectionFromEntry: cannot get new section." << endl;
      return InternError;
    }
  // ----- now insert addresses:
  for(addPos=entry.addresses.begin(); addPos!=entry.addresses.end(); ++addPos)
    {
      ++count;
      key.setNum(count);
      if(!addresses->add(key))
	{
	  kdDebug(KAB_KDEBUG_AREA) 
	    << "AddressBook::makeSectionFromEntry: cannot create address " << endl;
	  return InternError;
	}
      if(!addresses->find(key, address))
	{
	  kdDebug(KAB_KDEBUG_AREA) 
	    << "AddressBook::makeSectionFromEntry: cannot get new " << endl;
	  return InternError;
	}
      keys=address->getKeys();
      // ----- now insert keys into address:
      if(!keys->insert("headline", (*addPos).headline) ||
	 !keys->insert("position", (*addPos).position) ||
	 !keys->insert("org", (*addPos).org) ||
	 !keys->insert("orgunit", (*addPos).orgUnit) ||
	 !keys->insert("orgsubunit", (*addPos).orgSubUnit) ||
	 // !keys->insert("role", (*addPos).role) ||
	 !keys->insert("deliverylabel", (*addPos).deliveryLabel) ||
	 !keys->insert("address", (*addPos).address) ||
	 !keys->insert("zip", (*addPos).zip) ||
	 !keys->insert("town", (*addPos).town) ||
	 !keys->insert("country", (*addPos).country) ||
	 !keys->insert("state", (*addPos).state))
	{
	  kdDebug(KAB_KDEBUG_AREA) 
	    << "AddressBook::makeSectionFromEntry: cannot completely "
	    << "insert this address." << endl;
	  return InternError;
	}
    }
  // ----- now add the other fields:
  keys=section.getKeys();
  if(!keys->insert("title", entry.title) ||
     !keys->insert("rank", entry.rank) ||
     !keys->insert("fn", entry.fn) ||
     !keys->insert("nameprefix", entry.nameprefix) ||
     !keys->insert("firstname", entry.firstname) ||
     !keys->insert("middlename", entry.middlename) ||
     !keys->insert("lastname", entry.lastname) ||
     !keys->insert("birthday", entry.birthday) ||
     !keys->insert("comment", entry.comment) ||
     !keys->insert("talk", entry.talk) ||
     !keys->insert("emails", entry.emails) ||
     !keys->insert("keywords", entry.keywords) ||
     !keys->insert("telephone", entry.telephone) ||
     !keys->insert("URLs", entry.URLs) ||
     !keys->insert("user1", entry.user1) ||
     !keys->insert("user2", entry.user2) ||
     !keys->insert("user3", entry.user3) ||
     !keys->insert("user4", entry.user4) ||
     !keys->insert("custom", entry.custom) || 
     !keys->insert("categories", entry.categories))
    {
      kdDebug(KAB_KDEBUG_AREA) 
	<< "AddressBook::makeEntryFromSection: cannot insert "
	<< "all fields of the entry." << endl;
      return InternError;
    }
  // -----
  return NoError;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::createNew(const QString& filename)
{
  // ###########################################################################
  const QString KabTemplateFile=locate("data", "kab/template.kab");
  kdDebug(KAB_KDEBUG_AREA) 
    << "AddressBook::createNew: template file is \"" 
    << (const char*)KabTemplateFile.utf8() << "\"." << endl;
  QConfigDB db;
  // -----
  if(KabTemplateFile.isEmpty()
     || !db.setFileName(KabTemplateFile, true, true))
    {
      KMessageBox::error(this,
	 i18n("Cannot find kab's template file.\n"
	      "You cannot create new files."),
	 i18n("File Error"));
      return InternError;
    }
  if(!db.load())
    {
      KMessageBox::error(this,
	 i18n("Cannot read kab's template file.\n"
	      "You cannot create new files."),
	 i18n("Format Error"));

      return InternError;
    }
  if(!db.setFileName(filename, false, false))
    {
      KMessageBox::error(this,
	 i18n("Cannot create the file\n\"")
	 +filename+"\"\n"+
	 i18n("Could not create the new file."),
		 i18n("File Error"));
      return PermDenied;
    }
  if(!db.save())
    {
      KMessageBox::error(this,
	 i18n("Cannot save the file\n\"")
	 +filename+"\"\n"+
	 i18n("Could not create the new file."),
		 i18n("File Error"));
      return InternError;
    }
  // -----
  return NoError;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::createConfigFile()
{
  // ###########################################################################
  const QString ConfigTemplateFile=locate("data", "kab/template.config");
  kdDebug(KAB_KDEBUG_AREA) 
    << "AddressBook::createConfigFile: config template file is \"" 
    << (const char*)ConfigTemplateFile.utf8() << "\"." << endl;
  const QString filename= locateLocal( "data", STD_CONFIGFILENAME);
  QConfigDB db;
  // -----
  if(ConfigTemplateFile.isEmpty()
     || !db.setFileName(ConfigTemplateFile, true, true))
    {
      KMessageBox::error(this,
	 i18n("Cannot find kab's configuration template file.\n"
	      "kab cannot be configured."),
	 i18n("File Error"));

      return InternError;
    }
  if(!db.load())
    {
      KMessageBox::error(this,
	 i18n("Cannot read kab's configuration template file.\n"
	      "kab cannot be configured."),
	 i18n("File Error"));
      return InternError;
    }
  if(!db.setFileName(filename, false, false))
    {
      KMessageBox::error(this,
	 i18n("Cannot create the file\n\"")
	 +filename+"\"\n"+
	 i18n("Could not create the new configuration file."),
	 i18n("File Error"));
      return PermDenied;
    }
  if(!db.save())
    {
      KMessageBox::error(this,
	 i18n("Cannot save the file\n\"")
	 +filename+"\"\n"+
	 i18n("Could not create the new configuration file."),
	 i18n("File Error"));
      return InternError;
    }
  // -----
  return NoError;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::loadConfigFile()
{
  // ###########################################################################
  QString file = locateLocal( "data", STD_CONFIGFILENAME);
  if(config->setFileName(file, true, true))
    {
      if(config->load())
	{
	  return NoError;
	} else {
	  KMessageBox::information(this,
	     i18n("Cannot load kab's local configuration file.\n"
		  "There may be a formatting error.\n"
		  "kab cannot be configured."),
	     i18n("File Error"));
	  return InternError;
	}
    } else {
      KMessageBox::information(this,
	 i18n("Cannot find kab's local configuration file.\n"
	      "kab cannot be configured."),
	 i18n("File Error"));
      return NoSuchFile;
    }
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::makeVCardFromEntry(const Entry&, const QString&)
{
  // ###########################################################################
  return NotImplemented;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::makeEntryFromVCard(const QString&, Entry&)
{
  // ###########################################################################
  return NotImplemented;
  // ###########################################################################
}

QString
AddressBook::getStandardFileName()
{
  // ###########################################################################
  return locateLocal( "data", STD_USERFILENAME);
  // ###########################################################################
}

QString AddressBook::phoneType(AddressBook::Telephone phone)
{
  switch(phone)
    {
    case Fixed: return i18n("fixed"); break;
    case Mobile: return i18n("mobile"); break;
    case Fax: return i18n("fax"); break;
    case Modem: return i18n("modem"); break;
    default: return i18n("general");
    }
}

void AddressBook::externalChange()
{
  updateMirrorMap();
}

Section* AddressBook::configurationSection()
{
  Section *section;
  if(data!=0)
    {
      if(data->get(LOCAL_CONFIG_SECTION, section))
	{
	  return section;
	} else {
	  return 0;
	}
    } else {
      return 0;
    }
}

AddressBook::ErrorCode AddressBook::Entry::get(const char* fieldname, QVariant& field)
{
  // "title", "rank", "fn", "nameprefix", "firstname", "middlename", "lastname",
  // "birthday", "comment", "talk", "emails", "keywords", "telephone",
  // "urls", "user1", "user2", "user3", "user4", "custom"
  int dummy=0;
  // -----
  if(fieldname==Fields[dummy++])
    { // the title
      field=title;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the rank
      field=rank;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the formatted name
      field=fn;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the nameprefix
      field=nameprefix;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the firstname
      field=firstname;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the middle name
      field=middlename;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the last name
      field=lastname;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the birthday
      field=birthday.toString();
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the comment
      field=comment;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the talk addresses
      field=talk;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the email addresses
      field=emails;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the keywords
      field=keywords;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the telephones
      field=telephone;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the urls
      field=URLs;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the user field 1
      field=user1;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the user field 2
      field=user2;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the user field 3
      field=user3;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the user field 4
      field=user4;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the custom fields (app specific)
      field=custom;
      return NoError;
    }
  // ----- we did not find that field:
  return NoSuchField;
}

AddressBook::ErrorCode AddressBook::Entry::Address::get(const char* fieldname, 
						       QVariant& field)
{
  // "headline", "position", 
  // "org", "orgunit", "orgsubunit",
  // "deliverylabel", "address", "zip", "town", "country", "state"
  int dummy=0;
  // -----
  if(fieldname==Fields[dummy++])
    { // the headline
      field=headline;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the position
      field=position;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the organisation
      field=org;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the organisational unit
      field=orgUnit;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the organisational subunit
      field=orgSubUnit;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the delivery label
      field=deliveryLabel;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the address
      field=address;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the zip code
      field=zip;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the town
      field=town;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the country
      field=country;
      return NoError;
    }
  if(fieldname==Fields[dummy++])
    { // the state
      field=state;
      return NoError;
    }
  // ----- we did not find that field:
  return NoSuchField;
}

Section* AddressBook::categoriesSection()
{
  const QString Predefines[]= {
    i18n("Business"),
    i18n("Private"),
    i18n("Dates") };
  size_t Size=sizeof(Predefines)/sizeof(Predefines[0]);
  Section* section;
  Section* categories;
  KeyValueMap *keys;
  // -----
  if(data->get(KAB_CATEGORY_KEY, section))
    {
      // it exists, go ahead
      return section;
    } else {
      kdDebug(KAB_KDEBUG_AREA) 
	<< "AddressBook::categoriesSection: creating categories structure." << endl;
      // it does not exist - create it
      if(!data->createSection(KAB_CATEGORY_KEY))
	{
	  kdDebug(KAB_KDEBUG_AREA) 
	    << "AddressBook::categoriesSection: error creating categories section."
	    << endl;
	  return 0;
	}
      data->get(KAB_CATEGORY_KEY, section);
      // add the predefined categories:
      categories=new Section;
      keys=categories->getKeys(); 
      for(size_t count=0; count<Size; ++count)
	{
	  QStringList values; 
	  values.append(Predefines[count]);
	  keys->insert(QCString().setNum(count), values);
	}
      section->add(KAB_CATEGORY_KEY, categories);
      keys=section->getKeys(); 
      keys->insert("NextAvailableCatKey", (long)Size);
    }
  save();
  if(data->get(KAB_CATEGORY_KEY, section))
      return section;
  return 0; // might not happen
}

AddressBook::ErrorCode AddressBook::categories(CategoriesMap& cat)
{ // WORK_TO_DO: use a permanent cached map and update on changed()
  kdDebug(KAB_KDEBUG_AREA, !cat.isEmpty())
    << "AddressBook::categories: warning - categories map is supposed to be empty!"
    << endl;
  Section *section;
  Section *categories;
  KeyValueMap* keys;
  int key;
  bool rc;
  QStringList values;
  StringStringMap::iterator pos;
  // ----- query categories section:
  section=categoriesSection();
  Q_CHECK_PTR(section);
  // ----- 
  if(!section->find(KAB_CATEGORY_KEY, categories))
    {
      kdDebug(KAB_KDEBUG_AREA) 
	<< "AddressBook::categories: error in database structure."
	<< endl;
      return InternError;
    } 
  // ----- everything is set up, create the categories map:
  //       use an iterator to walk over all elements of categories key-value-map:
  keys=categories->getKeys();
  for(pos=keys->begin(); pos!=keys->end(); ++pos)
    {
      if(!keys->get((*pos).first, values))
	{
	  kdDebug(KAB_KDEBUG_AREA)
	    << "AddressBook::categories: internal error querying categories."
	    << endl;
	} else {
	  key=(*pos).first.toInt(&rc);
	  if(rc)
	    {
	      cat.insert(key, values[0]);
	    } else {
	      kdDebug(KAB_KDEBUG_AREA)
		<< "AddressBook::categories: error - non-integer category key - ignored."
		<< endl;
	    }
	}
    }
  return NoError;
}

#include "addressbook.moc"
