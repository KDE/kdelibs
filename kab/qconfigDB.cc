/* the Configuration Database library, Version II
 *
 * the KDE addressbook
 * copyright:  (C) Mirko Sucker, 1998, 1999
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko@kde.org>
 * requires:   recent C++-compiler, at least Qt 2.0
 * $Revision$
 */

#include "qconfigDB.h"
// #include "debug.h"

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
}

// #include <qstring.h>
#include <qtextstream.h>
#include <qfile.h>
#include <iostream.h>
#include <qtimer.h>
#include <qdatetime.h>
#include <qfileinfo.h>

#include "qconfigDB.moc"
#include <kdebug.h>
#include <knana.h>

#ifdef KAB_KDEBUG_AREA
#undef KAB_KDEBUG_AREA
#endif 

#define KAB_KDEBUG_AREA 800

// the global functions:
void evaluate_assertion(bool cond, const char* file, int line, const char* text)
{
  if(!cond)
    {
      debug("%s:%i: %s", file, line, text);
    }
}

// QCString AuthorEmailAddress; // assign your email address to this string

QCString ReadLineFromStream(QTextStream& stream)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "ReadLineFromStream:: reading line.\n");
  QCString line;
  // -----
  while(!stream.eof())
    {
      line=stream.readLine();
      if(!line.isEmpty())
	{
	  if(isComment(line))
	    {
	      line="";
	      continue;
	    }
	}
      break;
    }
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "ReadLineFromStream:: line \"%s\" read.\n", line.data());
  return line;
  // ############################################################################
}

bool isComment(QCString line)
{
  // ############################################################################
  line=line.stripWhiteSpace();
  if(line.isEmpty())
    {
      return false; // line is empty but not a comment
    } else  {
      return(line[0]=='#');
    }
  // ############################################################################
}

bool htmlizeString(const QCString& orig, QCString& target)
{
  register bool GUARD; GUARD=false;
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "htmlizeString: called.\n");
  // ############################################################################
  QCString temp;
  char current;
  unsigned int index;
  // -----
  for(index=0; index<orig.length(); index++)
    { // ----- the following code is character set depending 
      //       and will possibly need recompilation for EVERY
      //       system using another charset than the machine
      //       it was compiled on! Lets see.
      current=orig[index];
      switch(current)
	{
	case 'ä':
	default: temp+=current;
	}
    }
  target=temp;
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "htmlizeString: done.\n");
  return false; // not implemented
  // ############################################################################
}

void tokenize(list<QCString>& res, const QCString& text, char tr, bool strict)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "tokenize: called.\n");
  int eins=0, zwei=0;
  QCString teil;
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "tokenize: partening -->%s<--.\n", text.data());
  res.erase(res.begin(), res.end());
  // -----
  if(text.isEmpty())
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "tokenize: test is an empty string, done.\n");
      return;
    }
  while(zwei!=-1)
    {
      teil="";
      zwei=text.find(tr, eins);
      if(zwei!=-1)
	{
	  teil=text.mid(eins, zwei-eins);
	  CHECK(teil.length()==(unsigned)zwei-eins);
	  res.push_back(teil);
	} else { // last element
	  if(!strict) // nur wenn dazwischen Zeichen sind
	    {
	      teil=text.mid(eins, text.length()-eins);
	      CHECK(teil.length()==text.length()-eins);
	      res.push_back(teil);
	    }
	}
      eins=zwei+1;
      // if((unsigned)eins>=text.length()) break;
    }
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "tokenize: partened in %i parts.\n", res.size());
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "tokenize: done.\n");
  // ############################################################################
}

// class implementations:

list<QString> QConfigDB::LockFiles; // the lockfiles created by this session

KeyValueMap::KeyValueMap()
  : data(new StringStringMap)
{
  // ###########################################################################
  REQUIRE(data!=0);
  ENSURE(data->empty());
  // ###########################################################################
}

KeyValueMap::KeyValueMap(const KeyValueMap& orig)
  : data(new StringStringMap(*orig.data))
{
  // ###########################################################################
  REQUIRE(data!=0);
  ENSURE(size()==orig.size());
  // ###########################################################################
}

KeyValueMap::~KeyValueMap()
{
  // ###########################################################################
  REQUIRE(data!=0);
  delete data;
  // ###########################################################################
}

unsigned int 
KeyValueMap::size() const 
{
  // ###########################################################################
  CHECK(data!=0);
  return data->size();
  // ###########################################################################
}

void 
KeyValueMap::clear()
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::clear: erasing map contents ... ");
  // -----
  if(!data->empty()) // erase fails on empty containers!
    {
      data->erase(data->begin(), data->end());
    }
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "done.\n");
  ENSURE(data->empty());
  // ###########################################################################
}

bool 
KeyValueMap::fill(const QString& filename, bool force, bool relax)
{
  REQUIRE(!filename.isEmpty());
  register bool GUARD; GUARD=false;
  // ###########################################################################
  QFile file(filename);
  QCString line;
  // -----
  if(file.open(IO_ReadOnly))
    {
      QTextStream stream(&file);
      // -----
      while(!stream.eof())
	{
	  line=stream.readLine();
	  if(!line.isEmpty() /* && !stream.eof() */ && !isComment(line))
	    {
	      if(!insertLine(line, force, relax, false))
		{
		  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::fill: could not insert line %s.\n", 
		     (const char*)line); // ignore this case further
		}
	    }
	}
      file.close();
      // -----
      return true;
    } else {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::fill: cannot open file %s.\n",
	 (const char*)filename.utf8());
      return false;
    }
  // ###########################################################################
}

bool 
KeyValueMap::save(const QString& filename, bool force)
{
  register bool GUARD; GUARD=false;
  REQUIRE(!filename.isEmpty());
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::save: saving data to -->%s<--.\n", 
     (const char*)filename.utf8());
  StringStringMap::iterator pos;
  QFile file(filename);
  // ----- open file, regarding existence:
  if(!force)
    {
      if(file.exists())
	{
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::save: file exists but may not.\n");
	  return false;
	}
    }
  if(file.open(IO_WriteOnly))
    {
      QTextStream stream(&file);
      stream << "# saved by KeyValueMap object ($Revision$)" << endl;
      for(pos=data->begin(); pos!=data->end(); ++pos)
	{ // values do not get coded here
	  stream << (*pos).first << '=' << (*pos).second << endl;
	}
      file.close();
    } else {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::save: could not open file -->%s<-- for saving.\n", 
	 (const char*)filename.utf8());
      return false;
    }
  // ###########################################################################
  return true; 
}

bool 
KeyValueMap::save(QTextStream& file, int count)
{
  register bool GUARD; GUARD=false;
  CHECK(count>=0);
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::save: saving data to given output stream.\n");
  StringStringMap::iterator pos;
  bool ret=true;
  char* prefix=new char[count+1]; 
  memset(prefix, ' ', count);
  prefix[count]=0;
  CHECK(prefix!=0);
  // -----
  for(pos=data->begin(); pos!=data->end(); pos++)
    {
      file << prefix << (*pos).first << '=' << (*pos).second << endl;
    }
  delete [] prefix;
  // -----
  return ret;
  // ###########################################################################
}


bool 
KeyValueMap::erase(const QCString& key)
{
  // ###########################################################################
  bool rc=(data->erase(key)>0);
  ENSURE(data->find(key)==data->end());
  return rc;
  // ###########################################################################
}  


