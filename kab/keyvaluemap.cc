/* Implementation of the KeyValueMap class.
 * 
 * the Configuration Database library
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class
 * $Revision$
 */

#pragma implementation

#include "keyvaluemap.h"
#include "debug.h"
#include "functions.h"
#include <iostream.h>
extern "C" {
#include <stdio.h>
	   }
#include <fstream.h>

class StringStringMap 
  : public map<string, string, less<string> >
{}; // same as map...

KeyValueMap::KeyValueMap()
  : data(new StringStringMap)
{
  // ########################################################
  REQUIRE(data!=0);
  ENSURE(data->empty());
  // ########################################################
}

KeyValueMap::KeyValueMap(const KeyValueMap& orig)
  : data(new StringStringMap(*orig.data))
{
  // ########################################################
  REQUIRE(data!=0); // funny implementation, I think
  ENSURE(size()==orig.size());
  // ########################################################
}

KeyValueMap::~KeyValueMap()
{
  // ########################################################
  REQUIRE(data!=0);
  delete data;
  // ########################################################
}

bool KeyValueMap::invariant()
{
  // ########################################################
  return data!=0;
  // ########################################################
}

unsigned int KeyValueMap::size() const 
{
  // ########################################################
  CHECK(data!=0);
  return data->size();
  // ########################################################
}

void KeyValueMap::clear()
{
  ID(bool GUARD=false);
  // ########################################################
  // map::clear is not implemented for g++
  // data->clear();
  LG(GUARD, "KeyValueMap::clear: erasing map contents...");
  if(!data->empty())
    {
      data->erase(data->begin(), data->end());
    }
  LG(GUARD, "done.\n");
  ENSURE(data->empty());
  // ########################################################
}

bool KeyValueMap::fill(const string& filename, bool force,
		       bool relax)
{
  REQUIRE(!filename.empty());
  ID(bool GUARD=false);
  // ########################################################
  ifstream file(filename.c_str());
  string line;
  if(!file.good())
    {
      LG(GUARD, "KeyValueMap::fill: could not open file "
	 "%s.\n", filename.c_str());
      return false;
    }
  while(getline(file, line))
    {
      if(!line.empty() && !file.eof() && !isComment(line))
	{
	  if(!insertLine(line, force, relax, false))
	    {
	      LG(GUARD, "KeyValueMap::fill: could not "
		 "insert line %s.\n", line.c_str());
	      // ignore this case further
	    }
	}
    }
  return true;
  // ########################################################
}

bool KeyValueMap::save(const string& filename, bool force)
{
  ID(bool GUARD=false);
  REQUIRE(!filename.empty());
  // ########################################################
  LG(GUARD, "KeyValueMap::save: saving data to %s.\n", 
     filename.c_str());
  StringStringMap::iterator pos;
  ofstream file;
  // ----- open file regarding existence:
  if(force)
    {
      file.open(filename.c_str());
    } else {
      file.open(filename.c_str(), ios::nocreate);
    }
  if(!file.good())
    {
      LG(GUARD, "KeyValueMap::save: could not open "
	 "file %s for saving.\n", filename.c_str());
      return false;
    }
  file << "# automatically saved by KeyValueMap object "
       << "($Revision$)" << endl;
  for(pos=data->begin(); pos!=data->end(); pos++)
    {
      file << (*pos).first << '=' << (*pos).second << endl;
      if(!file.good())
	{
	  LG(GUARD, "KeyValueMap::save: error while saving "
	     "data to file %s.\n", filename.c_str());
	  file.close();
	  return false;
	}
    }
  file.close();
  // ########################################################
  return true; 
}

bool KeyValueMap::save(ofstream& file, int count)
{
  ID(bool GUARD=false);
  REQUIRE(file.good());
  CHECK(count>=0);
  // ########################################################
  LG(GUARD, "KeyValueMap::save: saving data to given "
     "output stream.\n");
  StringStringMap::iterator pos;
  char* prefix=new char[count+1](' '); prefix[count]=0;
  CHECK(prefix!=0);
  // -----
  for(pos=data->begin(); pos!=data->end(); pos++)
    {
      file << prefix;
      file << (*pos).first;
      file << '=';
      file << (*pos).second;
      file << endl;
      if(!file.good())
	{
	  LG(GUARD, "KeyValueMap::save: error while "
	     "saving data.\n");
	  return false;
	}
    }
  return true;
  // ########################################################
}

