/* -*- C++ -*-
 * This test creates a database of adresses and 
 * configuration values. It also reports any errors
 * that happen using the database functionality.
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
  
#ifndef ADDRESSBOOK_H
#define ADDRESSBOOK_H

#include "configDB.h"
#include <qdatetime.h>

// -----------------------------
// this will be incremented when kab's file format changes
// significantly
#if defined KAB_FILE_FORMAT
#undef KAB_FILE_FORMAT
#endif
/*
  0: all format before the email list was implemented
  1: format enhanced for unlimited number of email addresses
  2: format enhanced by more address fields
*/
#define KAB_FILE_FORMAT 2
// -----------------------------

class AddressBook : public ConfigDB
{
public:
  /** An object of the class {\em Entry} represents one entry.
    * More fields can easily be added: add a new string, find a
    * name for it in the entry-sections of the database that is 
    * not used until now (automatically done by nextAvailEntryKey),
    * and change the {\tt makeEntryFromSection}-,
    * the {\tt add(..)}-, the {\tt change}- and the 
    * nameOfField-methods.
    */
  class Entry {
  public:
    // changed on April 4 1998
    // (added talk-address-list)
    // changed on June 9 1998
    // (added keywords list)
    // ----------------------------------
    string name;
    string firstname;
    string additionalName;
    string namePrefix;
    string fn; // formatted name 
    string comment;
    string org; // organization -
    string orgUnit; 
    string orgSubUnit; 
    string title; 
    string role; 
    // QPixmap logo;
    QDate birthday; // now implemented
    list<string> talk; // list of talk addresses
    list<string> keywords; // list of form-free keywords
    // string sound; // base64 coded wav - file
    // string agent; // contains a complete vCard possibly?
    string deliveryLabel; // missing in implementation
    // Attention API users: 
    // The email fields will be changed to an unlimited list of
    // email addresses and remarks, see list<string> emails below.
    // Do not rely on the availablility of the three string 
    // members, I think this is much to inflexible.
    // Currently, the emails-list will be empty, but not for long!
    string email; 
    string email2; 
    string email3; 
    // This list contains the email addresses.
    list<string> emails;
    string address; 
    string town; 
    // new fields, hint from Preston Brown:
    string zip; // zip or postal code
    string state; // for state or province
    string country; // for federal states
    // -----
    string telephone; 
    string fax; 
    string modem; 
    string URL; 
  };
  /** An aggregat of all declared fields:
    */
  static const char* Fields[];
  /** The number of elements in Fields.
    */
  static const int NoOfFields;
protected:
  /** This method will be called by all REQUIRE and ENSURE statements. It
    * returns {\tt true} if no inconsistencies regarding the internal 
    * data structures can be found.
    * You need to be careful not to create endless looping here!
    * (See the Nana documentation, "eiffel.h", for details.)
    */
  bool invariant();
  /** The DB uses another map to access the entries in different orders.
    * This map has always the same number of entries as there are subsections
    * in the entry-section. The map contains strings describing the order of 
    * the entries (which can be reordered) and according to each string the 
    * {\em subsection name} of the entry in the entry-section. The 
    * {\tt current}-iterator is used to have one entry that is currently 
    * displayed.
    */
  typedef map<string, string, less<string> > StringStringMap;
  StringStringMap entries;
  StringStringMap::iterator current;
  bool isFirstEntry();
  bool isLastEntry();
  /** This method sets current to the element in entries at the given index.
    */
  bool setCurrent(int index); 
  /** This method sets the current key to the one with the given key 
    * in the ENTRIES-section. This is reverse to the method before!
    */
  bool setCurrent(const string& key); 
  /** This method creates the mirror map by inserting an entry for each 
    * (DB) entry in it. It sets current to the entry with the key <key>.
    */
  void updateEntriesMap(string key="");
  /// Convenience methods to retrieve frequently needed pointers and numbers.
  Section* configSection();
  Section* entrySection();
  /** See the implementation file addressbook.cc for the contents of the 
    * constants.
    */
  static const char* ConfigSection;
  static const char* EntrySection;
  static const char* DBFileName;
  static const char* Defaults[];
  static bool initialized;
  /** This converts a subsection of the entry-section to an object of the 
    * {\em Entry}-class. It erases alll fields in the referenced entry
    * even if they are empty in the section.
    */
  bool makeEntryFromSection(Section&, Entry&);
  /// Get a verbose name (human readable) for an entry:
  string getName(const string& key); 
  /// internal!
  string nextAvailEntryKey();  
  // creates a new database:
  bool createNew(string filename);
public:
  /// This methods retrieve the current entry.
  bool currentEntry(Section**);
  bool currentEntry(Entry&);
  /** This method returns the entry that has the key given in the 
    * reference. It returns false if there is no entry with this
    * key.
    */
  bool getEntry(const string& key, Entry& entry);
  /** Using getEntries(..), the caller will get a copy of all entries in the 
    *  database. This might seem unneeded, but the address database can be 
    *  used by multiple instances of the kab API at the same time, so that, 
    *  if the programmer wants, for example, print a letter header for all 
    *  persons, the database might change during the operation. That is why 
    *  she can retrieve the whole database in one operation. 
    *  It is required that the referenced list is empty.
    * @short Retrieves all entries out of the database.
    * @params entries Reference to a list of entries.
    * @return False on error or true.
    */
  bool getEntries(list<AddressBook::Entry>& entries);
  /** Returns the key of the current entry. The string is empty if
    * there are no entries.
    */
  string currentEntry();
  /** Returns the number of entries in the database.
    */
  unsigned int noOfEntries();
  /** The following methods are used to navigate through the entries.
    * They all change the current-iterator.
    */
  bool first();
  bool previous();
  bool next();
  bool last();
  // managing entries
  // add an entry, return the new key for further operations:
  bool add(const Entry&, string& key);
  inline bool add(string& key); // empty entry 
  /** Change the current entry.
    */
  bool change(const Entry&);
  /** Change the entry with the given key.
    */
  bool change(const string& key, const Entry&);
  /** Remove the current entry.
    */
  bool remove();
  /** Remove the  entry with the given key. Returns false if 
    * there is no entry with this key.
    */
  bool remove(const string& key);
  // some convenience functions with "soft" information
  /** This method is intended for printing and exporting purposes.
    * It returns the localized literal name of a data field (for
    * example, "name" (en) or "Name" (de) for the field "name")
    * in the second reference.
    * It returns false if the field descriptor (param 0) is unknown.
    */
  bool nameOfField(const string& field, string& name);
  /** This method returns the literal name for the entry, 
    * containing either the formatted name (if given) or a 
    * combination of the first, additional and last name. 
    * The name is returned in text.
    * If reverse is false, the text looks like
    *    firstname (add. name) last name,
    * if it is true, 
    +    last name, first name (add. name).
    * If initials is true, the text contains only initials:
    *    f. a. name [with reverse==false] or
    *    name, f. a. [with reverse==true].
    * If there is no entry with this key, the method returns false.
    */
  bool literalName(const string& key, string& text,
		   bool reverse=false, bool initials=false);
  /** This method returns the same as literalName, except that
    * it fills the returned text with an email or talk address
    * or any other useful information, so the text can be used
    * to identify an entry even if its name fields are all 
    * empty.
    */
  bool description(const string& key, string& text,
		   bool reverse=false, bool initials=false);
  /** This method returns the birthday for this entry in date or 
    * false if the entry does not have a birthday value. 
    */
  bool birthDay(const string& key, QDate& date);
  // constructor
  AddressBook(bool readonly=false);
  // -----
  void restoreDefaults();
  void changed(); // virtual from ConfigDB
  virtual void currentChanged(); // for notifications
};

#endif // ADDRESSBOOK_H