bool 
KeyValueMap::empty()
{
  // ###########################################################################
  REQUIRE(data!=0);
  return data->empty();
  // ###########################################################################
}

bool 
KeyValueMap::parseComplexString
(const QCString& orig, 
 int index, // first char to parse
 QCString& result, // string without leading and trailing ".."
 int& noOfChars) // no of chars that represented the
 const           // complex string in the original
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  int first;
  QCString temp;
  QCString mod;
  int count=1;
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::parseComplexString: parsing the string -->%s<--.\n", 
     (const char*)orig);
  // -----
  if(orig.isEmpty())
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::parseComplexString: string is empty.\n"
	 "                                 "
	 "This is no valid complex string.\n");
      return false;
    }
  // ----- prepare the string:
  temp=orig.mid(index, orig.length()-index); // remove everything before index
  mod=temp.stripWhiteSpace(); // remove leading and trailing white spaces
  // ----- test some conditions:
  if(mod.length()<2)
    {
      kDebugInfo("KeyValueMap::parseComplexString: no pair of brackets around the "
	"string.\n");
      return false;
    }
  if(mod[0]!='"')
    {
      kDebugInfo("KeyValueMap::parseComplexString: no opening bracket.\n");
      return false;
    }
  // ----- now parse it:
  first=1; // first character after opening bracket
  temp="";
  for(;;) 
    {
      if(mod[first]=='\\')
	{ // handle special characters
	  ++first;
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::parseComplexString: found "
	     "a special character \"%c\".\n", mod[first]);
	  if((unsigned)first==mod.length())
	    {
	      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::parseComplexString: "
		 "string lacks the closing \".\n          "
		 "                       This is no valid "
		 "complex string.\n");
	      return false;
	    }
	  switch(mod[first])
	    {
	    case 't': temp+='\t'; break;
	    case 'n': temp+='\n'; break;
	    case '"': temp+='"'; break;
	    case 'e': temp+="\\e"; break;
	    case '\\': temp+='\\'; break;
	    default:
	      // WORK_TO_DO: implement octal coding here!
	      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::parseComplexString: "
		 "invalid control character.\n            "
		 "                     This is no valid complex string.\n");
	      return false;	      
	    }
	  count+=2; // this took 2 characters
	  ++first;
	} else { // it is a character
	  ++count;
	  if(mod[first]=='"') // end of coded string?
	    {
	      break;
	    }
	  temp+=mod[first];
	  ++first;	  
	}
      if((unsigned)first>=mod.length())
	{
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::parseComplexString: "
	     "string lacks the closing \".\n              "
	     "                   This is no valid complex string.\n");
	  return false;
	}
    }
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::parseComplexString: finished parsing, no errors, "
     "%i characters, %i in string.\n", count, temp.length());
  noOfChars=count;
  result=temp;
  // ###########################################################################
  return true;
}

QCString 
KeyValueMap::makeComplexString(const QCString& orig)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::makeComplexString: coding the string\n           -->"
     "%s<--\n                                into a complex string.\n", 
     (const char*)orig);
  QCString temp;
  unsigned int count;
  // -----
  temp+='"'; // opening bracket
  for(count=0; count<orig.length(); count++)
    {
      switch(orig[count])
	{ // catch all special characters:
	case '"':
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::makeComplexString: found the special char "
	     "\"%c\".\n", orig[count]);
	  temp+='\\';
	  temp+='"';
	  break;
	case '\n':
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::makeComplexString: found the special char "
	     "\"%c\".\n", orig[count]);
	  temp+='\\';
	  temp+='n';
	  break;
	case '\t':
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::makeComplexString: found the special char "
	     "\"%c\".\n", orig[count]);
	  temp+='\\';
	  temp+='t';
	  break;
	case '\\':
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::makeComplexString: found the special char "
	     "\"%c\".\n", orig[count]);
	  temp+='\\';
	  temp+='\\';
	  break;
	default: temp+=orig[count];
	}
    }
  temp+='"'; // closing bracket
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::makeComplexString: result is\n           -->%s<--.\n", 
     (const char*)temp);
  return temp;
  // ###########################################################################
}

bool 
KeyValueMap::getRaw(const QCString& key, QCString& value) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::getRaw: trying to get raw value for key \"%s\" ... ", 
     (const char*)key);
  StringStringMap::iterator pos=data->find(key);
  // -----
  if(pos==data->end())
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "not in KeyValueMap.\n");
      return false;
    } else {
      value=(*pos).second;
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "in KeyValueMap, value is %s.\n", (const char*)value);
      return true;
    }
  // ###########################################################################
}

bool 
KeyValueMap::insertRaw(const QCString& key, const QCString& value, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insertRaw: inserting uncoded value %s for key %s.\n", 
     (const char*)value, (const char*)key);
  int n=0;
  // -----
  if(key.isEmpty()) // empty KEYS are errors:
    {
      kDebugInfo("KeyValueMap::insertRaw: tried to insert empty key.\n");
      return false;
    }
  if(force) // entry will be replaced
    {
      n=data->erase(key);
      CHECK(data->find(key)==data->end());
    }
  if(data->insert(StringStringMap::value_type(key, value)).second)
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insertRaw: success%s.\n",
	 (n==0 ? "" : " (forced)"));
      return true;
    } else {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insertRaw: failed, "
	 "key already in KeyValueMap.\n");
      return false;
    }  
  // ###########################################################################
}


// -----------------------------------------------------------------------------
// HUGE SEPARATOR BETWEEN INTERNAL LOGIC AND EXTENDABLE PAIRS OF GET- AND INSERT
// -METHODS.
// EXTENDABLE MEANS: OTHER DATATYPES CAN BE ADDED HERE.
// -----------------------------------------------------------------------------

/* The following functions are the pairs of insert-get-methods for different 
 * data types. See keyvaluemap.h for the declarations.  */

// ascii strings:

bool 
KeyValueMap::insert(const QCString& key, const QCString& value, bool force) 
{
  register bool GUARD; GUARD=false; 
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert: inserting value\n           -->%s<-- \n"
     "                     for key\n           -->%s<--.\n",
     (const char*)value, (const char*)key);
  return insertRaw(key, makeComplexString(value), force);
  // ###########################################################################
}

/* Attention: 
 * This is another insert function that partens lines like "key=value"!
 * It is used for reading files and command line parameters easily.
 */

bool 
KeyValueMap::insertLine(QCString line, bool force, bool relax, bool encode)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insertLine: inserting line -->%s<--.\n", 
     (const char*)line);
  int index;
  QCString key;
  QCString value;
  // ----- is the line empty or does it contain only whitespaces?
  for(index=0; isspace(line[index]) && (unsigned)index<line.length(); ++index);
  CHECK((unsigned)index<=line.length());
  if(line.isEmpty() || (unsigned)index==line.length())
    { 
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insertLine: line is empty.\n");
      return false;
    }
  // -----
  index=line.find('=');
  if(index==-1)  // not found
    {
      kDebugInfo("KeyValueMap::insertLine: no \"=\" found in \"%s\".\n", 
	(const char*)line);
      return false;
    }
  // -----
  key=line.mid(0, index); // copy from start to '='
  value=line.mid(index+1, line.length()-1-index); // copy the rest
  // ----- only alphanumerical characters are allowed in the keys:
  for(index=key.length()-1; index>-1; /* nothing */)
    {      
      if(!(isalnum(key[index]) || ispunct(key[index])))
	 {	
	   key=key.remove(index, 1); // WORK_TO_DO: optimize this (very slow)!
	 }
      --index;
    }
  // ----- now insert it if key is still valid:
  if(!key.isEmpty() && (relax==true ? 1 : !value.isEmpty() ) )
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insertLine: done.\n");
      if(encode)
	{ // the usual way:
	  return insert(key, value, force);
	} else { // while loading from a already coded file:
	  return insertRaw(key, value, force);
	}
    } else {
      kDebugInfo("KeyValueMap::insertLine: key %s is empty.\n", relax ? "" : "or value ");
      return false;
    }
  // ###########################################################################
}

