/* -*- C++ -*-
 * A configuration database consists of sections which in turn 
 * consist of other sections (recursive definition) and 
 * key-value-pairs. This file declares the Section class. An 
 * object of Section manages exactly one section during its 
 * lifetime. Please do not consider the Section class to be
 * part of the user interface, it will possibly be local to the 
 * configuration database class in future.
 * 
 * the Configuration Database library
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@hamburg.netsurf.de>
 *                          <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class,
 *             Nana (only for debugging)
 * $Revision$
 */
#ifndef SECTIONS_H_INCLUDED
#define SECTIONS_H_INCLUDED

#pragma interface

#include "stl_headers.h"
#include "keyvaluemap.h"
#include <algorithm>
#include <fstream.h>

class Section
{
public:
  typedef map<string, Section*, less<string> > StringSectionMap;
protected:
  StringSectionMap sections;
  KeyValueMap keys;
  // spaces inserted to indent each subsection
  static const int indent_width;
  // insert spaces for indention when saving 
  void insertIndentSpace(ofstream& file, int level);
  // evaluate lines that may describe sections in files
  bool isBeginOfSection(const string&);
  bool isEndOfSection(const string&);
  // this extracts the name of the section from the line 
  // that indicates the start of a section
  string nameOfSection(const string&);
public:
  Section();
  Section(const KeyValueMap&);
  // handling sections:
  bool add(const string&);
  bool add(const string&, Section*);
  bool find(const string&, StringSectionMap::iterator&);
  bool find(const string&, Section*&);
  bool remove(const string&);
  // returns the key-value-pairs of the section
  KeyValueMap* getKeys();
  /** Saves section to output stream, level is the position 
    * in section tree depth (hierarchy level). 
    * This is used for indenting.
    */
  bool save(ofstream& stream, int level=0);
  /** Reads one section from the stream, does not 
    * expect the begin of the section. If finish is false,
    * the code does also not except the section to be 
    * ended with [END name].
    */
  bool readSection(ifstream& file, bool finish=true);
  /// Clears all entries.
  bool clear();
  // methods to allow iterating through the subsections
  /// Returns an iterator to the subsections beginning.
  StringSectionMap::iterator sectionsBegin();
  /// Returns an iterator to the subsections end.
  StringSectionMap::iterator sectionsEnd();
  /// Returns the number of subsections of the section.
  unsigned int noOfSections();
};

#endif // SECTIONS_H_INCLUDED
