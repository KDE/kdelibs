/* This file defines the configuration database. It uses the
 * KeyValueMap class for handling formatted files with 
 * key-value pairs.
 *
 * the Configuration Database library
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class,
 *             Nana (only for debugging)
 * $Revision$
 */

#pragma implementation

#include "configDB.h"
#include "debug.h"
#include <fstream.h>
extern "C" {
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
	   }

list<string> ConfigDB::LockFiles;

ConfigDB::ConfigDB()
  : readonly(true),
    locked(false),
    mtime(0)
{
  // ########################################################
  // ########################################################
}

bool ConfigDB::invariant()
{
  // ########################################################
  if(!isRO() && !locked)
    {
      L("ConfigDB::invariant: DB has r/w state, "
	"but file is not locked.\n");
      return false;
    }
  return true;
  // ########################################################
}

bool ConfigDB::setFileName
(
 const string& filename_, 
 bool mustexist,
 bool readonly_
)
{
  ID(bool GUARD=true);
  // ########################################################  
  LG(GUARD, "ConfigDB::setFileName: "
     "setting filename to \"%s\"%s.\n",
     filename_.c_str(), readonly_ ? " (read only)" : "");
  // ----- remove previous lock:
  if(locked)
    {
      if(!unlock())
	{
	  LG(GUARD, "ConfigDB::setFileName: cannot release "
	     "previous lock.\n");
	  return false;
	}
    }
  if(mustexist)
    {
      if(access(filename_.c_str(), 
		readonly_==true ? R_OK : W_OK | R_OK)==0)
	{ 
	  LG(GUARD, "ConfigDB::setFileName: "
	     "permission granted, file exists.\n");
	  if(!readonly_)
	    { //       we need r/w access:
	      if(lock(filename_))
		{
		  locked=true;
		} else {
		  LG(GUARD, "ConfigDB::setFileName: "
		     "could not lock the file.\n");
		  return false;
		}
	    }
	  readonly=readonly_;
	  filename=filename_;
	  storeFileAge(); CHECK(storeFileAge());
	  return true;
	} else {
	  L("ConfigDB::setFileName: permission denied, "
	    "filename not set.\n                       "
	    "(hint: file must exist, but it does not)\n");
	  return false;
	}
    } else {
      if(access(filename_.c_str(), F_OK)==0)
	{
	  LG(GUARD, "ConfigDB::setFileName: file exists.\n");
	  if(access(filename_.c_str(), W_OK | R_OK)==0)
	    {
	      LG(GUARD, "ConfigDB::setFileName: "
		 "permission granted.\n");
	      if(!readonly_)
		{ //       we need r/w access:
		  if(lock(filename_))
		    {
		      locked=true;
		    } else {
		      LG(GUARD, "ConfigDB::setFileName: "
			 "could not lock the file.\n");
		      return false;
		    }
		}
	      readonly=readonly_;
	      filename=filename_;
	      storeFileAge(); CHECK(storeFileAge());	      
	      return true;
	    } else {
	      LG(GUARD, "ConfigDB::setFileName: "
		 "permission denied, filename not set.\n");
	      return false;	      
	    }
	} else {
	  LG(GUARD, "ConfigDB::setFileName: "
	     "permission granted, new file.\n");
	  readonly=readonly_;
	  filename=filename_;
	  if(!readonly)
	    {
	      if(!lock())
		{
		  LG(GUARD, "ConfigDB::setFileName: "
		     "could not lock the file.\n");
		  return false;
		}
	    }
	  storeFileAge();
	  return true;
	}
    }
  // ########################################################
}

string ConfigDB::fileName()
{
  // ########################################################
  return filename;
  // ########################################################
}

bool ConfigDB::save(const char* header)
{
  ID(bool GUARD=false);
  REQUIRE(!filename.empty());
  // ########################################################
  LG(GUARD, "ConfigDB::save: saving database -->%s<--.\n",
     filename.c_str());
  // -----
  if(fileChanged())
    {
      LG(GUARD, "ConfigDB::save: file is newer, "
	 "not saving.\n");
      return false;
    }
  if(!readonly)
    {
      ofstream file(filename.c_str());
      if(!file.good())
	{
	  L("ConfigDB::save: "
	    "error opening file \"%s\" for writing.\n",
	    filename.c_str());
	  return false;
	} else {
	  if(header!=0)
	    {
	      file << "# " << header << endl;
	    }
	  file << '#' << " [File created by ConfigDB object " 
	       << version() 
	       << "]" << endl;
	  if(!top.save(file)) // traverse tree
	    {
	      LG(GUARD, "ConfigDB::save: "
		 "error saving subsections.\n");
	    }
	  storeFileAge(); CHECK(storeFileAge());
	  return true;
	}
    } else {
      return false;
    }
  // ########################################################
}

