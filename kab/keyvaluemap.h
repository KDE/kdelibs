/* -*- C++ -*-
 * The class KeyValueMap is used for managing key-value-pairs
 * WITHOUT any hierarchical structure.   Objects of it can be
 * used as they are or in conjunction with the  configuration
 * database class.
 *
 * Capabilities of the class are:
 * - storing of any key-value-pair that is storable in 
 *   string values,
 * - key-value-pairs are saved in human-readable text files
 *   when saving to disk,
 * - the values may contain newline and tabulator characters
 *   which will still be there after saving and rereading,
 * - supports the following datatypes:
 *   - strings (of course),
 *   - integers,
 *   - floating point values and
 *   - boolean states
 * - supports storing and retrieving lists of values of the 
 *   following datatypes:
 *   - strings,
 *   - integers and
 *   - floating point values
 *   (bools possibly supported in future, 
 *    string lists partly n.i.)
 * - easy syntax of files, in general it is supposed to be a 
 *   kind of guarantee  ( you know that free software  never 
 *   guarantess anything, don't you?)  that every value that 
 *   has been  stored  by one of the  member function of the 
 *   class like 
 *      insert(const string& key, [value&]);
 *   can also be retrieved using 
 *      get(const string& key, [value&]);
 *   (please report anything that does not do so!)
 *
 * the Configuration Database library
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@hamburg.netsurf.de>
 *                          <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class
 *             Nana (only for debugging)
 * $Revision$
 */
#ifndef KEYVALUEMAP_H
#define KEYVALUEMAP_H

#pragma interface

#include "stl_headers.h"
#include <fstream.h>

// forward declaration needed to avoid duplicate declaration 
// of global specifiers "red" and "black" in
// * map.h and
// * qcolor.h (Qt-library)

class StringStringMap;

// ########################################################

class KeyValueMap {
protected:
  StringStringMap* data;
  // helper for file handling:
  // bool isComment(const string&);
  // WORK_TO_DO: this is alpha, remove comment after testing
  /// returns true if a "complex string" was found
  bool parseComplexString(const string& orig, 
			  string::size_type index,
			  string& result,
			  int& noOfChars) const;
  /// codes a normal string into a complex string:
  string makeComplexString(const string& orig);
  /** Inserts a string (it must be an already coded one)
    * without coding it.
    */
  bool insertRaw(const string& key, 
		 const string& value, 
		 bool force=false);
  /** Retrieves the undecoded value of the given key.
    */
  bool getRaw(const string& key, string& value) const;
  // -------------------------
public:
  KeyValueMap();
  KeyValueMap(const KeyValueMap&); // copy constructor
  virtual ~KeyValueMap();
  /// returns true if object is OK 
  bool invariant();
  /// returns the number of key-value-pairs in the map
  unsigned int size() const; 
  /// deletes all entries
  void clear(); 
  /** Fills the database from the file. If the parameter 
    * force is true, it overrides keys that are already declared in
    * the database and are declared again in the file.
    * params: ° filename 
    *         ° wether to override existing keys or not
    *         ° wether values may be empty
    */
  bool fill(const string&, bool force=false, bool relax=false);
  /** Saves the database to a file. Only overrides existing 
    * files if force is true.
    * params: filename and wether to override existing files or not
    */
  bool save(const string&, bool force=false);
  /** Saves contents to an already open stream,
    * placing "count" spaces before each line. This method is 
    * called to save fierarchical databases.
    */
  bool save(ofstream& file, int count);
  /* The following pairs of get(..) and insert(..) member functions
   * can be used to store single objects and lists of the 
   * respective data types.
   * The string-insert- and -get-methods are somewhat basic since
   * they are used by all other insert functions, the others 
   * create a string from the respective data and insert this 
   * string.
   */
  // params of get: key, reference to value storage
  // params of insert: key, value and wether  to 
  // override existing keys or not
  // string values:
  bool get(const string&, string&) const;
  bool insert(const string&, const string&, 
	      bool force=false);
  // dummy function to explicitly catch char* as strings
  // (pgcc treats them as bool objects!)
  inline bool insert(const string& key, const char* value,
		     bool force=false);
  // additional insert method for lines
  // params: "<key>=<value>",
  // wether  to override existing keys or not and
  // wether values my be empty strings or not and
  // wether to code the value or not (INTERNAL)
  bool insertLine(string, 
		  bool force=false, 
		  bool relax=false, 
		  bool encode=true);
  // ---------------
  // integer values:
  bool get(const string&, int&) const;
  bool insert(const string&, const int&, bool force=false);
  // ---------------
  // double values:
  bool get(const string&, double&) const;
  bool insert(const string&, const double&, bool force=false);
  // ---------------
  // boolean values:
  bool get(const string&, bool&) const;
  bool insert(const string&, const bool&, bool force=false);
  // ---------------
  // the following methods try to parten the values into lists of values
  // string lists:
  bool get(const string&, list<string>&) const;
  bool insert(const string&, const list<string>&, bool force=false);
  // --------------
  // integer lists:
  bool get(const string&, list<int>&) const;
  bool insert(const string&, const list<int>&, bool force=false);
  // --------------
  // double lists:
  bool get(const string&, list<double>&) const;
  bool insert(const string&, const list<double>&, bool force=false);
  // --------------
  // end of corresponding get-insert-pairs
  // -------------------------
  /// Returns true if there are no keys declared in this map.
  bool empty();
  /** Erases all key-value-pairs in the map, 
    * the map is empty after it.
    */ 
  bool erase(const string& key);
};

// ----- inline functions:
bool KeyValueMap::insert
(const string& key, const char* value,
 bool force=false)
{ 
  string temp(value); 
  return insert(key, temp, force); 
}
// -----

#endif // KEYVALUEMAP_H