bool 
KeyValueMap::get(const QCString& key, QCString& value) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[string]: trying to get value for key \"%s\" ... ", 
     (const char*)key);
  QCString raw;
  QCString temp;
  // -----
  if(!getRaw(key, raw))
    {
      return false; // key does not exist
    } else {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[string]: checking wether this is a complex "
	 "string.\n");
      {
	int count;
	if(parseComplexString(raw, 0, temp, count))
	  {
	    kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[string]: complex string found.\n");
	    value=temp;
	  } else {
	    kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[string]: this is no complex string.\n");
	    // disabled this strong check:
	    // CHECK(false); // kill debug version
	    return false;
	  }
      }
      // ^^^^^^
      return true;
    }
  // ###########################################################################
}

// (^^^ ascii strings)
// UNICODE strings:

bool 
KeyValueMap::insert(const QCString& key, const QString& value, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  QCString v;
  // -----
  v=value.utf8();
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[QString]: trying to insert \"%s\" for key\n       "
     "    -->%s<--.\n", !value.isNull() ? "TRUe" : "false", (const char*)v);
  return insert(key, v, force);
  // ###########################################################################
}

bool 
KeyValueMap::get(const QCString& key, QString& value)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QString]: trying to get a QString value for key "
     "%s.\n", (const char*)key);
  QCString v;
  // ----- get string representation:
  if(!get(key, v)) 
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QString]: key %s not in KeyValueMap.\n", 
	 (const char*)key);
      return false;
    }
  // ----- find its state:
  value=QString::fromUtf8(v); // is there a better way?
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QString]: success, value (in UTF8) is %s.\n", 
     (const char*)v);
  return true;
  // ###########################################################################
}

// (^^^ UNICODE strings)
// bool:

bool 
KeyValueMap::insert(const QCString& key, const bool& value, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[bool]: trying to insert \"%s\" for key\n       "
     "    -->%s<--.\n", value==true ? "true" : "false", (const char*)key);
  return insert(key, value==true ? "true" : "false", force);
  // ###########################################################################
}


bool 
KeyValueMap::get(const QCString& key, bool& value) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[bool]: trying to get BOOL value for key %s.\n", 
     (const char*)key);
  QCString v;
  // ----- get string representation:
  if(!get(key, v)) 
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[bool]: key %s not in KeyValueMap.\n", 
	 (const char*)key);
      return false;
    }
  // ----- find its state:
  v=v.stripWhiteSpace();
  if(v=="true")
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[bool]: success, value is TRUE.\n");
      value=true;
      return true;
    }
  if(v=="false")
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[bool]: success, value is FALSE.\n");
      value=false;
      return true;
    }
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[bool]: failure, unknown value.\n");
  // -----
  return false;
  // ###########################################################################
}

// (^^^ bool)
// long:

bool 
KeyValueMap::insert(const QCString& key, const long& value, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[int]: trying to insert value \"%i\" for key\n  "
     "         -->%s<--.\n", (int)value, (const char*)key);
  QCString temp;
  // -----
  temp.setNum(value);
  return insert(key, temp, force);
  // ###########################################################################
}

bool 
KeyValueMap::get(const QCString& key, long& value) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[int]: trying to get INTEGER value for key %s.\n", 
     (const char*)key);
  QCString v;
  bool ok;
  long temp;
  // -----
  if(!get(key, v)) 
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[int]: key %s not in KeyValueMap.\n", 
	 (const char*)key);
      return false;
    }
  // -----
  temp=v.toLong(&ok);
  if(ok)
    {
      value=temp;
      return true;
    } else {
      return false;
    }
  // ###########################################################################
}

// (^^^ long)
// long int lists:

bool 
KeyValueMap::insert(const QCString& key, const list<long>& values, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[long int list]: trying to insert"
     " long int list into map.\n");
  QCString temp;
  QCString value;
  list<long>::const_iterator pos;
  // -----
  for(pos=values.begin(); pos!=values.end(); pos++)
    {
      temp.setNum(*pos);
      value=value+temp+", ";
    }
  if(!value.isEmpty())
    { // remove last comma and space:
      value.remove(value.length()-2, 2); 
    }
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[long int list]: constructed string value is "
     "%s.\n", (const char*)value);
  return insert(key, value, force);
  // ###########################################################################
}

bool 
KeyValueMap::get(const QCString& key, list<long>& values) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[long int list]: trying to decode int list for key "
     "%s.\n", (const char*)key);
  kDebugInfo(!values.empty(), KAB_KDEBUG_AREA, "KeyValueMap::get[long int list]: attention - list should "
     "be empty but is not.\n");
  QCString value;
  list<QCString> tokens;
  list<QCString>::iterator pos;
  long temp;
  bool ok;
  // -----
  if(!get(key, value)) 
  {
    kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[long int list]: no such key.\n");
    return false;
  }
  tokenize(tokens, value, ',');
  if(tokens.empty())
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[long int list]: no tokens.\n");
      return false;
    }
  // -----
  for(pos=tokens.begin(); pos!=tokens.end(); ++pos) 
    {
      temp=(*pos).toLong(&ok);
      if(ok)
	{
	  values.push_back(temp);
	} else {
	  kDebugInfo("KeyValueMap::get[long int list]: conversion error for %s.\n", 
	    (const char*)(*pos));
	}
    }
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[long int list]: done.\n");
  // ###########################################################################
  return true;
}

// (^^^ long int lists)
// int lists:

bool 
KeyValueMap::insert(const QCString& key, const list<int>& values, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[int list]: trying to insert"
     " int list into map.\n");
  QCString temp;
  QCString value;
  list<int>::const_iterator pos;
  // -----
  for(pos=values.begin(); pos!=values.end(); pos++)
    {
      temp.setNum(*pos);
      value=value+temp+", ";
    }
  if(!value.isEmpty())
    { // remove last comma and space:
      value.remove(value.length()-2, 2); 
    }
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[int list]: constructed string value is %s.\n", 
     (const char*)value);
  return insert(key, value, force);
  // ###########################################################################
}

bool 
KeyValueMap::get(const QCString& key, list<int>& values) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[int list]: trying to decode int list for key "
     "%s.\n", (const char*)key);
  kDebugInfo(!values.empty(), KAB_KDEBUG_AREA, "KeyValueMap::get[int list]: attention - list should "
     "be empty but is not.\n");
  QCString value;
  list<QCString> tokens;
  list<QCString>::iterator pos;
  int temp;
  bool ok;
  // -----
  if(!get(key, value)) 
  {
    kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[int list]: no such key.\n");
    return false;
  }
  tokenize(tokens, value, ',');
  if(tokens.empty())
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[int list]: no tokens.\n");
      return false;
    }
  // -----
  for(pos=tokens.begin(); pos!=tokens.end(); ++pos) 
    {
      temp=(*pos).toInt(&ok);
      if(ok)
	{
	  values.push_back(temp);
	} else {
	  kDebugInfo("KeyValueMap::get[int list]: conversion error for %s.\n", 
	    (const char*)(*pos));
	}
    }
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[long int list]: done.\n");
  // ###########################################################################
  return true;
}