bool ConfigDB::load()
{
  ID(bool GUARD=true);
  REQUIRE(!filename.empty());
  // ########################################################  
  ifstream file(filename.c_str());
  string line;
  // -----
  if(!file.good())
    {
      L("ConfigDB::load: "
	"error opening file \"%s\" for reading.\n",
	filename.c_str());
      return false;
    } else {
      LG(GUARD, "ConfigDB::load: file access OK.\n");
      bool rc=top.readSection(file, false);
      changed();
      storeFileAge(); CHECK(storeFileAge());
      return rc;
    }
  // ########################################################
}


bool ConfigDB::get
(const list<string>& key, 
 KeyValueMap*& map)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "ConfigDB::get: trying to get keys ... ");
  Section* section=&top;
  list<string>::const_iterator pos;
  // -----
  if(key.empty()) 
    {
      LG(GUARD, "\nConfigDB::get: path is empty, returning "
	 "toplevel section.\n");
      map=top.getKeys();
      return true;
    }
  for(pos=key.begin(); pos!=key.end(); pos++)
    {
      if(!section->find(*pos, section))
	{
	  LG(GUARD, "failed,\n"
	     "               at least the element \"%s\" of "
	     "the key-list is not declared.\n", 
	     (*pos).c_str());
	  return false;
	}
    }
  map=section->getKeys();
  LG(GUARD, "success.\n");
  return true;
  // ########################################################
}

KeyValueMap* ConfigDB::get()
{
  // ########################################################
  return top.getKeys();
  // ########################################################
}

bool ConfigDB::createSection(const list<string>& key)
{ // WORK_TO_DO: report errors (much) better!
  // (by now it is only possible to report ANY failure)
  /*
    Behaviour: 
    - returns false if an error occured
    - creates ALL sections needed to fulfill the request
  */
  REQUIRE(!key.empty());
  // ########################################################  
  Section* section=&top;
  unsigned int index;
  list<string>::const_iterator pos;
  Section* thenewone;
  // -----
  pos=key.begin();
  for(index=0; index<key.size()-1; index++)
    {
      if(!section->find(*pos, section))
	{ // this section is not declared
	  Section* temp=new Section;
	  if(section->add(*pos, temp))
	    {
	      section=temp; 
	    } else {
	      CHECK(false); // this may not happen
	      delete temp;
	    }
	}
      ++pos;
    }
  // pos now points to the last element of key
  // and section to the parent of the section 
  // that will be inserted
  thenewone=new Section;
  section->add(*pos, thenewone);
  // this overrides section (!!):
  CHECK(section->find(*pos, section)); 
  return true; // missing error report! WORK_TO_DO
  // ########################################################
}

bool ConfigDB::clear()
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "ConfigDB::clear: clearing database.\n");
  bool rc=top.clear();
  changed();
  LG(GUARD, "ConfigDB::clear: done.\n");
  return rc;
  // ########################################################
}

bool ConfigDB::createSection(const string& desc)
{
  // ########################################################
  return createSection(stringToKeylist(desc));
  // ########################################################
}

bool ConfigDB::get(const string& key, KeyValueMap*& map)
{
  // ########################################################
  return get(stringToKeylist(key), map);
  // ########################################################
}