bool KeyValueMap::erase(const string& key)
{
  // ########################################################
  bool rc=(data->erase(key)>0);
  ENSURE(data->find(key)==data->end());
  return rc;
  // ########################################################
}  

bool KeyValueMap::empty()
{
  // ########################################################
  REQUIRE(data!=0);
  return data->empty();
  // ########################################################
}

bool KeyValueMap::parseComplexString
(const string& orig, 
 string::size_type index, // first char to parse
 string& result, // string without leading and trailing ".."
 int& noOfChars) // no of chars that represented the 
 const           // complex string in the original
{
  ID(bool GUARD=false);
  // ########################################################
  string::size_type first, second;
  string temp;
  int count=1;
  LG(GUARD, "KeyValueMap::parseComplexString: parsing the "
     "string -->%s<--.\n", orig.c_str());
  if(orig.empty())
    {
      LG(GUARD, "KeyValueMap::parseComplexString: string is "
	 "empty.\n                                 "
	 "This is no valid complex string.\n");
      return false;
    }
  first=orig.find('"', index);
  // bug fix for reading empty strings, Feb 14 98:
  second=orig.find_first_not_of(" \t\"", index);
  if(first==string::npos
     || (first>second && second!=string::npos))
    {
      LG(GUARD, "KeyValueMap::parseComplexString: opening "
	 "double quote, but other chars before it.\n"
	 "           This is no valid complex string.\n");
	return false;
    }
  second=orig.find('"', first+1);
  // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  if(second==string::npos 
     || first>second)
    {
      LG(GUARD, "KeyValueMap::parseComplexString: could not "
	 "find beginning of string.\n                       "
	 "          This is no valid complex string.\n");
      return false;
    }
  first++;
  for(;;) 
    {
      CHECK(first<orig.size());
      if(orig[first]=='\\')
	{ // handle special characters
	  first++;
	  LG(GUARD, "KeyValueMap::parseComplexString: found "
	     "a special character \"%c\".\n", orig[first]);
	  if(first==orig.size())
	    {
	      LG(GUARD, "KeyValueMap::parseComplexString: "
		 "string lacks the closing \".\n          "
		 "                       This is no valid "
		 "complex string.\n");
	      return false;
	    }
	  switch(orig[first])
	    {
	    case 't': temp+='\t'; break;
	    case 'n': temp+='\n'; break;
	    case '"': temp+='"'; break;
	    case '\\': temp+='\\'; break;
	    default:
	      LG(GUARD, "KeyValueMap::parseComplexString: "
		 "invalid control character.\n            "
		 "                     This is no valid "
		 "complex string.\n");
	      return false;	      
	    }
	  count+=2; // this took 2 characters
	  ++first;
	} else { // it is a character
	  ++count;
	  if(orig[first]=='"') 
	    {
	      break;
	    }
	  temp+=orig[first];
	  ++first;	  
	}
      if(first>=orig.size())
	{
	  LG(GUARD, "KeyValueMap::parseComplexString: "
	     "string lacks the closing \".\n              "
	     "                   This is no valid complex "
	     "string.\n");
	  return false;
	}
    }
  LG(GUARD, "KeyValueMap::parseComplexString: finished "
     "parsing, no errors, %i characters, %i in string.\n",
     count, temp.size());
  noOfChars=count;
  result=temp;
  // ########################################################
  return true;
}

string KeyValueMap::makeComplexString(const string& orig)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "KeyValueMap::makeComplexString: coding the "
     "string\n           -->%s<--\n                          "
     "      into a complex string.\n", orig.c_str());
  string temp;
  unsigned int count;
  temp+='"';
  for(count=0; count<orig.size(); count++)
    {
      switch(orig[count])
	{ // catch all special characters:
	case '"':
	  LG(GUARD, "KeyValueMap::makeComplexString: "
	     "found the special char \"%c\".\n", 
	     orig[count]);
	  temp+='\\';
	  temp+='"';
	  break;
	case '\n':
	  temp+='\\';
	  temp+='n';
	  break;
	case '\t':
	  temp+='\\';
	  temp+='t';
	  break;
	case '\\':
	  temp+='\\';
	  temp+='\\';
	  break;
	default: temp+=orig[count];
	}
    }
  temp+='"';  
  LG(GUARD, "KeyValueMap::makeComplexString: result is\n"
            "           -->%s<--.\n", temp.c_str());
  return temp;
  // ########################################################
}