// (^^^ int lists)
// doubles:
 
bool 
KeyValueMap::insert(const QCString& key, const double& value, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[double]: trying to insert value \"%f\" for key"
     "\n           -->%s<--.\n", value, (const char*)key);
  QCString temp;
  // -----
  temp.setNum(value);
  return insert(key, temp, force);
  // ###########################################################################
}

bool 
KeyValueMap::get(const QCString& key, double& value) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[double]: trying to get FLOAT value for key %s.\n", 
     (const char*)key);
  QCString v;
  bool ok;
  double temp;
  // -----
  if(!get(key, v)) 
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[int]: key %s not in KeyValueMap.\n", 
	 (const char*)key);
      return false;
    }
  // -----
  temp=v.toDouble(&ok);
  if(ok)
    {
      value=temp;
      return true;
    } else {
      return false;
    }
  // ###########################################################################
}

// (^^^ doubles)
// lists of strings:

bool 
KeyValueMap::get(const QCString& key, list<QCString>& values) const
{
  register bool GUARD; GUARD=false;
  kDebugInfo(!values.empty(), KAB_KDEBUG_AREA, "KeyValueMap::get[string list]: attention!\n             "
     "\"values\" list reference is not empty!\n"); 
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[string list]: trying to decode string list for key"
     " %s.\n", (const char*)key);
  QCString raw, part, value;
  int first=1, second=1, i;
  // ----- get the string value as a whole:
  if(!getRaw(key, raw))
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[list<string>]: key %s not in KeyValueMap.\n", 
	 (const char*)key);
      return false;
    }
  // -----
  for(;;)
    { // ----- parten the string down into a list, find special characters:
      second=first;
      for(;;)
	{
	  second=raw.find('\\', second);
	  // ----- this may never be the last and also not the second last 
	  //       character in a complex string:
	  if(second!=-1)
	    { // ----- check for string end:
	      CHECK((unsigned)first<raw.length()-2);
	      // we use "\e" as token for the string-delimiter
	      if(raw[second+1]=='e' // the right character
		 && raw[second-1]!='\\') // not escaped
		{ 
		  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[list<string>]: found string "
		     "end at pos %i.\n", second);
		  break;
		} else {
		  ++second;
		}
	    } else {
	      break;
	    }
	}
      if(second!=-1)
	{
	  // ----- now second points to the end of the substring:
	  part="\""+raw.mid(first, second-first)+"\"";
	  // ----- insert decoded value into the list:
	  if(parseComplexString(part, 0, value, i))
	    {
	      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[list<string>]: found item %s.\n", 
		 value.data());
	      values.push_back(value);
	    } else {
	      kDebugInfo("KeyValueMap::get[list<string>]: parse error.\n");
	      return false;
	    }
	  if((unsigned)second<raw.length()-3) 
	    { // ----- there may be another string
	      first=second+2;
	    } else { // ----- we are completely finished
	      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[list<string>]: list end found.\n");
	      break;
	    }
	} else { // ----- finished:
	  break;
	}
    }
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[list<string>]: done.\n");
  return true;
  // ###########################################################################
}

bool 
KeyValueMap::insert(const QCString& key, const list<QCString>& values, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[string list]: coding string list.\n");
  QCString value="\""; 
  QCString temp;
  list<QCString>::const_iterator pos;
  // ----- create coded string list:
  for(pos=values.begin();
      pos!=values.end();
      pos++)
    { // create strings like "abc\efgh\eijk":
      temp=makeComplexString(*pos);
      CHECK(temp.length()>=2); // at least "\\"\\""
      temp.remove(0, 1); // remove the leading "\""
      temp.remove(temp.length()-1, 1); // the trailing "\""
      value+=temp;
      value+="\\e";
    }
  value+="\""; // finish the string
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[string list]: result of coding is %s.\n", 
     (const char*)value);
  // ----- insert it without coding:
  return insertRaw(key, value, force);
  // ###########################################################################
}

// (^^^ lists of strings)
// QStrList-s:

bool 
KeyValueMap::get(const QCString& key, QStrList& values) const
{
  register bool GUARD; GUARD=false;
  kDebugInfo(!values.isEmpty(), KAB_KDEBUG_AREA, "KeyValueMap::get[QStrList]: attention!\n             "
     "\"values\" list reference is not empty!\n"); 
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QStrList]: trying to decode string list for key"
     " %s.\n", (const char*)key);
  QCString raw, part, value;
  int first=1, second=1, i;
  // ----- get the string value as a whole:
  if(!getRaw(key, raw))
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QStrList]: key %s not in KeyValueMap.\n", 
	 (const char*)key);
      return false;
    }
  // -----
  for(;;)
    { // ----- parten the string down into a list, find special characters:
      second=first;
      for(;;)
	{
	  second=raw.find('\\', second);
	  // ----- this may never be the last and also not the second last 
	  //       character in a complex string:
	  if(second!=-1)
	    { // ----- check for string end:
	      CHECK((unsigned)first<raw.length()-2);
	      // we use "\e" as token for the string-delimiter
	      if(raw[second+1]=='e' // the right character
		 && raw[second-1]!='\\') // not escaped
		{ 
		  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QStrList]: found string "
		     "end at pos %i.\n", second);
		  break;
		} else {
		  ++second;
		}
	    } else {
	      break;
	    }
	}
      if(second!=-1)
	{
	  // ----- now second points to the end of the substring:
	  part="\""+raw.mid(first, second-first)+"\"";
	  // ----- insert decoded value into the list:
	  if(parseComplexString(part, 0, value, i))
	    {
	      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QStrList]: found item %s.\n", 
		 value.data());
	      values.append(value);
	    } else {
	      kDebugInfo("KeyValueMap::get[QStrList]: parse error.\n");
	      return false;
	    }
	  if((unsigned)second<raw.length()-3) 
	    { // ----- there may be another string
	      first=second+2;
	    } else { // ----- we are completely finished
	      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QStrList]: list end found.\n");
	      break;
	    }
	} else { // ----- finished:
	  break;
	}
    }
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QStrList]: done.\n");
  return true;
  // ###########################################################################
}

bool 
KeyValueMap::insert(const QCString& key, const QStrList& values, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[QStrList]: coding string list.\n");
  QCString value="\""; 
  QCString temp;
  unsigned int count;
  // ----- create coded string list:
  for(count=0; count<values.count(); ++count)
    { // create strings like "abc\efgh\eijk":
      temp=makeComplexString(((QStrList)values).at(count));
      CHECK(temp.length()>=2); // at least "\\"\\""
      temp.remove(0, 1); // remove the leading "\""
      temp.remove(temp.length()-1, 1); // the trailing "\""
      value+=temp;
      value+="\\e";
    }
  value+="\""; // finish the string
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[QStrList]: result of coding is %s.\n", 
     (const char*)value);
  // ----- insert it without coding:
  return insertRaw(key, value, force);
  // ###########################################################################
}

// (^^^ QStrList-s)
// QStringList-s:

bool 
KeyValueMap::get(const QCString& key, QStringList& values) const
{
  register bool GUARD; GUARD=false;
  kDebugInfo(!values.isEmpty(), KAB_KDEBUG_AREA, "KeyValueMap::get[QStringList]: attention!\n          "
     "   \"values\" list reference is not empty!\n"); 
  // ###########################################################################
  /* The values are stored in a utf8-coded set of QCStrings.
     This list is retrieved and converted back to Unicode strings. */
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QStringList]: trying to decode QStringList for"
     " key %s.\n", (const char*)key);
  QStrList temp;
  unsigned int count;
  // ----- get the plain C strings:
  if(!get(key, temp))
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QStringList]: key %s not in KeyValueMap.\n", 
	 (const char*)key);
      return false;
    }
  // ----- do the conversion:
  for(count=0; count<temp.count(); ++count)
    {
      values.append(QString::fromUtf8(temp.at(count)));
    }
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QStringList]: done.\n");
  return true;
  // ###########################################################################
}

bool 
KeyValueMap::insert(const QCString& key, const QStringList& values, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[QStringList]: coding QStringList.\n");
  // The method simply creates a list of utf8-coded strings and inserts it.
  QStrList utf8strings;
  unsigned int count;
  // ----- create QCString list:
  for(count=0; count<values.count(); ++count)
    {
      utf8strings.append((*values.at(count)).utf8());
    }
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[QStringList]: done.\n");
  return insert(key, utf8strings, force);
  // ###########################################################################
}

// (^^^ QStringList-s)
// lists of doubles:

bool 
KeyValueMap::insert(const QCString& key, const list<double>& values, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[double list]: trying to insert double list into"
     " map.\n");  
  QCString buffer;
  // QCString value(30*values.size()); // not usable with Qt 2
  QCString value; // WORK_TO_DO: how to reserve enough space to avoid growing?
  list<double>::const_iterator pos;
  // -----
  for(pos=values.begin(); pos!=values.end(); pos++)
    {
      buffer.setNum(*pos);
      value=value+buffer+", ";
    }
  if(!value.isEmpty())
    { // remove last comma and space:
      value.remove(value.length()-2, 2); 
    }
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[double list]: constructed string value is "
     "%s.\n", (const char*)value);
  return insert(key, value, force);  
  // ###########################################################################
}

bool
KeyValueMap::get(const QCString& key, list<double>& values) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[double list]: trying to decode double list for key"
     " %s.\n", (const char*)key);  
  kDebugInfo(!values.empty(), KAB_KDEBUG_AREA, "KeyValueMap::get[double list]: attention - list should be"
     " empty but is not.\n");
  QCString value;
  list<QCString> tokens;
  list<QCString>::iterator pos;
  double temp;
  bool ok;
  // -----
  if(!get(key, value)) 
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[double list]: no such key.\n");
      return false;
    }
  // -----
  tokenize(tokens, value, ',');
  for(pos=tokens.begin(); pos!=tokens.end(); ++pos)
    {
      temp=(*pos).toDouble(&ok);
      if(ok)
	{
	  values.push_back(temp);
	} else {
	  kDebugInfo("KeyValueMap::get[double list]: conversion error for %s.\n", 
	    (const char*)(*pos));
	}
    }
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[int list]: done.\n");
  // ###########################################################################  
  return true;
}

// (^^^ lists of doubles)
// QDates:

bool 
KeyValueMap::insert(const QCString& key, const QDate& value, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[QDate]: trying to insert QDate into"
     " map.\n");  
  list<long> values;
  // -----
  if(!value.isValid())
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::insert[QDate]: invalid date, inserting a null "
	 "date.\n");
      for(int i=0; i<3; ++i) values.push_back(0);
    } else {
      values.push_back(value.year());
      values.push_back(value.month());
      values.push_back(value.day());
    }
  CHECK(values.size()==3);
  // -----
  return insert(key, values, force);
  // ###########################################################################  
}

bool 
KeyValueMap::get(const QCString& key, QDate& date) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QDate]: trying to decode QDate for key"
     " %s.\n", key.data());  
  list<long> values;
  long y, m, d;
  QDate temp;
  // -----
  if(!get(key, values))
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QDate]: no such key.\n");
      return false;
    }
  if(values.size()!=3)
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QDate]: more or less than 3 values.\n");
      return false;
    }
  y=values.front(); values.pop_front();
  m=values.front(); values.pop_front();
  d=values.front(); CHECK(values.size()==1);
  // -----
  if(y!=0 || m!=0 || d!=0) temp.setYMD(y, m, d); // avoid QDate messages
  if(!temp.isValid() && !temp.isNull())
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QDate]: no valid date.\n");
      return false;
    } else {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "KeyValueMap::get[QDate]: done.\n");
      date=temp;
      return true;
    }  
  // ###########################################################################  
}
  
// (^^^ QDates)
// Section class:

const int Section::indent_width=2;

Section::Section()
{
  // ###########################################################################  
  // ###########################################################################  
}

Section::Section(const KeyValueMap& contents)
{
  // ###########################################################################  
  keys=contents;
  CHECK(keys.size()==contents.size());
  // ###########################################################################  
}

bool 
Section::add(const QCString& name)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################  
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::add: adding section \"%s\" to this section ... ",
     name.data());
  Section* section;
  bool rc;
  // -----
  if(name.isEmpty())
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::add: empty key.\n");
      return false;
    }
  section=new Section; // create an empty section
  if(section==0)
    {
      kDebugInfo("Section::add: out of memory.\n");
      return false;
    }
  rc=add(name, section);
  if(!rc)
    {
      kDebugInfo(GUARD && !rc, KAB_KDEBUG_AREA, " failed.\n");
      delete section;
      return false;
    } else {
      kDebugInfo(GUARD && rc, KAB_KDEBUG_AREA, " success.\n"); 
      return true;
    }
  // ###########################################################################  
}

bool 
Section::add(const QCString& name, Section* section)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################  
  if(sections.insert(StringSectionMap::value_type(name, section)).second)
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::add: added section %s successfully.\n", name.data());
      return true;
    } else {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::add: failed to add section %s, section already "
	 "exists.\n", name.data());
      return false;
    }
  // ###########################################################################  
}

bool 
Section::find(const QCString& name, StringSectionMap::iterator& result)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################  
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::find: trying to get section \"%s\" ... ", name.data());
  StringSectionMap::iterator pos;
  // -----
  pos=sections.find(name);
  if(pos==sections.end())
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "failed, no such section.\n");
      return false;
    } else {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "success.\n");
      result=pos;
      return true;
    }
  // ###########################################################################  
}

bool 
Section::remove(const QCString& name)
{
  // ###########################################################################  
  StringSectionMap::iterator pos;
  // -----
  if(!find(name, pos))
    {
      return false; // no such section
    } else {
      sections.erase(pos);
      ENSURE(!find(name, pos));
      return true;
    }
  // ###########################################################################  
}

bool 
Section::find(const QCString& name, Section*& section)
{
  // ###########################################################################  
  StringSectionMap::iterator pos;
  // -----
  if(!find(name, pos))
    {
      return false;
    } else {
      section=(*pos).second;
      return true;
    }
  // ###########################################################################  
}

KeyValueMap* 
Section::getKeys()
{
  // ###########################################################################  
  return &keys;
  // ###########################################################################  
}

void 
Section::insertIndentSpace(QTextStream& file, int level)
{
  CHECK(level>=0);
  // ###########################################################################  
  int i, j;
  // -----
  for(i=0; i<level; i++)
    {
      for(j=0; j<indent_width; j++)
	file << ' ';
    }
  // ###########################################################################  
}