list<string> ConfigDB::stringToKeylist(const string& desc)
{
  ID(bool GUARD=false);
  LG(GUARD, "ConfigDB::stringToKeylist: parsing path %s.\n",
     desc.c_str());
  // ########################################################
  list<string> key;
  string::size_type first, second;
  string temp;
  first=0;
  // -----
  if(desc.empty())
    {
      LG(GUARD, "ConfigDB::stringToKeylist: "
	 "path is empty.\n");
      return key;
    }
  for(;;) 
    {
      second=desc.find('/', first);
      if(second==string::npos)
	{
	  if(first<desc.size())
	    {
	      temp=desc.substr(first);
	      LG(GUARD, "ConfigDB::stringToKeylist: "
		 "found last part %s.\n", temp.c_str());
	      key.push_back(temp);
	    }
	  break;
	}
      temp=desc.substr(first, second-first);
      LG(GUARD, "ConfigDB::stringToKeylist: "
	 "found part %s.\n", temp.c_str());
      key.push_back(temp);
      first=second+1;
    }
  LG(GUARD, "ConfigDB::stringToKeylist: done.\n");
  return key;
  // ########################################################
}

bool ConfigDB::get(const string& key, Section*& section)
{
  // ########################################################
  return get(stringToKeylist(key), section);
  // ########################################################
}

bool ConfigDB::get
(const list<string>& key, 
 Section*& section)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "ConfigDB::get: searching section ... ");
  Section* temp=&top;
  list<string>::const_iterator pos;
  for(pos=key.begin(); pos!=key.end(); pos++)
    {
      if(!temp->find(*pos, temp))
	{
	  LG(GUARD, "failure, no such section.\n");
	  return false;
	}
    }
  section=temp; 
  LG(GUARD, "success, section found.\n");
  return true;
  // ########################################################
}

bool ConfigDB::isRO()
{
  // ########################################################
  return readonly;
  // ########################################################
}

int ConfigDB::IsLocked(const string& file)
{
  ID(bool GUARD=true);
  // ########################################################
  string lockfile=file+(string)".lock";
  int pid=-1;
  // -----
  if(access(lockfile.c_str(), F_OK)==0)
    {
      LG(GUARD, "ConfigDB::IsLocked: the file\n        %s\n"
	 "                    has a lockfile.\n", 
	 file.c_str());
      ifstream stream(lockfile.c_str());
      if(!stream.good())
	{
	  LG(GUARD, "ConfigDB::IsLocked: cannot open "
	     "the lockfile.\n");
	  return -1;
	}
      stream >> pid;
      if(pid==-1)
	{
	  LG(GUARD, "ConfigDB::IsLocked: the file does not "
	     "contain the ID\n        of the process that "
	     "created it.\n");
	  return -1;
	}
      return pid;      
    } else {
      LG(GUARD, "ConfigDB::IsLocked: the file\n        %s "
	 "has no lockfile.\n", file.c_str());
      return 0;

    }
  // ########################################################
}

bool ConfigDB::lock()
{
  ID(bool GUARD=true);
  REQUIRE(!filename.empty());
  // ########################################################
  if(locked)
    {
      LG(GUARD, "ConfigDB::lock (current file):"
	 " file is already locked by this object.\n");
      return false;
    }
  if(lock(filename))
    {
      locked=true;
      return true;
    } else {
      return false;
    }
  // ########################################################
}

bool ConfigDB::lock(const string& file)
{
  ID(bool GUARD=true);
  // ########################################################
  LG(GUARD, "ConfigDB::lock: locking the file %s.\n",
     file.c_str());
  string lockfile=file+".lock";
  // -----
  if(access(lockfile.c_str(), F_OK)==0)
    {
      LG(GUARD, "ConfigDB::lock: the file is locked by "
	 "another process.\n");
      return false;
    } else {
      ofstream stream(lockfile.c_str());
      if(!stream.good())
	{
	  LG(GUARD, "ConfigDB::lock: unable to create "
	     "lockfile.\n");
	  return false;
	}
      stream << getpid() << endl;
    }
  LockFiles.push_back(lockfile);
  return true;
  // ########################################################  
}