bool KeyValueMap::getRaw
(const string& key,
 string& value) const
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "KeyValueMap::getRaw: trying to get raw value "
     "for key \"%s\" ... ", key.c_str());
  StringStringMap::iterator pos=data->find(key);
  if(pos==data->end())
    {
      LG(GUARD, "not in KeyValueMap.\n");
      return false;
    } else {
      value=(*pos).second;
      LG(GUARD, "in KeyValueMap, value is %s.\n", 
	 value.c_str());
      return true;
    }
  // ########################################################
}

bool KeyValueMap::insertRaw
(const string& key,
 const string& value,
 bool force)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "KeyValueMap::insertRaw: inserting uncoded value"
     " %s for key %s.\n", value.c_str(), key.c_str());
  int n=0;
  if(key.empty()) 
    { // empty keys are errors:
      L("KeyValueMap::insertRaw: "
	"tried to insert empty key.\n");
      return false;
    }
  if(force) // entry will be replaced
    {
      n=data->erase(key);
      // for all i in data i.key!=key?
      I(AO(i, (*data), !((*i).first==key)));
    }
  if(data->insert(StringStringMap::value_type
		 (key, value)).second)
    {
      LG(GUARD, "KeyValueMap::insertRaw: success%s.\n",
	 (n==0 ? "" : " (forced)"));
      return true;
    } else {
      LG(GUARD, "KeyValueMap::insertRaw: failed, "
	 "key already in KeyValueMap.\n");
      return false;
    }  
  // ########################################################
}

// ----------------------------------------------------------
// HUGE SEPARATOR BETWEEN INTERNAL LOGIC AND EXTENDABLE PAIRS
// OF GET-AND INSERT-METHODS.
// EXTENDABLE MEANS: OTHER DATATYPES CAN BE ADDED HERE.
// ----------------------------------------------------------

/** The following functions are the pairs of 
  * insert-get-methods for different data types. See 
  * keyvaluemap.h for the declarations.
  */

// strings:

bool KeyValueMap::insert
(const string& key, 
 const string& value,
 bool force) 
{
  ID(bool GUARD=false); 
  // ########################################################
  LG(GUARD, 
     "KeyValueMap::insert: inserting value\n"
     "           -->%s<-- \n"
     "                     for key\n"
     "           -->%s<--.\n",
     value.c_str(), key.c_str());
  return insertRaw(key, makeComplexString(value), force);
  // ########################################################
}

/* Attention: this is another insert function that partens 
 * lines like "key=value"!
 * It is used for reading files and command line parameters.
 */

bool KeyValueMap::insertLine(string line, 
			     bool force, 
			     bool relax,
			     bool encode)
{
  ID(bool GUARD=false);
  //  REQUIRE(!line.empty());
  // ########################################################
  string::size_type index;
  string key;
  string value;
  // ----- is the line empty or does it contain only 
  //       whitespaces?
  index=line.find_first_not_of(" \t");
  if(line.empty() || index==string::npos)
    { 
      LG(GUARD, "KeyValueMap::insertLine: line is empty.\n");
      return false;
    }
  // -----
  index=line.find('=');
  if(index==string::npos) 
    {
      L("KeyValueMap::insertLine: no \"=\" found in \"%s\".\n", 
	line.c_str());
      return false;
    }
  // -----
  key.assign(line, 0, index); // copy from start to '='
  value.assign(line, index+1); // copy the rest
  // keys should not contain whitespaces 
  // to avoid unpredictable results
  for(;;)
    {
      index=key.find_first_of(" \t");
      /* exit : if */ if(index==string::npos) break;
      key.ERASE(index, 1);
    }
  if(!key.empty() && (relax==true ? 1 : !value.empty() ) )
    {
      if(encode)
	{ // normal way:
	  return insert(key, value, force);
	} else { // while loading from a already coded file:
	  return insertRaw(key, value, force);
	}
    } else {
      L("KeyValueMap::insertLine: key %s is empty.\n",
	relax ? "" : "or value ");
      return false;
    }
  // ########################################################
}

bool KeyValueMap::get
(const string& key, 
 string& value) const
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "KeyValueMap::get[string]: trying to get value "
     "for key \"%s\" ... ", key.c_str());
  string raw;
  string temp;
  if(!getRaw(key, raw))
    {
      return false;
    } else {
      // change from Feb 3 1998:
      LG(GUARD, "KeyValueMap::get[string]: checking wether "
	 "this is a complex string.\n");
      {
	int count;
	if(parseComplexString(raw, 0, temp, count))
	  {
	    LG(GUARD, "KeyValueMap::get[string]: "
	       "complex string found.\n");
	    value=temp;
	  } else {
	    LG(GUARD, "KeyValueMap::get[string]: "
	       "this is not a complex string.\n");
	    // non-complex strings are no more allowed in 
	    // KeyValueMap objects - there is no way to 
	    // insert them exepct by hand!
	    CHECK(false); // kill debug version
	    return false; // only in non-debug version
	  }
      }
      // ^^^^^^
      return true;
    }
  // ########################################################
}


