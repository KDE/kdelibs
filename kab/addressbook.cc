/* -*- C++ -*-
 * This file implements the basic personal information management class
 * used in the KDE addressbook.
 *
 * the KDE addressbook
 * copyright:  (C) Mirko Sucker, 1998, 1999
 * mail to:    Mirko Sucker <mirko@kde.org>
 * requires:   recent C++-compiler, at least Qt 2.1

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

 * $Id$
 */

#include "addressbook.h"
#include "qconfigDB.h"
#include <qmessagebox.h>

#include <qfileinfo.h>
#include <qstringlist.h>

#include <kapp.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <knana.h>

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
#define STD_USERFILENAME "addressbook.kab"
#ifdef STD_CONFIGFILENAME
#undef STD_CONFIGFILENAME
#endif
#define STD_CONFIGFILENAME "kab.config"
#ifdef KAB_LOCALDIR
#undef KAB_LOCALDIR
#endif
#define KAB_LOCALDIR "kab" // change to "kab" after hot states
#ifdef ENTRY_SECTION
#undef ENTRY_SECTION
#endif
#define ENTRY_SECTION "entries"
#ifdef LOCAL_CONFIG_SECTION // the name of the file-local configuration section
#undef LOCAL_CONFIG_SECTION
#endif
#define LOCAL_CONFIG_SECTION "config"
#ifdef ADDRESS_SUBSECTION // the name of the subsection for each entry
#undef ADDRESS_SUBSECTION
#endif
#define ADDRESS_SUBSECTION "addresses"
#ifdef KAB_TEMPLATEFILE
#undef KAB_TEMPLATEFILE
#endif
#define KAB_TEMPLATEFILE "template.kab"
#ifdef KAB_CONFIGTEMPLATE
#undef KAB_CONFIGTEMPLATE
#endif
#define KAB_CONFIGTEMPLATE "template.config"

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

/*
  const QConfigDB*
  AddressBook::Entry::getFile()
  {
  // ###########################################################################
  return file;
  // ###########################################################################
  }
*/