bool 
Section::save(QTextStream& stream, int level)
{
  register bool GUARD; GUARD=false;
  CHECK(level>=0);
  // ###########################################################################  
  StringSectionMap::iterator pos;
  // -----
  if(!sections.empty())
    { // ----- insert a comment:
      insertIndentSpace(stream, level);
      stream << "# subsections:" << endl;
    }
  for(pos=sections.begin(); pos!=sections.end(); pos++)
    {
      insertIndentSpace(stream, level);
      stream << '[' << (*pos).first << ']' << endl;
      if(!(*pos).second->save(stream, level+1))
	{
	  kDebugInfo("Section::save: error saving child section \"%s\".\n",
	    (*pos).first.data());
	  return false;
	} else {
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::save: saved section \"%s\".\n", 
	     (*pos).first.data());
	}
      insertIndentSpace(stream, level);
      stream << "[END " << (*pos).first << ']' << endl;
    }
  if(!keys.empty())
    {
      insertIndentSpace(stream, level);
      stream << "# key-value-pairs:" << endl;
      if(!keys.save(stream, level*indent_width))
	{
	  kDebugInfo("Section::save: error saving key-value-pairs.\n");
	  return false;
	}
    }
  // -----
  return true;
  // ###########################################################################  
}

bool 
Section::readSection(QTextStream& file, bool finish)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################  
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::readSection: reading section.\n");
  QCString line;
  QCString name;
  Section* temp;
  // -----
  for(;;)
    {
      line="";
      line=ReadLineFromStream(file);
      if(isEndOfSection(line))
	{ // ----- eof does not matter:
	  return true;
	} else { // ----- verify it:
	  if(file.eof())
	    {
	      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::readSection: EOF, line is \"%s\".\n",
		 line.data());
	      if(!line.isEmpty())
		{
		  if(!keys.insertLine(line, false, true, false))
		    {
		      cerr << "Attention: unable to parse key-value-pair " 
			   << endl << "\t\"" << line << "\"," << endl 
			   << "ignoring and continuing (maybe duplicate "
			   << "declaration of the key)." 
			   << endl;
		    }
		}
	      if(finish==true) 
		{
		  kDebugInfo("Section::readSection: missing end of section.\n");
		  return false;
		} else {
		  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::readSection: EOF (no error).\n");
		  return true;
		}
	    }
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
	      kDebugInfo("Section::readSection: unable to read subsection \"%s\".\n",
		name.data());
	      return false;
	    }
	} else { // ----- it has to be a key-value-pair:
	  if(!keys.insertLine(line, false, true, false))
	    {
	      cerr << "Attention: unable to parse key-value-pair " << endl
		   << "\t\"" << line << "\"," << endl 
		   << "ignoring and continuing (maybe duplicate declaration of"
		   << " the key)." 
		   << endl;
	    }
	}
    }
  // -----
  CHECK(false); // unreachable!
  // ###########################################################################  
}

bool 
Section::isBeginOfSection(QCString line)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################  
  line=line.simplifyWhiteSpace();
  if(line.isEmpty() || line.length()<2)
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::isBeginOfSection: too short or empty line.\n");
      return false;
    }
  if(line[0]!='[' || line[line.length()-1]!=']')
    {
      return false;
    }
  // -----
  if(line.contains("END"))
    {
      return false;
    } else {
      return true;
    }
  // ###########################################################################  
}

bool 
Section::isEndOfSection(QCString line)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::isEndOfSection: is %s the end of a section?\n",
     line.data());
  int first=1, second;
  QCString temp;
  // -----
  line=line.simplifyWhiteSpace();
  if(line.isEmpty() || line.length()<2)
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::isBeginOfSection: too short or empty line.\n");
      return false;
    }
  if(line[0]!='[' || line[line.length()-1]!=']')
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::isBeginOfSection: does not match.\n");
      return false;
    }
  // ----- find the word inside the brackets:
  for(first=1; line[first]==' '; ++first); // find first non-whitespace character
  for(second=first; line[second]!=' ' && line[second]!=']'; ++second);
  CHECK(first<=second && (unsigned)second<line.length());
  temp=line.mid(first, second-first);
  if(temp=="END")
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::isBeginOfSection: yes, it is.\n");
      return true;
    } else {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::isBeginOfSection: no, it is not.\n");
      return false;
    }
  // ###########################################################################  
}

QCString 
Section::nameOfSection(const QCString& line)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################  
  int first=1, second;
  QCString temp;
  // -----
  temp=line.simplifyWhiteSpace();
  if(temp.isEmpty() || temp.length()<=2)
    { // empty section names are not allowed
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "Section::isBeginOfSection: too short or empty line.\n");
      return "";
    }
  if(temp[0]!='[' || temp[temp.length()-1]!=']')
    {
      return "";
    }
  // ----- find the word inside the brackets:
  for(first=1; temp[first]==' '; ++first); // find first non-whitespace character
  for(second=first; temp[second]!=' ' && temp[second]!=']'; ++second);
  CHECK(first<=second && (unsigned)second<temp.length());
  temp=temp.mid(first, second-first);
  if(temp=="END")
    {
      return "";
    } else {
      return temp;
    }
  // ###########################################################################  
}

bool 
Section::clear()
{
  // ###########################################################################  
  StringSectionMap::iterator pos;
  // -----
  for(pos=sections.begin(); pos!=sections.end(); pos++)
    {
      if(!(*pos).second->clear()) return false;
      delete(*pos).second;
    }
  // sections.clear(); // seems to be not implemented
  sections.erase(sections.begin(), sections.end());
  keys.clear();
  CHECK(sections.empty());
  // -----
  ENSURE(keys.empty());
  return true;
  // ###########################################################################  
}

bool 
Section::empty()
{
  // ###########################################################################  
  return keys.empty() && sections.empty();
  // ###########################################################################  
}

Section::StringSectionMap::iterator 
Section::sectionsBegin()
{
  // ###########################################################################  
  return sections.begin();
  // ###########################################################################  
}

Section::StringSectionMap::iterator 
Section::sectionsEnd()
{
  // ###########################################################################  
  return sections.end();
  // ###########################################################################  
}

unsigned int 
Section::noOfSections()
{
  // ###########################################################################  
  return sections.size();
  // ###########################################################################  
}

QConfigDB::QConfigDB(QWidget* parent, const char* name)
  : QWidget(parent, name),
    timer(0),
    readonly(true),
    locked(false),
    mtime(new QDateTime)
{
  // ###########################################################################
  hide();
  // ###########################################################################  
}


QConfigDB::~QConfigDB()
{
  // ############################################################################
  // disconnect();
  // -----
  if(timer!=0)
    {
      delete timer; timer=0;
    }
  if(!clear()) // this will emit changed() a last time
    {
      kDebugInfo("QConfigDB destructor: cannot remove me.\n");
    }
  CHECK(empty());
  if(locked) 
    {
      unlock();
    }
  // ############################################################################
}

bool 
QConfigDB::get(const list<QCString>& key, KeyValueMap*& map)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::get: trying to get keys ... ");
  Section* section=&top;
  list<QCString>::const_iterator pos;
  // -----
  if(key.empty()) 
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "\nConfigDB::get: path is empty, returning toplevel section.\n");
      map=top.getKeys();
      return true;
    }
  for(pos=key.begin(); pos!=key.end(); pos++)
    {
      if(!section->find(*pos, section))
	{
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "failed,\n               at least the element \"%s\" of "
	     "the key-list is not declared.\n", (*pos).data());
	  return false;
	}
    }
  // -----
  map=section->getKeys();
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "success.\n");
  return true;
  // ############################################################################
}