// (^^^ strings)
// bool:

bool KeyValueMap::insert
(const string& key,
 const bool& value,
 bool force)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "KeyValueMap::insert[bool]: trying to insert "
     "\"%s\" for key\n           -->%s<--.\n",
     value==true ? "true" : "false", key.c_str());
  string temp;
  if(value==true)
    {
      temp="true";
    } else {
      temp="false";
    }
  return insert(key, temp, force);
  // ########################################################
}


bool KeyValueMap::get(const string& key, bool& value) const
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "KeyValueMap::get[bool]: trying to get BOOL "
     "value for key %s.\n", key.c_str());
  string v;
  // ----- get string representation:
  if(!get(key, v)) 
    {
      LG(GUARD, "KeyValueMap::get[bool]: key %s not in "
	 "KeyValueMap.\n", key.c_str());
      return false;
    }
  // ----- find its state:
  //       hopefully the compiler optimizes the logical tests
  //       and finishs after comparing with "true" if v is 
  //       true
  if(v=="true" || v=="TRUE" || v=="True" || v=="1")
    {
      LG(GUARD, "KeyValueMap::get[bool]: success, value "
	 "is TRUE.\n");
      value=true;
      return true;
    }
  if(v=="false" || v=="FALSE" || v=="False" || v=="0")
    {
      LG(GUARD, "KeyValueMap::get[bool]: success, value "
	 "is FALSE.\n");
      value=false;
      return true;
    }
  LG(GUARD, "KeyValueMap::get[bool]: "
     "failure, unknown value.\n");
  return false;
  // ########################################################
}

// (^^^ bool)
// int:

bool KeyValueMap::insert
(const string& key, 
 const int& value, 
 bool force)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "KeyValueMap::insert[int]: trying to insert "
     "value \"%i\" for key\n           -->%s<--.\n",
     value, key.c_str());
  const int BufferSize=64;
  char buffer[BufferSize];
  // ----- insert a single integer into the buffer:
  if(sprintf(buffer, "%i", value)>=BufferSize)
    {
      L("KeyValueMap::insert[int]: buffer overflow.\n");
      return false;
    } else {
      return insert(key, buffer, force);
    }
  // ########################################################
}

bool KeyValueMap::get(const string& key, int& value) const
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "KeyValueMap::get[int]: trying to get INTEGER "
     "value for key %s.\n", key.c_str());
  string v;
  int temp;
  if(!get(key, v)) 
    {
      LG(GUARD, "KeyValueMap::get[int]: key %s not in "
	 "KeyValueMap.\n", key.c_str());
      return false;
    }
  errno=0;
  temp=strtol(v.c_str(), 0, 0);
  if(errno==ERANGE)
    {
      LG(GUARD, "KeyValueMap::get[int]: the value %s is no "
	 "int number.\n", v.c_str());
      return false;
    }
  LG(GUARD, "KeyValueMap::get[int]: success, value is %i.\n", 
     temp);
  value=temp;
  return true;
  // ########################################################
}

// (^^^ int)
// int lists:

bool KeyValueMap::insert(const string& key, 
			 const list<int>& values, 
			 bool force)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "KeyValueMap::insert[int list]: trying to insert"
     " int list into map.\n");
  char buffer[1024];
  string value;
  // -----
  list<int>::const_iterator pos;
  for(pos=values.begin(); pos!=values.end(); pos++)
    {
      sprintf(buffer, "%i, ", (*pos));
      value+=buffer;
    }
  if(!value.empty())
    { // remove the comma and the space:
      value.ERASE(value.size()-2, 2); 
    }
  LG(GUARD, "KeyValueMap::insert[int list]: constructed "
     "string value is %s.\n", value.c_str());
  return insert(key, value, force);
  // ########################################################
}


