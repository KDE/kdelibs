/* -*- C++ -*-
 * Implementation of the Section class.
 *
 * the Configuration Database library
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class,
 *             Nana (only for debugging)
 * $Revision$
 */

#ifndef KDE_USE_FINAL
#pragma implementation "sections.h"
#endif

#include "sections.h" 
#include <qwidget.h>
#include "debug.h"
#include "functions.h"

const int Section::indent_width=2;

Section::Section()
{
  // ########################################################
  // ########################################################
}

Section::Section(const KeyValueMap& contents)
{
  // ########################################################
  keys=contents;
  CHECK(keys.size()==contents.size());
  // ########################################################
}

bool Section::add(const string& name)
{
  register bool GUARD; GUARD=false;
  // ########################################################
  Section* section=new Section; // create an empty section
  bool rc;
  CHECK(section!=0);
  LG(GUARD, "Section::add: "
     "adding section \"%s\" to this section ... ",
     name.c_str());
  rc=add(name, section);
  if(!rc)
    {
      LG(GUARD && !rc, " failed.\n");
      delete section;
      return false;
    } else {
      LG(GUARD && rc, " success.\n");
      return true;
    }
  // ########################################################
}

bool Section::add(const string& name, Section* section)
{
  register bool GUARD; GUARD=false;
  // ########################################################
  if(sections.insert(StringSectionMap::value_type
		     (name, section)).second)
    {
      LG(GUARD, "Section::add: "
	 "added section %s successfully.\n", name.c_str());
      return true;
    } else {
      LG(GUARD, "Section::add: failed to add section %s, "
	 "section already exists.\n", name.c_str());
      return false;
    }
  // ########################################################
}

bool Section::find
(const string& name,
 StringSectionMap::iterator& result)
{
  register bool GUARD; GUARD=false;
  // ########################################################
  LG(GUARD, "Section::find: "
     "trying to get section \"%s\" ... ", name.c_str());
  StringSectionMap::iterator pos=sections.find(name);
  if(pos==sections.end())
    {
      LG(GUARD, "failed, no such section.\n");
      return false;
    } else {
      LG(GUARD, "success.\n");
      result=pos;
      return true;
    }
  // ########################################################
}

bool Section::remove(const string& name)
{
  // ########################################################
  StringSectionMap::iterator pos;
  if(!find(name, pos))
    {
      // semantic tautology, but calls invariant():
      ENSURE(!find(name, pos));
      return false; // no such section
    } else {
      sections.erase(pos);
      ENSURE(!find(name, pos));
      return true;
    }
  // ########################################################
}

bool Section::find(const string& name, Section*& section)
{
  // ########################################################
  StringSectionMap::iterator pos;
  if(!find(name, pos))
    {
      return false;
    } else {
      section=(*pos).second;
      return true;
    }
  // ########################################################
}

KeyValueMap* Section::getKeys()
{
  // ########################################################
  return &keys;
  // ########################################################
}

void Section::insertIndentSpace(ofstream& file, int level)
{
  REQUIRE(file.good());
  CHECK(level>=0);
  // ########################################################
  int i, j;
  for(i=0; i<level; i++)
    {
      for(j=0; j<indent_width; j++)
	file << ' ';
    }
  // ########################################################
}

bool Section::save(ofstream& stream, int level)
{
  register bool GUARD; GUARD=false;
  REQUIRE(stream.good());
  CHECK(level>=0);
  // ########################################################
  StringSectionMap::iterator pos;
  if(!sections.empty())
    {
      insertIndentSpace(stream, level);
      stream << "# subsections" << endl;
    }
  for(pos=sections.begin(); pos!=sections.end(); pos++)
    {
      insertIndentSpace(stream, level);
      stream << '[' << (*pos).first << ']' << endl;
      if(!(*pos).second->save(stream, level+1))
	{
	  L("Section::save: "
	    "error saving child section \"%s\".\n",
	    (*pos).first.c_str());
	  return false;
	} else {
	  LG(GUARD, "Section::save: "
	     "saved section \"%s\".\n",
	     (*pos).first.c_str());
	}
      insertIndentSpace(stream, level);
      stream << "[END " << (*pos).first << ']' << endl;
    }
  if(!keys.empty())
    {
      insertIndentSpace(stream, level);
      stream << "# key-value-pairs" << endl;
      if(!keys.save(stream, level*indent_width))
	{
	  L("Section::save: "
	    "error saving key-value-pairs.\n");
	  return false;
	}
    }
  return true;
  // ########################################################
}