KeyValueMap* 
QConfigDB::get()
{
  // ############################################################################
  return top.getKeys();
  // ############################################################################
}

bool 
QConfigDB::createSection(const list<QCString>& key)
{ REQUIRE(!key.empty());
  // ############################################################################
  Section* section=&top;
  unsigned int index;
  list<QCString>::const_iterator pos;
  Section* thenewone;
  bool rc;
  // -----
  pos=key.begin();
  for(index=0; index<key.size()-1; index++)
    {
      if(!section->find(*pos, section))
	{ // this section is not declared
	  Section* temp=new Section; // WORK_TO_DO: memory hole?
	  if(section->add(*pos, temp))
	    {
	      section=temp; 
	    } else {
	      CHECK(false); // this may not happen
	      delete temp;
	      return false;
	    }
	}
      ++pos;
    }
  // pos now points to the last element of key,
  // section to the parent of the section that will be inserted
  thenewone=new Section;
  rc=section->add(*pos, thenewone);
  // this overrides section (!!):
  CHECK(section->find(*pos, section)); // now it must be there in every case
  return rc; // missing error report! WORK_TO_DO
  // ############################################################################
}

bool 
QConfigDB::clear()
{
  // ############################################################################
  bool rc=top.clear();
  emit(changed(this));
  return rc;
  // ############################################################################
}

bool 
QConfigDB::empty()
{
  // ############################################################################
  return top.empty();
  // ############################################################################
}

bool 
QConfigDB::createSection(const QCString& desc)
{
  // ############################################################################
  return createSection(stringToKeylist(desc));
  // ############################################################################
}

bool 
QConfigDB::get(const QCString& key, KeyValueMap*& map)
{
  // ############################################################################
  return get(stringToKeylist(key), map);
  // ############################################################################
}

list<QCString> 
QConfigDB::stringToKeylist(const QCString& desc)
{
  register bool GUARD; GUARD=false;
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::stringToKeylist: parsing path %s.\n", desc.data());
  // ############################################################################
  list<QCString> key;
  int first=0, second;
  QCString temp;
  // -----
  if(desc.isEmpty())
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::stringToKeylist: path is empty.\n");
      return key;
    }
  for(;;) 
    {
      second=desc.find('/', first);
      if(second==-1)
	{
	  if((unsigned)first<desc.length()+1)
	    {
	      temp=desc.mid(first, desc.length()-first);
	      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::stringToKeylist: found last part %s.\n", 
		 temp.data());
	      key.push_back(temp);
	    }
	  break;
	}
      temp=desc.mid(first, second-first);
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::stringToKeylist: found part %s.\n", temp.data());
      key.push_back(temp);
      first=second+1;
    }
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::stringToKeylist: done.\n");
  return key;
  // ############################################################################
}


bool 
QConfigDB::get(const QCString& key, Section*& section)
{
  // ############################################################################
  return get(stringToKeylist(key), section);
  // ############################################################################
}

bool 
QConfigDB::get(const list<QCString>& key, Section*& section)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::get: searching section ... ");
  Section* temp=&top;
  list<QCString>::const_iterator pos;
  // -----
  for(pos=key.begin(); pos!=key.end(); pos++)
    {
      if(!temp->find(*pos, temp))
	{
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "failure, no such section.\n");
	  return false;
	}
    }
  // -----
  section=temp; 
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "success, section found.\n");
  return true;
  // ############################################################################
}

bool 
QConfigDB::isRO()
{
  // ############################################################################
  return readonly;
  // ############################################################################
}

int 
QConfigDB::IsLocked(const QString& file)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  QString lockfile=file+".lock";
  int pid=-1;
  // -----
  if(access(lockfile, F_OK)==0)
    {
      QFile f(lockfile);
      // -----
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::IsLocked: the file\n        %s\n"
	 "                    has a lockfile.\n", file.data());
      if(f.open(IO_ReadOnly))
	{
	  QTextStream stream(&f);
	  // -----
	  stream >> pid;
	  if(pid==-1)
	    {
	      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::IsLocked: the file does not contain the ID\n"
		 "        of the process that created it.\n");
	      return -1;
	    }
	  f.close();
	} else {
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::IsLocked: cannot open the lockfile.\n");
	  return -1;
	}
      return pid;      
    } else {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::IsLocked: the file\n        %s has no lockfile.\n", 
	 file.data());
      return 0;
    }
  // ############################################################################
}

bool 
QConfigDB::lock()
{
  register bool GUARD; GUARD=false;
  REQUIRE(!filename.isEmpty());
  // ############################################################################
  if(locked)
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::lock (current file): file is already locked by this "
	 "object.\n");
      return false;
    }
  if(lock(filename))
    {
      locked=true;
      return true;
    } else {
      return false;
    }
  // ############################################################################
}

bool 
QConfigDB::lock(const QString& file)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::lock: locking the file %s.\n", file.data());
  QString lockfile=file+".lock";
  QFile f(lockfile);
  // -----
  if(access(lockfile, F_OK)==0)
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::lock: the file is locked by another process.\n");
      return false;
    } else {
      if(f.open(IO_WriteOnly))
	{
	  QTextStream stream(&f);
	  // -----
	  stream << getpid() << endl;
	  f.close();
	} else {
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::lock: unable to create lockfile.\n");
	  return false;
	}
    }
  // -----
  LockFiles.push_back(lockfile);
  return true;
  // ############################################################################
}

bool 
QConfigDB::unlock()
{
  register bool GUARD; GUARD=false;
  REQUIRE(!filename.isEmpty());
  // ############################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::unlock: unlocking the file %s.\n", filename.data());
  QString lockfile=filename+".lock";
  list<QString>::iterator pos;
  // -----
  if(!locked)
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::unlock: this app did not lock the file!\n");
      return false;
    }
  if(access(lockfile, F_OK | W_OK)==0)
    {
      if(::remove(lockfile)==0)
	{
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::unlock: lockfile deleted.\n");
	  for(pos=LockFiles.begin(); pos!=LockFiles.end(); ++pos)
	    {
	      if((*pos)==lockfile) break;
	    }
	  if(pos!=LockFiles.end())
	    {
	      LockFiles.erase(pos); --pos;
	    } else {
	      kDebugInfo("QConfigDB::unlock: file not mentioned in lockfile list.\n");
	    }
	  locked=false;
	  return true;
	} else {
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::unlock: unable to delete lockfile.\n");
	  return false;
	}
    } else {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::unlock: the file is not locked or permission has "
	 "been denied.\n");
      return false;
    }
  // ############################################################################
}

void 
QConfigDB::CleanLockFiles(int)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  list<QString>::iterator pos;
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::CleanLockFiles: removing %i remaining lockfiles.\n", 
     LockFiles.size());
  for(pos=LockFiles.begin(); pos!=LockFiles.end(); pos++)
    {
      if(::remove(*pos)==0)
	{
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "                          %s removed.\n", (*pos).data());
	  LockFiles.erase(pos); --pos;
	} else {
	  kDebugInfo("                          could not remove  %s.\n", (*pos).data());
	}
    }
  // -----
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::CleanLockFiles: done.\n");
  // ############################################################################
}