bool KeyValueMap::get
(const string& key, 
 list<int>& values) const
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "KeyValueMap::get[int list]: trying to decode"
  " int list for key %s.\n", key.c_str());
  LG(!values.empty(), "KeyValueMap::get[int list]: attention"
     " - list should be empty but is not.\n");
  string value;
  string::size_type first=0;
  char* c_str;
  char** end=&c_str;
  int temp;
  // -----
  if(!get(key, value)) 
  {
    LG(GUARD, "KeyValueMap::get[int list]: no such key.\n");
    return false;
  }
  // -----
  for(;;) 
    {
      errno=0;
      temp=strtol((c_str=(char*)value.c_str())+first, 
		  end, 0);
      if(temp==0) 
	{ // ----- possibly there where no numbers:
	  if(*end==c_str)
	    {
	      LG(GUARD, "KeyValueMap::get[int list]: no "
		 "numbers found.\n");
	    } else { // the number is really zero:
	      LG(GUARD, "KeyValueMap::get[int list]: "
		 "found zero.\n");
	      values.push_back(temp);
	    }
	  first=value.find(',', first+1);
	} else {	    
	  if(errno==ERANGE)
	    {
	      LG(GUARD, "KeyValueMap::get[int list]: "
		 "the value %s is no int number.\n", 
		 value.c_str()+first+1);
	      return false;
	    } else {
	      LG(GUARD, "KeyValueMap::get[int list]: "
		 "found %i.\n", temp);
	      values.push_back(temp);
	      first=value.find(',', first+1);
	    }
	}
      /* exit: if */ if(first==string::npos) break;
      ++first;
    }
  LG(GUARD, "KeyValueMap::get[int list]: done.\n");
  // ########################################################
  return true;
}

// (^^^ int lists)
// doubles:
 
bool KeyValueMap::insert
(const string& key,
 const double& value,
 bool force)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "KeyValueMap::insert[double]: trying to insert "
     "value \"%f\" for key\n           -->%s<--.\n",
     value, key.c_str());
  const int BufferSize=128;
  char buffer[BufferSize];
  if(sprintf(buffer, "%f", value)>=BufferSize)
    {
      LG(GUARD, "KeyValueMap::insert[double]: "
	 "buffer overflow.\n");
      return false;
    } else {
      return insert(key, buffer, force);
    }
  // ########################################################
}
 
 
bool KeyValueMap::get(const string& key, double& value) const
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "KeyValueMap::get[double]: trying to get FLOAT "
     "value for key %s.\n", key.c_str());
  string v;
  double temp;
  if(!get(key, v)) 
    {
      LG(GUARD, "KeyValueMap::get[double]: key %s not in "
	 "KeyValueMap.\n", key.c_str());
      return false;
    }
  errno=0; 
  temp=strtod(v.c_str(), 0);
  if(errno==ERANGE)
    {
      LG(GUARD, "KeyValueMap::get[double]: the value %s is "
	 "no double number.\n", v.c_str());
      return false;
    }
  LG(GUARD, "KeyValueMap::get[double]: success, value is "
     "%f.\n", temp);
  value=temp;  
  return true;
  // ########################################################
}

// (^^^ doubles)
// lists of strings:

bool KeyValueMap::get
(const string& key, 
 list<string>& values) const
{
  ID(bool GUARD=false);
  LG(!values.empty(), "KeyValueMap::get[string list]: "
     "attention!\n             \"values\" list reference "
     "is not empty!\n"); 
  // ########################################################
  LG(GUARD, "KeyValueMap::get[string list]: trying to decode"
  " string list for key %s.\n", key.c_str());
  string raw, value, part;
  int i;
  string::size_type first=1, second;
  // -----
  // ----- get the string value as a whole:
  if(!getRaw(key, raw))
    {
      LG(GUARD, "KeyValueMap::get[list<string>]: key %s not"
	 " in KeyValueMap.\n", key.c_str());
      return false;
    }
  // "first" points to the start of the currently parsed 
  // substring
  for(;;)
    { // ----- parten the string down into a list
      // find special characters:
      second=first;
      for(;;)
	{
	  second=raw.find("\\", second);
	  // this may never be the last and also not the 
	  // second last character in a complex string
	  if(second!=string::npos)
	    { // ----- check for string end:
	      CHECK(first<raw.size()-2);
	      // we use "\e" as token for the 
	      // string-delimiter
	      if(raw[second+1]=='e' // the right character
		 && raw[second-1]!='\\') // not escaped
		{ 
		  LG(GUARD, "KeyValueMap::get"
		     "[list<string>]: found string end "
		     "at pos %i.\n", second);
		  break;
		} else {
		  ++second;
		}
	    } else {
	      break;
	    }
	}
      if(second!=string::npos)
	{
	  // ----- now second points to the end of the 
	  //       substring:
	  part="\""+raw.substr(first, second-first)+"\"";
	  // ----- insert decoded value into the list:
	  if(parseComplexString(part, 0, value, i))
	    {
	      LG(GUARD, "KeyValueMap::get[list<string>]: "
		 "found item %s.\n", value.c_str());
	      values.push_back(value);
	    } else {
	      L("KeyValueMap::get[list<string>]: parse "
		"error in string list.\n");
	      return false;
	    }
	  if(second<raw.size()-3) 
	    { // there may be another string
	      first=second+2;
	    } else { // we are completely finished
	      LG(GUARD, "KeyValueMap::get[list<string>]: "
		 "list end found.\n");
	      break;
	    }
	} else { // ----- finished:
	  break;
	}
    }
  LG(GUARD, "KeyValueMap::get[list<string>]: done.\n");
  return true;
  // ########################################################
}