bool ConfigDB::unlock()
{
  ID(bool GUARD=true);
  REQUIRE(!filename.empty());
  // ########################################################
  LG(GUARD, "ConfigDB::unlock: unlocking the file %s.\n",
     filename.c_str());
  string lockfile=filename+".lock";
  list<string>::iterator pos;
  // -----
  if(!locked)
    {
      LG(GUARD, "ConfigDB::unlock: this app did not lock the"
	 " file!\n");
      return false;
    }
  if(access(lockfile.c_str(), F_OK | W_OK)==0)
    {
      if(::remove(lockfile.c_str())==0)
	{
	  LG(GUARD, "ConfigDB::unlock: lockfile deleted.\n");
	  for(pos=LockFiles.begin(); 
	      pos!=LockFiles.end(); pos++)
	    {
	      if((*pos)==lockfile) break;
	    }
	  if(pos!=LockFiles.end())
	    {
	      LockFiles.erase(pos); --pos;
	    } else {
	      L("ConfigDB::unlock: file not mentioned in "
		"lockfile list.\n");
	    }
	  locked=false;
	  return true;
	} else {
	  LG(GUARD, "ConfigDB::unlock: unable to delete "
	     "lockfile.\n");
	  return false;
	}
    } else {
      LG(GUARD, "ConfigDB::unlock: the file is not locked "
	 "or permission has been denied.\n");
      return false;
    }
  // ########################################################
}

void ConfigDB::CleanLockFiles(int)
{
  ID(bool GUARD=true);
  // ########################################################
  list<string>::iterator pos;
  // -----
  LG(GUARD, "ConfigDB::CleanLockFiles: "
     "removing %i remaining lockfiles.\n", LockFiles.size());
  for(pos=LockFiles.begin(); pos!=LockFiles.end(); pos++)
    {
      if(::remove((*pos).c_str())==0)
	{
	  LG(GUARD, "                          %s "
	     "removed.\n", (*pos).c_str());
	  LockFiles.erase(pos); --pos;
	} else {
	  LG(GUARD, "                          could not "
	     "remove  %s.\n", (*pos).c_str());
	}
    }
  LG(GUARD, "ConfigDB::CleanLockFiles: done.\n");
  // ########################################################
}

bool ConfigDB::CheckLockFile(const string& file)
{
  ID(bool GUARD=true);
  // ########################################################
  LG(GUARD, "ConfigDB::CheckLockFile: called.\n");
  int pid;
  // -----
  pid=IsLocked(file);
  if(pid==0)
    {
      LG(GUARD, "ConfigDB::CheckLockFile: the file is not "
	 "locked.\n");
      return false;
    }
  if(pid>0)
    {
      if(kill(pid, 0)!=0) 
	{ // ----- no such process, 
	  //       we may remove the lockfile
	  return false;
	}
    }
  if(pid<0)
    {
      LG(GUARD, "ConfigDB::CheckLockFile: the file has not "
	 "been created by ConfigDB::lock.\n");
    }
  // ----- check system time and creation time of lockfile:
  // not implemented
  LG(GUARD, "ConfigDB::CheckLockFile: done.\n");
  return true;
  // ########################################################
}
  
bool ConfigDB::fileChanged()
{
  ID(bool GUARD=true);
  // LG(GUARD, "ConfigDB::fileChanged: called.\n");
  // ########################################################
  struct stat s;
  // -----
  if(filename.empty()) 
    { // ----- false, as file does not exist and thus may 
      //       be stored anyway
      LG(GUARD, "ConfigDB::fileChanged: no filename.\n");
      return false;
    }
  if(stat(filename.c_str(), &s)==0)
    {
      if(s.st_mtime>mtime)
	{
	  LG(GUARD, "ConfigDB::fileChanged: file has been "
	     "changed.\n");
	  return true;
	} else {
	  return false;
	}
    } else {
      LG(GUARD, "ConfigDB::save: could not stat file, "
	 "file does not exist.\n");
      if(mtime==0)
	{ // the file did never exist for us:
	  return false; // ... so it has not changed
	} else { // it existed, and now it does no more
	  return true;
	}
      //       // error stating an existing file? very strange!
      //       CHECK(false);
    }
  // return false; // should be unreachable
  // ########################################################
}
  
bool ConfigDB::storeFileAge()
{
  ID(bool GUARD=false);
  REQUIRE(!filename.empty());
  // ########################################################
  struct stat s;
  // -----
  LG(GUARD, "ConfigDB::storeFileAge: called.\n");
  if(stat(filename.c_str(), &s)==0)
    {
      mtime=s.st_mtime;
      return true;
    } else {
      LG(GUARD, "ConfigDB::save: could not stat file.\n");
      mtime=0;
      return false;
    }  
  // ########################################################
}  

void ConfigDB::changed()
{
  // ########################################################
  // ########################################################
}