bool Section::readSection(ifstream& file, bool finish)
{
  register bool GUARD; GUARD=false;
  REQUIRE(file.good());
  // ########################################################
  LG(GUARD, "Section::readSection: reading section.\n");
  string line;
  string name;
  Section* temp;
  for(;;)
    {
      line=ReadLineFromStream(file);
      if(file.eof())
	{
	  if(finish==true)
	    {
	      L("Section::readSection: "
		"missing end of section.\n");
	      return false;
	    } else {
	      LG(GUARD, "Section::readSection: "
		 "EOF (no error).\n");
	      return true;
	    }
	}
      if(isEndOfSection(line))
	{
	  return true;
	}
      if(isBeginOfSection(line))
	{
	  name=nameOfSection(line);
	  add(name);
	  find(name, temp);
	  // errors may not happen here:
	  CHECK(find(name, temp));
	  if(!temp->readSection(file))
	    {
	      L("Section::readSection: "
		"unable to read subsection \"%s\".\n",
		name.c_str());
	      return false;
	    }
	} else { // it has to be a key-value-pair
	  if(!keys.insertLine(line, false, true, false))
	    {
	      cerr
		<< "Attention: unable to parse "
		<< "key-value-pair " << endl
		<< "\t\"" << line << "\"," << endl
		<< "ignoring and continuing (maybe "
		<< "duplicate declaration of the key)."
		<< endl;
	    }
	}
    }
  CHECK(false); // unreachable!
  // ########################################################
}

bool Section::isBeginOfSection(const string& line)
{
  // ########################################################
  string::size_type first, second;
  first=line.find('[');
  second=line.find_first_not_of("[ \t");
  if(first==string::npos || first>second)
    {
      return false;
    }
  // we found the opening bracket
  first=line.find(']', second);
  if(first==string::npos || first<second)
    {
      return false;
    }
  // we found the closing bracket
  second=line.find_first_not_of("] \t", first);
  if(second!=string::npos)
    {
      cerr << "Attention: characters after start of "
	   << "section in line" << endl
	   << "\t\"" << line << "\"" << endl
	   << "will be ignored." << endl;
    }
  return true;
  // ########################################################
}

bool Section::isEndOfSection(const string& line)
{
  // ########################################################
  string::size_type first, second;
  first=line.find('[');
  second=line.find_first_not_of("[ \t");
  if(first==string::npos || first>second)
    {
      return false;
    }
  // we found the opening bracket
  first=line.find(']', second);
  if(first==string::npos || first<second)
    {
      return false;
    }
  // we found the closing bracket
  first=line.find("END");
  if(first==string::npos) return false;
  second=line.find('E');
  if(second!=first)
    {
      return false;
    }
  return true;
  // ########################################################
}

string Section::nameOfSection(const string& line)
{
  // ########################################################
  string::size_type first, second;
  string part;
  first=line.find('[');
  second=line.find_first_not_of("[ \t", first);
  CHECK(first!=string::npos);
  CHECK(second!=string::npos);
  CHECK(first<second);
  first=line.find_first_of("] \t", second+1);
  CHECK(first>second); CHECK(first!=string::npos);
  part.append(line, second, first-second);
  return part;
  // ########################################################
}

bool Section::clear()
{
  // ########################################################
  StringSectionMap::iterator pos;
  for(pos=sections.begin(); pos!=sections.end(); pos++)
    {
      if(!(*pos).second->clear()) return false;
      delete(*pos).second;
    }
  // sections.clear(); // seems to be not implemented
  sections.erase(sections.begin(), sections.end());
  keys.clear();
  CHECK(sections.empty());
  ENSURE(keys.empty());
  return true;
  // ########################################################
}

Section::StringSectionMap::iterator Section::sectionsBegin()
{
  // ########################################################
  return sections.begin();
  // ########################################################
}

Section::StringSectionMap::iterator Section::sectionsEnd()
{
  // ########################################################
  return sections.end();
  // ########################################################
}

unsigned int Section::noOfSections()
{
  // ########################################################
  return sections.size();
  // ########################################################
}