void 
QConfigDB::watch(bool state)
{
  // ############################################################################
  if(timer==0)
    {
      if(state==true)
	{
	  timer=new QTimer(this); 
	  connect(timer, SIGNAL(timeout()), SLOT(checkFileChanged()));
	  timer->start(1000);
	}
    } else {
      if(state==false)
	{
	  delete timer; 
	  timer=0;
	}
    }
  // ############################################################################
}

bool 
QConfigDB::CheckLockFile(const QString& file)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::CheckLockFile: called.\n");
  int pid;
  // -----
  pid=IsLocked(file);
  if(pid==0)
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::CheckLockFile: the file is not locked.\n");
      return false;
    }
  if(pid>0)
    {
      if(kill(pid, 0)!=0) 
	{ // ----- no such process, we may remove the lockfile:
	  return false;
	}
    }
  if(pid<0)
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::CheckLockFile: the file has not been created by "
	 "QConfigDB::lock.\n");
    }
  // ----- check system time and creation time of lockfile:
  // WORK_TO_DO: not implemented
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::CheckLockFile: done.\n");
  return true;
  // ############################################################################
}
  
bool 
QConfigDB::checkFileChanged()
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  // kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::checkFileChanged: called.\n");
  if(filename.isEmpty()) 
    { // ----- false, as file does not exist and thus may be stored anyway
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::checkFileChanged: no filename.\n");
      return false;
    }
  QFileInfo file(filename);
  // -----
  if(file.exists())
    {
      if(file.lastModified() > *mtime)
	{
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::checkFileChanged: file has been changed.\n");
	  emit(fileChanged());
	  return true;
	} else {
	  return false;
	}
    } else {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::checkFileChanged: could not stat file, "
	 "file does not exist.\n");
      if(!mtime->isValid())
	{ // the file did never exist for us:
	  return false; // ... so it has not changed
	} else { // it existed, and now it does no more
	  emit(fileChanged());
	  return true;
	}
    }
  // ############################################################################
}

bool 
QConfigDB::storeFileAge()
{
  register bool GUARD; GUARD=false;
  REQUIRE(!filename.isEmpty());
  // ############################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::storeFileAge: called.\n");
  QFileInfo file(filename);
  // -----
  if(file.exists())
    {
      *mtime=file.lastModified();
      return true;
    } else {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::save: could not stat file.\n");
      *mtime=QDateTime(); // a null date
      return false;
    }  
  // ############################################################################
}  


bool 
QConfigDB::setFileName(const QString& filename_, bool mustexist, bool readonly_)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::setFileName: setting filename to \"%s\"%s.\n",
     filename_.data(), readonly_ ? " (read only)" : "");
  // ----- remove previous lock:
  if(locked)
    {
      if(!unlock())
	{
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::setFileName: cannot release previous lock.\n");
	  return false;
	}
    }
  // ----- remove possible stale lockfile:
  if(IsLocked(filename_)!=0 && !CheckLockFile(filename_))
    { // ----- it is stale:
      if(::remove(filename_+".lock")==0)
	{
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::setFileName: removed stale lockfile.\n");
	} else {
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::setFileName: cannot remove stale lockfile.\n");
	  return false;
	}
    }
  // -----
  if(mustexist)
    {
      if(access(filename_, readonly_==true ? R_OK : W_OK | R_OK)==0)
	{ 
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::setFileName: permission granted.\n");
	  if(!readonly_)
	    { //       we need r/w access:
	      if(lock(filename_))
		{
		  locked=true;
		} else {
		  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::setFileName: "
		     "could not lock the file.\n");
		  return false;
		}
	    }
	  readonly=readonly_;
	  filename=filename_;
	  storeFileAge(); CHECK(storeFileAge());
	  return true;
	} else {
	  kDebugInfo("QConfigDB::setFileName: permission denied, "
	    "filename not set.\n                       "
	    "(hint: file must exist, but it does not)\n");
	  return false;
	}
    } else {
      if(access(filename_, F_OK)==0)
	{
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::setFileName: file exists.\n");
	  if(access(filename_, W_OK | R_OK)==0)
	    {
	      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::setFileName: permission granted.\n");
	      if(!readonly_)
		{ //       we need r/w access:
		  if(lock(filename_))
		    {
		      locked=true;
		    } else {
		      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::setFileName: "
			 "could not lock the file.\n");
		      return false;
		    }
		}
	      readonly=readonly_;
	      filename=filename_;
	      storeFileAge(); CHECK(storeFileAge());	      
	      return true;
	    } else {
	      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::setFileName: "
		 "permission denied, filename not set.\n");
	      return false;	      
	    }
	} else {
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::setFileName: permission granted, new file.\n");
	  readonly=readonly_;
	  filename=filename_;
	  if(!readonly)
	    {
	      if(!lock())
		{
		  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::setFileName: could not lock the file.\n");
		  return false;
		}
	    }
	  storeFileAge();
	  return true;
	}
    }
  // ############################################################################
}

QString 
QConfigDB::fileName()
{
  // ############################################################################
  return filename;
  // ############################################################################
}

bool 
QConfigDB::save(const char* header, bool force)
{
  register bool GUARD; GUARD=true;
  REQUIRE(!filename.isEmpty());
  // ############################################################################
  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::save: saving database -->%s<--.\n", filename.data());
  bool wasRO=false;
  bool rc;
  // -----
  if(checkFileChanged())
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::save: file is newer, not saving.\n");
      return false;
    }
  if(force && isRO())
    {
      if(setFileName(fileName(), true, false))
	{
	  wasRO=true;
	  CHECK(!isRO());
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::save: switched to (forced) r/w mode.\n");
	} else {
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::save: cannot switch to (forced) r/w mode.\n");
	  return false; 
	}
    }
  // ----- now save it:
  if(!isRO())
    {
      QFile file(filename);
      if(file.open(IO_WriteOnly))
	{
	  QTextStream stream(&file);
	  // -----
	  if(header!=0)
	    {
	      stream << "# " << header << endl;
	    }
	  stream << '#' << " [File created by QConfigDB object " 
		 << version() << "]" << endl;
	  if(!top.save(stream)) // traverse tree
	    {
	      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::save: error saving subsections.\n");
	    }
	  storeFileAge(); CHECK(storeFileAge());
	  file.close();
	  rc=true;
	} else {
	  kDebugInfo("QConfigDB::save: error opening file \"%s\" for writing.\n",
	    filename.data());
	  rc=false;
	}
    } else {
      rc=false;
    }
  // ----- reset r/o mode:
  if(wasRO) // only true if we switched to forced r/w mode here
    {
      if(setFileName(fileName(), false, true))
	{
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::save: reset (forced) r/w mode.\n");
	} else {
	  kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::save: cannot reset (forced) r/w mode.\n");
	  rc=false; 
	}
    }
  // -----
  return rc;
  // ############################################################################
}

bool 
QConfigDB::load()
{
  register bool GUARD; GUARD=true;
  REQUIRE(!filename.isEmpty());
  // ############################################################################
  QFile file(filename);
  QCString line;
  // -----
  if(file.open(IO_ReadOnly))
    {
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::load: file access OK.\n");
      QTextStream stream(&file);
      // -----
      clear();
      CHECK(empty());
      bool rc=top.readSection(stream, false);
      storeFileAge(); CHECK(storeFileAge());
      file.close();
      emit(changed(this));
      kDebugInfo(GUARD, KAB_KDEBUG_AREA, "QConfigDB::load: done.\n");
      return rc;
    } else {
      kDebugInfo("QConfigDB::load: error opening file \"%s\" for reading.\n",
	filename.data());
      return false;
    }
  // ############################################################################
}