AddressBook::ErrorCode
AddressBook::Entry::getAddress(int index, Address& address)
{
  // ###########################################################################
  list<Address>::iterator pos;
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
    // background(new QImage),
    // backgroundEnabled(true),
    entries(new StringKabKeyMap),
    state(NoFile)
    // currentAddress(0),
    // addressCombo(new QComboBox(this))
{
  register bool GUARD; GUARD=true;
  // ###########################################################################
  QString dir, filename;
  // ----- do memory checks (do not rely on exception handling):
  if(config==0 || data==0 || entries==0
     /* || urlEmail==0 || urlHomepage==0
	|| addressCombo==0 || background==0 */ )
    {
      KMessageBox::error(this,
	 i18n("Cannot initialize local variables."),
	 i18n("Out of memory"));
      kapp->quit(); // It is critical, but will possibly never happen.
    }
  connect(data, SIGNAL(fileChanged()), SLOT(dataFileChanged()));
  connect(data, SIGNAL(changed(QConfigDB*)),
	  SLOT(reloaded(QConfigDB*)));
  connect(config, SIGNAL(fileChanged()), SLOT(configFileChanged()));
  // ----- set style:
  setLineWidth(1);
  setFrameStyle(QFrame::Box | QFrame::Sunken);
  // urlEmail->setAutoResize(true);
  // urlEmail->setTransparentMode(false);
  // urlEmail->setLineWidth(0);
  // urlEmail->setMidLineWidth(0);
  // urlHomepage->setAutoResize(true);
  // urlHomepage->setTransparentMode(false);
  // urlHomepage->setLineWidth(0);
  // urlHomepage->setMidLineWidth(0);
  // setInteractiveMode(false);
  // -----
  // connect(addressCombo, SIGNAL(highlighted(int)), SLOT(addressSelected(int)));
  // -----
  // connect(urlEmail, SIGNAL(leftClickedURL(const char*)), SLOT(mailURLClicked(const char*)));
  // connect(urlHomepage, SIGNAL(leftClickedURL(const char*)), SLOT(homeURLClicked(const char*)));
  // ----- check and possibly create kab's local directory:
  dir=baseDir();
  CHECK(dir!=QString::null);
  // ----- open or create the configuration file and load it:
  filename=dir+"/"+STD_CONFIGFILENAME;
  if(::access(filename.ascii(), F_OK)!=0) // if it does not exist
    {
      if(createConfigFile()!=NoError)
	{
	  KMessageBox::error(this,
	     i18n("Your local kab configuration file\n"
		  "\"%1\"\n"
		  "could not be created. kab will probably not\n"
		  "work correctly without it.\n"
		  "Make sure you did not remove writing permission\n"
		  "from your local kde directory.").arg(filename),
	     i18n("File error"));
	  state=PermDenied;
	} else {
	  KMessageBox::information
	    (this,
	     i18n("kab has created your local configuration file in\n"
		  "\"%1\"").arg(filename));
	}
    }
  loadConfigFile();
  // ----- check and possibly create user standard file:
  filename=dir+"/"+STD_USERFILENAME;
  if(::access(filename.ascii(), F_OK)!=0) // if it does not exist
    {
      if(createNew(filename)!=NoError) // ...and we cannot create it
	{
	  KMessageBox::error(this,
	     i18n("Your standard kab database file\n"
		  "\"%1\"\n"
		  "could not be created. kab will probably not\n"
		  "work correctly without it.\n"
		  "Make sure you did not remove writing permission\n"
		  "from your local kde directory.").arg(filename),
	     i18n("File error"));
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
	  // ----- create the backup file:
	  QString temp=data->fileName();
	  if(data->setFileName(temp+".backup", false, false))
	    {
	      if(!data->save())
		{
		  KMessageBox::information(this,
		     i18n("Cannot create backup file (permission denied)."),
		     i18n("File error"));
		}
	    } else {
	      KMessageBox::error(this,
		 i18n("Cannot open backup file for "
		      "writing (permission denied)."),
	 i18n("File error"));

	    }
	  // ----- reset the filename:
	  if(!data->setFileName(temp, true, true))
	    {
	      KMessageBox::error(this,
		 i18n("Critical error:\n"
		      "Permissions changed in local directory!"),
	 i18n("File error"));

	      closeFile(false);
	      state=PermDenied;
	    } else {
	      state=NoError;
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
  // delete background;
  delete entries;
  // ###########################################################################
}

QConfigDB*
AddressBook::getConfig()
{
  // ###########################################################################
  return config;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::getState()
{
  // ###########################################################################
  return state;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::load(QString filename)
{
  // ----- Remark: Close the file if it could not be loaded!
  REQUIRE(access(baseDir().ascii(), X_OK | F_OK)==0);
  // ###########################################################################
  const QString dir=baseDir();
  ErrorCode rc=NoError;
  QFileInfo newfile, oldfile;
  // -----
  if(filename.isEmpty())
    { // ----- reload the curent data file:
      filename=data->fileName();
    }
  if(filename.isEmpty()) // there was never a filename set:
    {
      state=NoFile;
      return NoFile;
    }
  // -----
  newfile.setFile(filename);
  oldfile.setFile(data->fileName());
  if(isSameFile(filename, data->fileName()))
    { // ----- possibly deleted file:
      if(data->load())
	{
	  emit(setStatus(i18n("File reloaded.")));
	  state=NoError;
	} else {
	  switch
	    (QMessageBox::information
	     (this, i18n("File error"),
	      i18n("The currently loaded file\n\"")
	      +oldfile.absFilePath()+"\"\n"+
	      i18n
	      ("cannot be reloaded. kab may close or save it.\n"
	       "Save it if you accidently deleted your data file.\n"
	       "Close it if you intended to do so.\n"
	       "Your file will be closed by default."),
	      i18n("&Close"), i18n("&Save")))
	    {
	    case 1: // save
	      if(!data->save(i18n("(Safety copy on file error)"), true))
		{
		  KMessageBox::information(this,
		     i18n("Cannot save the file, will close it now."),
		     i18n("File error"));
		  closeFile(false);
		  rc=PermDenied;
		}
	      break; // no error if we could save the file
	    default: // close
	      closeFile(false);
	      rc=NoSuchFile;
	      break;
	    }
	}
    } else { // ----- set new filename
      if(data->setFileName(filename, true, true))
	{
	  if(data->load())
	    {
	      emit(changed());
	      emit(setStatus(i18n("File opened.")));
	      state=NoError;
	    } else {
	      KMessageBox::information(this,
		 i18n("Could not load the file."),
		 i18n("File error"));
	      closeFile(false);
	      emit(setStatus(i18n("No such file.")));
	      rc=NoSuchFile;
	    }
	} else {
	  if(QMessageBox::information
	     (this, i18n("No such file"),
	      i18n("The file\n\"")
	      +filename+"\"\n"+
	      i18n(" cannot be found. Create a new one?"),
	      i18n("OK"), i18n("Cancel"))==0)
	    {
	      if(createNew(filename)==NoError)
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
  ENSURE((unsigned)strings->count()==entries->size());
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
	 i18n("File error"));
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
  register bool GUARD; GUARD=true;
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

/*
  AddressBook::ErrorCode
  AddressBook::displayEntry(const AddressBook::Entry& entry, int index)
  {
  // ###########################################################################
  QStrList strings;
  list<AddressBook::Entry::Address>::iterator pos;
  // -----
  current=entry;
  currentAddress=index;
  for(pos=current.addresses.begin(); pos!=current.addresses.end(); ++pos)
  {
  strings.append((*pos).headline);
  }
  addressCombo->clear();
  addressCombo->insertStrList(&strings);
  repaint(false);
  return NoError;
  // ###########################################################################
  }

  AddressBook::ErrorCode
  AddressBook::displayEntry(int position, int index)
  {
  // ###########################################################################
  KabKey key;
  QStrList strings;
  list<AddressBook::Entry::Address>::iterator pos;
  // -----
  if(getKey(position, key)!=NoError)
  {
  return NoSuchEntry;
  }
  if(getEntry(key, current)!=NoError)
  {
  CHECK(false);
  return InternError;
  }
  currentAddress=index;
  for(pos=current.addresses.begin(); pos!=current.addresses.end(); ++pos)
  {
  strings.append((*pos).headline);
  }
  addressCombo->clear();
  addressCombo->insertStrList(&strings);
  repaint(false);
  return NoError;
  // ###########################################################################
  }
*/


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
	  CHECK(false); // Inconsistency between mirror map and database!
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
  // -----
  thelist.erase(thelist.begin(), thelist.end());
  CHECK(thelist.empty());
  for(pos=entries->begin(); pos!=entries->end(); ++pos)
    {
      if(getEntry((*pos).second, entry))
	{
	  thelist.push_back(entry);
	} else {
	  CHECK(false); // Inconsistency between mirror map and database!
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
	kdDebug() << (*pos).second.getKey().data() << " <--> " <<
	    key.getKey().data() << endl;
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
	kdDebug() << "AddressBook::add: no entries section." << endl;
      return NoFile;
    }
  newEntry=new Section;
  if(newEntry==0)
    {
      KMessageBox::error(this,
	 i18n("Cannot initialize local variables."),
	 i18n("Out of memory"));
      kapp->quit(); // It is critical, but will possibly never happen.
      return InternError; // shut the compiler up...
    }
  // ----- lock the file:
  locked=lock();
  switch(locked)
    {
    case PermDenied:
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "AddressBook::add: permission denied." << endl;
      return PermDenied; // cannot get r/w mode
    case Locked:
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "AddressBook::add: db is already in r/w mode." << endl;
      break;
    case NoError:
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "AddressBook::add: got writing permissions." << endl;
      break;
    default:
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "AddressBook::add: unknown response, exiting." << endl;
      return InternError;
    }
  // -----
  if(makeSectionFromEntry(entry, *newEntry)==NoError)
    {
      nextKey=nextAvailEntryKey();
      if(!theEntries->add(nextKey.getKey(), newEntry))
	{
	  kDebugInfo("AddressBook::add: Cannot insert section.");
	  rc=InternError;
	} else {
	  key=nextKey;
	  rc=NoError;
	}
      if(update) updateMirrorMap();
    } else {
      rc=InternError;
    }
  if(locked!=Locked)
    { // ----- unlock the file here:
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "AddressBook::add: dropped writing permissions." << endl;
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
      CHECK(oldEntry->empty());
      rc=makeSectionFromEntry(entry, *oldEntry);
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
		 i18n("File error"));
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

/*
  void
  AddressBook::resizeEvent(QResizeEvent*)
  {
  // ###########################################################################
  const int Grid=3;
  const int FW=frameWidth();
  // -----
  addressCombo->setGeometry
  (Grid+FW,
  3*Grid+FW+2*fontMetrics().height(),
  width()-2*Grid-2*FW,
  addressCombo->sizeHint().height());
  // ###########################################################################
  }
*/

/* The view of this widget is partened into four parts. On top the name of the
   person is displayed, including titles and ranks. Two lines of text are
   reserved for this.
   Below this two lines the address is shown, below a KDataNavigator that allows
   to select an address. This part needs the height of the KDataNavigator and
   four lines of text. A line is drawn below it.
   The comment is printed below the address. The comment field stretches to the
   space available, since it may be a long text.
   On the bottom contact information is displayed, the telephone numbers, email
   addresses and the URLs. */
// WORK_TO_DO: the comment is not displayed by now.
/*
  void
  AddressBook::paintEvent(QPaintEvent* e)
  {
  REQUIRE(background!=0);
  // ###########################################################################
  const int Grid=3;
  const int FW=frameWidth();
  QFont original, font;
  QFrame::paintEvent(e);
  bool useBackground=false;
  bool drawSeparator=false;
  QPixmap pm(contentsRect().width(), contentsRect().height());
  QPainter p;
  QString temp;
  int posSeparator=0, cy, addressHeight, contactHeight;
  Entry::Address address;
  int nameHeight, bottomHeight, noOfBottomLines=0;
  bool showName, showAddress, showBottom;
  // ----- begin painting:
  p.begin(&pm);
  // ----- decide some style rules, get fonts, get the address:
  if(!background->isNull() && backgroundEnabled==true)
  {
  useBackground=true;
  } else {
  useBackground=false;
  }
  if(current.getAddress(currentAddress, address)!=NoError)
  {
  // kDebugInfo("AddressBook::paintEvent: address index out of range!\n");
  }
  // ----- draw the background:
  if(useBackground)
  {
  CHECK(background!=0 && !background->isNull());
  QPixmap pm;
  pm=*background;
  p.drawTiledPixmap(0, 0, width(), height(), pm);
  } else {
  p.setPen(kapp->backgroundColor);
  p.setBrush(kapp->backgroundColor);
  p.drawRect(0, 0, width(), height());
  p.setPen(black);
  }
  // ----- now draw on the background:
  original=p.font();
  font.setFamily(original.family());
  font.setPointSize(10);
  p.setFont(font);
  // ----- calculate sizes and decide display contents:
  nameHeight=2*Grid+FW+2*p.fontMetrics().height();
  if(!current.telephone.empty()) noOfBottomLines+=1;
  if(!current.URLs.empty()) noOfBottomLines+=1;
  if(!current.emails.empty()) noOfBottomLines+=1;
  bottomHeight=3*Grid+FW+noOfBottomLines*p.fontMetrics().height();
  addressHeight=height()-nameHeight-bottomHeight;
  showAddress=height()-nameHeight-bottomHeight
  >addressCombo->sizeHint().height()+4*QFontMetrics(original).height();
showBottom=height()-nameHeight>=bottomHeight;
showName=height()>=nameHeight;
//   kDebugInfo("AddressBook::paintEvent: %sshow name,\n"
//     "                         %sshow address,\n"
//     "                         %sshow bottom.\n",
//     showName ? "" : "do not ",
//     showAddress ? "" : "do not ",
//     showBottom ? "" : "do not ");
// ----- now draw the contact lines and labels:
if(showBottom)
{
cy=pm.height()-Grid;
if(!current.URLs.empty())
{
temp=(QString)"URL: "+current.URLs.front();
if(current.URLs.size()>1)
{
temp+=" [..]";
}
urlHomepage->setFont(font);
urlHomepage->setText(temp);
urlHomepage->move(Grid+frameWidth(),
  cy+3+frameWidth()-urlHomepage->height());
urlHomepage->show();
cy-=urlHomepage->height();
drawSeparator=true;
temp="";
} else {
urlHomepage->hide();
}
if(!current.emails.empty())
{
temp=(QString)"email: "+current.emails.front();
if(current.emails.size()>1)
{
temp+=" [..]";
}
urlEmail->setFont(font);
urlEmail->setText(temp);
urlEmail->move(Grid+frameWidth(), cy-urlEmail->height()+5);
urlEmail->show();
cy-=urlEmail->height();
drawSeparator=true;
temp="";
} else {
urlEmail->hide();
}
if(!current.telephone.empty())
{
temp+=QString("tel: ")+current.telephone.front();
if(current.telephone.size()>1)
{
temp+=" [..]";
}
p.drawText(Grid, cy, temp);
cy-=p.fontMetrics().height();
drawSeparator=true;
temp="";
}
if(drawSeparator)
{
  posSeparator=cy;
  cy-=Grid;
}
contactHeight=height()-cy;
CHECK(contactHeight>0);
if(drawSeparator)
{
  p.drawLine(Grid, posSeparator, pm.width()-Grid, posSeparator);
}
} else {
  urlHomepage->hide();
urlEmail->hide();
}
// (now contactHeight is the actual number of pixels needed)
if(showName)
{
// ----- print the birthday in the upper right corner if it is valid:
if(current.birthday.isValid())
  { //       by now I do not take care if there is enough space left
    p.drawText
      (pm.width()-Grid-p.fontMetrics().width(current.birthday.toString()),
       Grid+p.fontMetrics().ascent(), current.birthday.toString());
  }
// ----- draw the address, begin on top
 cy=Grid;
 font.setPointSize(12);
 p.setFont(font);
 if(!current.fn.isEmpty())
   {
     temp=current.fn;
   } else {
     if(!current.rank.isEmpty())
       {
	 if(!temp.isEmpty()) temp+=" ";
	 temp+=current.rank;
       }
     if(!current.nameprefix.isEmpty())
       {
	 if(!temp.isEmpty()) temp+=" ";
	 temp+=current.nameprefix;
       }
if(!current.firstname.isEmpty())
{
  if(!temp.isEmpty()) temp+=" ";
  temp+=current.firstname;
}
if(!current.middlename.isEmpty())
{
  if(!temp.isEmpty()) temp+=" ";
  temp+=current.middlename;
}
if(!current.lastname.isEmpty())
{
  if(!temp.isEmpty()) temp+=" ";
  temp+=current.lastname;
}
}
if(!temp.isEmpty())
{
  font.setItalic(true);
  p.setFont(font);
  p.setPen(blue);
  p.drawText(2*Grid, cy+p.fontMetrics().height(), temp);
  font.setItalic(false);
  p.setFont(font);
  p.setPen(black);
  cy+=p.fontMetrics().height();
}
if(!current.title.isEmpty())
{
  p.drawText(2*Grid, cy+p.fontMetrics().height(), current.title);
  cy+=p.fontMetrics().height();
}
}
// ----- now draw the address:
if(showAddress)
{
  // find starting point:
  cy=addressCombo->y()+addressCombo->height();
  if(!address.role.isEmpty())
    {
      p.drawText(2*Grid, cy+p.fontMetrics().height(), address.role);
      cy+=p.fontMetrics().height();
    }
  if(!address.address.isEmpty())
    {
      p.drawText(2*Grid, cy+p.fontMetrics().height(), address.address);
      cy+=p.fontMetrics().height();
    }
  temp = "";
  if(!address.town.isEmpty())
    {
      temp+=address.town;
    }
  if(!address.state.isEmpty())
    {
      if (!temp.isEmpty()) temp += " ";
      temp+=address.state;
    }
  if(!address.zip.isEmpty())
    {
      if (!temp.isEmpty()) temp += " ";
      temp += address.zip;
      p.drawText(2*Grid, cy+p.fontMetrics().height(), temp);
      cy+=p.fontMetrics().height();
    }
  if(!address.country.isEmpty())
    {
      p.drawText(2*Grid, cy+p.fontMetrics().height(), address.country);
      cy+=p.fontMetrics().height();
    }
  addressHeight=cy+Grid;
  addressCombo->show();
} else {
  addressCombo->hide();
}
// ----- finish painting:
p.end();
bitBlt(this, contentsRect().left(), contentsRect().top(), &pm);
// ###########################################################################
}
*/

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
	      kDebugInfo("AddressBook::nextAvailEntryKey: non-integer entry "
			 "key.");
	      CHECK(false);
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
  CHECK(key.getKey().toInt(&good)==max);
  return key;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::updateMirrorMap()
{
  register bool GUARD; GUARD=true;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	     "AddressBook::updateMirrorMap: updating mirror map.\n";
  QString key;
  Entry entry;
  ErrorCode ec;
  KabKey kk;
  Section *section=entrySection();
  Section::StringSectionMap::iterator pos;
  // -----
  entries->erase(entries->begin(), entries->end());
  CHECK(entries->empty());
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
	  CHECK(false);
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
  ENSURE(entries->size()==section->noOfSections());
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "AddressBook::updateMirrorMap: done." << endl;
  return NoError;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::makeEntryFromSection(Section* section, Entry& entry)
{
  REQUIRE(section!=0);
  // ###########################################################################
  // -----
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
  CHECK(StringKeySize==sizeof(StringValues)/sizeof(StringValues[0]));
  const QCString StringListKeys[]= {
    "talk",
    "emails",
    "keywords",
    "telephone",
    "URLs"
  };
  QStringList* StringListValues[]= {
    &temp.talk,
    &temp.emails,
    &temp.keywords,
    &temp.telephone,
    &temp.URLs
  };
  const int StringListKeySize=sizeof(StringListKeys)/sizeof(StringListKeys[0]);
  CHECK(StringListKeySize==
	sizeof(StringListValues)/sizeof(StringListValues[0]));
  // ----- first parse "addresses" subsection:
  if(!section->find(ADDRESS_SUBSECTION, addresses))
    {
      CHECK(false); // currently we are very critical here in the debug version
      return InternError; // no subsection called "addresses"
    }
  CHECK(addresses!=0);
  for(pos=addresses->sectionsBegin(); pos!=addresses->sectionsEnd(); ++pos)
    {
      if(!addresses->find((*pos).first, addressSection))
	{
	  CHECK(false); // currently we are very critical ...
	  return InternError; // no section we have an iterator for?
	}
      keys=addressSection->getKeys();
      CHECK(keys!=0);
      address=addressDummy; // clean it up
      if(makeAddressFromMap(keys, address)==AddressBook::NoError)
	{
	  // ----- add the address to the list of addresses:
	  temp.addresses.push_back(address);
	} else {
	  debug("AddressBook::makeEntryFromSection: cannot find all fields "
		"in an address subsection.");
	  CHECK(false); // meanwhile, not in release
	}
    }
  CHECK(temp.addresses.size()==addresses->noOfSections());
  // ----- now parse all other fields directly:
  keys=section->getKeys();
  CHECK(keys!=0);
  for(count=0; count<StringKeySize; ++count)
    {
      if(!keys->get(StringKeys[count], *StringValues[count]))
	{
	  debug("AddressBook::makeEntryFromSection: error: could not get "
		"value for key %s.", (const char*)StringKeys[count]);
	  CHECK(false); // kill in debug version
	}
    }
  for(count=0; count<StringListKeySize; ++count)
    {
      if(!keys->get(StringListKeys[count], *StringListValues[count]))
	{
	  debug("AddressBook::makeEntryFromSection: error: could not get "
		"value for key %s.", (const char*)StringListKeys[count]);
	  CHECK(false); // kill in debug version
	}
    }
  CHECK(temp.telephone.count()%2==0); // must be an even number
  // ----- finally get the birthday:
  keys->get("birthday", temp.birthday); // this may return false (no date)
  // -----
  entry=temp;
  CHECK(entry.addresses.size()==temp.addresses.size());
  return NoError;
  // ###########################################################################
}

AddressBook::ErrorCode
AddressBook::makeAddressFromMap(KeyValueMap* keys, Entry::Address& address)
{
  REQUIRE(keys!=0);
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
  CHECK(Size==sizeof(strings)/sizeof(strings[0]));
  int count;
  // -----
  for(count=0; count<Size; ++count)
    {
      if(!keys->get(Keys[count], *strings[count]))
	{
	  debug("AddressBook::makeAddressFromSection: error: could not get "
		"value for key %s.", (const char*)Keys[count]);
	}
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
  CHECK(section.empty());
  // ----- first create "addresses" subsection:
  if(!section.add(ADDRESS_SUBSECTION))
    {
      kDebugInfo("AddressBook::makeSectionFromEntry: cannot create %s "
		 "subsection.", ADDRESS_SUBSECTION);
      CHECK(false); // currently we are very critical here in the debug version
      return InternError;
    }
  if(!section.find(ADDRESS_SUBSECTION, addresses))
    {
      kDebugInfo("AddressBook::makeSectionFromEntry: cannot get new section.");
      CHECK(false); // currently we are very critical here in the debug version
      return InternError;
    }
  // ----- now insert addresses:
  for(addPos=entry.addresses.begin(); addPos!=entry.addresses.end(); ++addPos)
    {
      ++count;
      key.setNum(count);
      CHECK(key.toInt(0)==count);
      if(!addresses->add(key))
	{
	  kDebugInfo("AddressBook::makeSectionFromEntry: cannot create address "
		     "subsection %s.", (const char*)key);
	  CHECK(false); // currently we are very critical...
	  return InternError;
	}
      if(!addresses->find(key, address))
	{
	  kDebugInfo("AddressBook::makeSectionFromEntry: cannot get new "
		     "subsection.");
	  CHECK(false); // currently we are very critical...
	  return InternError;
	}
      CHECK(address!=0);
      keys=address->getKeys();
      CHECK(keys->empty()); // semantical check
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
	  kDebugInfo("AddressBook::makeSectionFromEntry: cannot completely "
		     "create address subsection.");
	  CHECK(false); // meanwhile, not in release
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
     !keys->insert("user4", entry.user4))
    {
      kDebugInfo("AddressBook::makeEntryFromSection: cannot insert all fields of the "
	"entry into its section!");
      CHECK(false); // currently we are very critical here in the debug version
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
  // const QString KabPublicDir=KApplication::kde_datadir()+"/kab/";
  // const QString KabPublicDir="./"; // WORK_TO_DO: remove this line!
  const QString KabTemplateFile=locate("data", "kab/template.kab");
  debug("AddressBook::createNew: template file is \"%s\".",
	(const char*)KabTemplateFile.utf8());
  // KabPublicDir+KAB_TEMPLATEFILE;
  QConfigDB db;
  // -----
  if(KabTemplateFile.isEmpty()
     || !db.setFileName(KabTemplateFile, true, true))
    {
      KMessageBox::error(this,
	 i18n("Cannot find kab's template file.\n"
	      "You cannot create new files."),
	 i18n("File error"));
      return InternError;
    }
  if(!db.load())
    {
      KMessageBox::error(this,
	 i18n("Cannot read kab's template file.\n"
	      "You cannot create new files."),
	 i18n("Format error"));

      return InternError;
    }
  if(!db.setFileName(filename, false, false))
    {
      KMessageBox::error(this,
	 i18n("Cannot create the file\n\"")
	 +filename+"\"\n"+
	 i18n("Could not create the new file."),
		 i18n("File error"));
      return PermDenied;
    }
  if(!db.save())
    {
      KMessageBox::error(this,
	 i18n("Cannot save the file\n\"")
	 +filename+"\"\n"+
	 i18n("Could not create the new file."),
		 i18n("File error"));
      return InternError;
    }
  // -----
  return NoError;
  // ###########################################################################
}

/*
  AddressBook::ErrorCode
  AddressBook::configureFile()
  {
  // ###########################################################################
  DialogBase base(this);
  KabFileConfigWidget conf(data, &base, 0);
  // -----
  base.setCaption(i18n("kab: Configure this file"));
  base.enableButtonApply(false);
  base.setMainWidget(&conf);
  base.resize(base.minimumSize());
  if(base.exec())
    { // ----- first "save" settings into the QConfigDB object:
    if(conf.saveSettings()==NoError)
    { // ----- ...then save it to the file:
    return save();
    } else {
    return PermDenied;
    }
    } else {
    emit(setStatus(i18n("Rejected.")));
    return Rejected;
    }
    // ###########################################################################
    }

bool
AddressBook::getBackgroundEnabled()
{
  // ###########################################################################
  return backgroundEnabled;
  // ###########################################################################
}

void
AddressBook::setBackgroundEnabled(bool state)
{
  // ###########################################################################
  backgroundEnabled=state;
  // ###########################################################################
}

void
AddressBook::setBackground(const QImage& image)
{
  // ###########################################################################
  *background=image;
  // ###########################################################################
}

bool
AddressBook::getInteractiveMode()
{
  // ###########################################################################
  return urlsEnabled;
  // ###########################################################################
}

void
AddressBook::setInteractiveMode(bool state)
{
  // ###########################################################################
  urlsEnabled=state;
  urlEmail->setFloat(state);
  urlEmail->setUnderline(state);
  // urlEmail->setEnabled(state;
  urlHomepage->setFloat(state);
  urlHomepage->setUnderline(state);
  // urlHomepage->setEnabled(state);
  // ###########################################################################
}

void
AddressBook::appearanceChanged()
{
  // ###########################################################################
  repaint(false);
  // ###########################################################################
}
*/

AddressBook::ErrorCode
AddressBook::createConfigFile()
{
  // ###########################################################################
  // const QString KabPublicDir=KApplication::kde_datadir()+"/kab/";
  // const QString KabPublicDir=globalDir(); // WORK_TO_DO: remove this line!
  // KabPublicDir+"/"+KAB_CONFIGTEMPLATE;
  const QString ConfigTemplateFile=locate("data", "kab/template.config");
  debug("AddressBook::createConfigFile: config template file is \"%s\".",
	(const char*)ConfigTemplateFile.utf8());
  const QString filename=baseDir()+"/"+STD_CONFIGFILENAME;
  QConfigDB db;
  // -----
  if(ConfigTemplateFile.isEmpty()
     || !db.setFileName(ConfigTemplateFile, true, true))
    {
      KMessageBox::error(this,
	 i18n("Cannot find kab's configuration template file.\n"
	      "kab cannot be configured."),
	 i18n("File error"));

      return InternError;
    }
  if(!db.load())
    {
      KMessageBox::error(this,
	 i18n("Cannot read kab's configuration template file.\n"
	      "kab cannot be configured."),
	 i18n("File error"));
      return InternError;
    }
  if(!db.setFileName(filename, false, false))
    {
      KMessageBox::error(this,
	 i18n("Cannot create the file\n\"")
	 +filename+"\"\n"+
	 i18n("Could not create the new configuration file."),
	 i18n("File error"));
      return PermDenied;
    }
  if(!db.save())
    {
      KMessageBox::error(this,
	 i18n("Cannot save the file\n\"")
	 +filename+"\"\n"+
	 i18n("Could not create the new configuration file."),
	 i18n("File error"));
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
  QString file;
  // -----
  file=baseDir()+"/"+STD_CONFIGFILENAME;
  if(config->setFileName(file, true, true))
    {
      if(config->load())
	{
	  return NoError;
	} else {
	  KMessageBox::information(this,
	     i18n("Cannot load kab's local configuration file,\n"
		  "possibly there is a formation error.\n"
		  "kab cannot be configured."),
	     i18n("File error"));
	  return InternError;
	}
    } else {
      KMessageBox::information(this,
	 i18n("Cannot find kab's local configuration file.\n"
	      "kab cannot be configured."),
	 i18n("File error"));
      return NoSuchFile;
    }
  // ###########################################################################
}

/*
  void
  AddressBook::mailURLClicked(const char* c)
  {
  // ###########################################################################
  emit(mail(c));
  // ###########################################################################
  }

  void
  AddressBook::homeURLClicked(const char* c)
  {
  // ###########################################################################
  emit(browse(c));
  // ###########################################################################
  }

  void
  AddressBook::addressSelected(int index)
  {
  // ###########################################################################
  CHECK(index>=0 && (unsigned)index<current.addresses.size());
  currentAddress=index;
  repaint(false);
  // ###########################################################################
  }
*/

AddressBook::ErrorCode
AddressBook::makeVCardFromEntry(const Entry&, QString)
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
AddressBook::getStandardFilename()
{
  // ###########################################################################
  QString temp;
  // -----
  temp=baseDir();
  temp+=QString("/")+STD_USERFILENAME;
  return temp;
  // ###########################################################################
}

QString
AddressBook::baseDir()
{
  // ###########################################################################
  QString dir;
  // -----
  dir=KGlobal::dirs()->saveLocation("data", KAB_LOCALDIR, false);
  if(dir==QString::null)
    {
      KMessageBox::information(this,
	 i18n("The directory where user specific data for the\n"
	      "addressbook application is stored does not exist.\n"
	      "The program will try to create\n         \"")
	 +dir+"\"\n"+
	 i18n("and store all local files in this directory."),
	i18n("First usage"));
      // ----- create the directory and check success:
      dir=KGlobal::dirs()->saveLocation("data", KAB_LOCALDIR, true);
      if(dir==QString::null)
	{
	  KMessageBox::information(this,
		i18n("The directory could not be created.\n"
		  "Probably you do not have used KDE before, so\n"
		  "you do not have a local \".kde\" directory\n"
		  "in your home directory. Run the KDE filemanager\n"
		  "kfm once to automatically create it.\n"
		  "kab will continue, but no file will be loaded."),
		i18n("Error creating directory"));
	  state=PermDenied;
	  return QString::null;
	} else { // ----- just for interactivity:
	  CHECK(access(dir.ascii(), X_OK | F_OK)==0);
	  KMessageBox::information(this,
		i18n("The directory has been created."),
		i18n("Directory created"));
	  return dir;
	}
    } else {
      return dir;
    }
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