bool KeyValueMap::insert
(const string& key, 
 const list<string>& values,
 bool force)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "KeyValueMap::insert[string list]: "
     "coding string list.\n");
  string value="\""; string temp;
  list<string>::const_iterator pos;
  // ----- create coded string list:
  for(pos=values.begin();
      pos!=values.end();
      pos++)
    { // create strings like "abc\efgh\eijk":
      temp=makeComplexString(*pos);
      CHECK(temp.size()>=2);
      temp.ERASE(0, 1); // remove the leading "\""
      temp.ERASE(temp.size()-1, 1); // the trailing "\""
      value+=temp;
      value+="\\e";
    }
  value+="\""; // finish the string
  LG(GUARD, "KeyValueMap::insert[string list]: result of "
     "coding is %s.\n", value.c_str());
  // ----- insert it without coding:
  return insertRaw(key, value, force);
  // ########################################################
}

// (^^^ lists of strings)
// lists of doubles:

bool KeyValueMap::insert(const string& key,
			 const list<double>& values,
			 bool force)
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "KeyValueMap::insert[double list]: trying to "
     "insert double list into map.\n");  
  char buffer[1024];
  string value;
  // -----
  list<double>::const_iterator pos;
  for(pos=values.begin(); pos!=values.end(); pos++)
    {
      sprintf(buffer, "%f, ", (*pos));
      value+=buffer;
    }
  if(!value.empty())
    { // remove the comma and the space:
      value.ERASE(value.size()-2, 2); 
    }
  LG(GUARD, "KeyValueMap::insert[double list]: constructed "
     "string value is %s.\n", value.c_str());
  return insert(key, value, force);  
  // ########################################################
}

bool KeyValueMap::get
(const string& key, 
 list<double>& values) const
{
  ID(bool GUARD=false);
  // ########################################################
  LG(GUARD, "KeyValueMap::get[double list]: trying to decode"
     " double list for key %s.\n", key.c_str());  
  LG(!values.empty(), "KeyValueMap::get[double list]: "
     "attention - list should be empty but is not.\n");
  string value;
  char* c_str;
  char** end=&c_str;
  string::size_type first=0;
  double temp;
  if(!get(key, value)) 
    {
      LG(GUARD, 
	 "KeyValueMap::get[double list]: no such key.\n");
      return false;
    }
  // -----
  for(;;) 
    {
      errno=0; 
      temp=strtod((c_str=(char*)value.c_str())+first, end);
      if(temp==0)
	{ // possibly there where no chars representing an 
	  // int:
	  if(*end==c_str)
	    {
	      LG(GUARD, "KeyValueMap::get[double list]: no "
		 "numbers found.\n");
	    } else {
	      LG(GUARD, "KeyValueMap::get[double list]: "
		 "found zero.\n");
	      values.push_back(temp);
	      first=value.find(',', first+1);	    
	    }
	} else {
	  if(errno==ERANGE)
	    {
	      LG(GUARD, "KeyValueMap::get[double list]: "
		 "the value %s is no double number.\n", 
		 value.c_str()+first+1);
	      return false;
	    } else {
	      LG(GUARD, "KeyValueMap::get[double list]: "
		 "found %f.\n", temp);
	      values.push_back(temp);
	      first=value.find(',', first+1);
	    }
	}
      /* exit: if */ if(first==string::npos) break;
      ++first;
    }
  LG(GUARD, "KeyValueMap::get[int list]: done.\n");
  // ########################################################  
  return true;
}

// (^^^ lists of doubles)
