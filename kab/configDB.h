/*  -*- C++ -*-
 * This file declares the configuration database. It uses the
 * KeyValueMap class which is an own project for handling
 * formatted files with key-value pairs.
 *
 * the Configuration Database library
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@hamburg.netsurf.de>
 *                          <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class
 * $Revision$
 */
#ifndef CONFIGDB_H_INCLUDED
#define CONFIGDB_H_INCLUDED

// this is for g++: (needed for -fexternal-templates)
#pragma interface

#include "stl_headers.h"
#include "keyvaluemap.h"
#include "sections.h"

extern "C" {
#include <sys/stat.h>
#include <unistd.h>
	   }

class ConfigDB
{
protected:
  Section top;
public: 
  // constructor(s) and destructor:
  ConfigDB();
  virtual ~ConfigDB() { if(locked) unlock(); }
  // methods for data manipulation (manipulate KeyValueMap directly)
  // a ConfigDB object NEVER manipulates the key-value-pairs itself
  bool get(const string& key, KeyValueMap*& map);
  bool get(const list<string>& key, KeyValueMap*& map);
  // get the address of the specified Section object
  // (should this be public? 
  // I think so - it is needed for "for all" -operations)
  // -> never call "delete" (1) for the section pointers or 
  // perform other operations that may be hazardous!
  // (1) Not all of the Section objects are created using "new"!
  // returns false if the section has not been declared
  bool get(const string& key, Section*&);
  bool get(const list<string>& key, Section*&);
  KeyValueMap* get(); // get keys of top level section
  // methods for creating sections
  bool createSection(const string& key);
  bool createSection(const list<string>& key);
  // methods for file handling
  bool load();
  // header will be the comment in the first line of the file
  bool save(const char* header=0); 
  bool setFileName(const string& name,
		   bool mustexist=true,
		   bool readonly_=false);
  // this method returns true if the file has been changed on disk
  // after the last reading or saving
  bool fileChanged();
  // call this to set the intern time mark of the file access:
  bool storeFileAge();
  // returns the current filename
  string fileName(); 
  // returns if the current file is opened or will be opened read-only
  bool isRO(); 
  // clear all contents of the database:
  bool clear();
  // this method will notify changes of the database contents
  // the notification is needed if pointers (iterators) are stored 
  // outside the database object because they will need an update then
  // (it is empty in this definition)
  virtual void changed(); 
  static const char* version() 
    { return "0.6 (devel version) $Revision$"; }
  // this method checks wether the given file is locked:
  // returns zero if not, 
  // a number > zero is the pid of the process locking the file,
  // a number < zero reports an error and the file is locked
  static int IsLocked(const string& filename_);
  // this method checks an existing lockfile for the given file,
  // that means, it gets the name of the DATA file as the parameter
  // as lockfiles often remain when a program crashes, this function 
  // checks certain conditions that show that a lockfile is not in 
  // use anymore, these are:
  // ° there is no process with the pid in the lockfile,
  // ° the systems boot-time is after the creation of the lockfile.
  // the problem is that, if there is a process with the pid we have, 
  // this does not need to be the process that created the lockfile
  // the method returns only false if it is shure that no such process 
  // exists
  // returns false if the lockfile exists and is definitely stale
  //         or there is none,
  // returns true if the lockfile seems to be valid 
  static bool CheckLockFile(const string& filename);
  /* In this list, all created lockfiles are notified. The static 
   * method CleanLockFiles removes all files in this list when called.
   * Thus this function should be installed as a handler for SIGINT, 
   * SIGQUIT, SIGKILL, SIGTERM and other program abortion signals or
   * should be called by the respective handlers. 
   * The list contains the names of the lockfiles, not of the files 
   * itselfes.
   */
  static list<string> LockFiles;
  static void CleanLockFiles(int);
  /* lock and unlock files: locking is currently advisory locking,
   * by creating a file <filename>.lock. ConfigDB-objects will reject
   * opening a file for reading and writing if a lockfile for the 
   * filename exists.
   * I plan do implement this using mandatory locking in future, at 
   * least for Linux.
   */
  bool lock();
  bool unlock();
protected:
  list<string> stringToKeylist(const string&);
  // data elements
  string filename; 
  bool readonly; 
  bool locked;
  // to recognize file changes, 0 if unknown (new file):
  time_t mtime; // stores modification time of last check
  // for internal use: lock other files:
  bool lock(const string& file);
  // debugging aid: called from REQUIRE and ENSURE macros:
  bool invariant();
};

#endif // CONFIGDB_H_INCLUDED

