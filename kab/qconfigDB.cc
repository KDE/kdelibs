/* the Configuration Database library, Version II
 
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
#include <qtimer.h>
#include <qdatetime.h>
#include <qfileinfo.h>

#include "qconfigDB.moc"
#include <kdebug.h>

#ifdef KAB_KDEBUG_AREA
#undef KAB_KDEBUG_AREA
#endif

#define KAB_KDEBUG_AREA 800

static bool isComment(QCString line)
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

static void tokenize(list<QCString>& res, const QCString& text, char tr, bool strict=false)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "tokenize: called." << endl;
  int eins=0, zwei=0;
  QCString teil;
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "tokenize: partening -->%" << text.data() << "<--." << endl;
  res.erase(res.begin(), res.end());
  // -----
  if(text.isEmpty())
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "tokenize: text is an empty string, done." << endl;
      return;
    }
  while(zwei!=-1)
    {
      teil="";
      zwei=text.find(tr, eins);
      if(zwei!=-1)
	{
	  teil=text.mid(eins, zwei-eins);
	  res.push_back(teil);
	} else { // last element
	  if(!strict) // nur wenn dazwischen Zeichen sind
	    {
	      teil=text.mid(eins, text.length()-eins);
	      res.push_back(teil);
	    }
	}
      eins=zwei+1;
      // if((unsigned)eins>=text.length()) break;
    }
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "tokenize: partened in "
				  << res.size() << " parts.\n";
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "tokenize: done." << endl;
  // ############################################################################
}

// QCString AuthorEmailAddress; // assign your email address to this string

static QCString ReadLineFromStream(QTextStream& stream)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "ReadLineFromStream:: reading line." << endl;
  QCString line;
  // -----
  while(!stream.eof())
    {
      line=stream.readLine().ascii();
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "ReadLineFromStream:: line \"" << line.data() << "\" read.\n";
  return line;
  // ############################################################################
}

// class implementations:

list<QString> QConfigDB::LockFiles; // the lockfiles created by this session

KeyValueMap::KeyValueMap()
  : data(new StringStringMap)
{
  // ###########################################################################
  // ###########################################################################
}

KeyValueMap::KeyValueMap(const KeyValueMap& orig)
  : data(new StringStringMap(*orig.data))
{
  // ###########################################################################
  // ###########################################################################
}

KeyValueMap::~KeyValueMap()
{
  // ###########################################################################
  delete data;
  // ###########################################################################
}

bool KeyValueMap::invariant()
{
  return true;
}

StringStringMap::iterator KeyValueMap::begin()
{
  return data->begin();
}

StringStringMap::iterator KeyValueMap::end()
{
  return data->end();
}

unsigned int
KeyValueMap::size() const
{
  // ###########################################################################
  return data->size();
  // ###########################################################################
}

void
KeyValueMap::clear()
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	     "KeyValueMap::clear: erasing map contents ... " << endl;
  // -----
  if(!data->empty()) // erase fails on empty containers!
    {
      data->erase(data->begin(), data->end());
    }
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "done." << endl;
  // ###########################################################################
}

bool
KeyValueMap::fill(const QString& filename, bool force, bool relax)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  QFile file(filename);
  QCString line;
  // -----
  if(file.open(IO_ReadOnly))
    {
      QTextStream stream(&file);
      // We read/write utf8 strings, so we don't want that QTextStream uses local8bit
      // Latin1 means : no conversion, when giving char*s to a QTextStream. (DF)
      stream.setEncoding(QTextStream::Latin1);
      // -----
      while(!stream.eof())
	{
	  line=stream.readLine().ascii();
	  if(!line.isEmpty() /* && !stream.eof() */ && !isComment(line))
	    {
	      if(!insertLine(line, force, relax, false))
		{
		    kdDebug(GUARD, KAB_KDEBUG_AREA) <<
			"KeyValueMap::fill: could not insert line "
						    << line << ".\n"; // ignore this case further
		}
	    }
	}
      file.close();
      // -----
      return true;
    } else {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	  "KeyValueMap::fill: cannot open file " <<
	  filename << endl;
      return false;
    }
  // ###########################################################################
}

bool
KeyValueMap::save(const QString& filename, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
      "KeyValueMap::save: saving data to -->" <<
      filename << "<--.\n";
  StringStringMap::iterator pos;
  QFile file(filename);
  // ----- open file, regarding existence:
  if(!force)
    {
      if(file.exists())
	{
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		     "KeyValueMap::save: file exists but may not." << endl;
	  return false;
	}
    }
  if(file.open(IO_WriteOnly))
    {
      QTextStream stream(&file);
      stream.setEncoding(QTextStream::Latin1); // no conversion
      stream << "# saved by KeyValueMap object ($Revision$)" << endl;
      for(pos=data->begin(); pos!=data->end(); ++pos)
	{ // values do not get coded here
	  stream << (*pos).first << '=' << (*pos).second << endl;
	}
      file.close();
    } else {
	kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	    "KeyValueMap::save: could not open file -->%s<-- for saving." <<
	    filename.utf8() << endl;
      return false;
    }
  // ###########################################################################
  return true;
}

bool
KeyValueMap::save(QTextStream& file, int count)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	     "KeyValueMap::save: saving data to given output stream." << endl;
  StringStringMap::iterator pos;
  bool ret=true;
  char* prefix=new char[count+1];
  memset(prefix, ' ', count);
  prefix[count]=0;
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
  return rc;
  // ###########################################################################
}


bool
KeyValueMap::empty()
{
  // ###########################################################################
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
  QCString temp(2*orig.length());
  QCString mod;
  int count=1;
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
    "KeyValueMap::parseComplexString: parsing the string -->"
				  << orig << "<--.\n";
  // -----
  if(orig.isEmpty())
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "KeyValueMap::parseComplexString: string is empty.\n"
		 "                                 "
		 "This is no valid complex string." << endl;
      return false;
    }
  // ----- prepare the string:
  temp=orig.mid(index, orig.length()-index); // remove everything before index
  mod=temp.stripWhiteSpace(); // remove leading and trailing white spaces
  // ----- test some conditions:
  if(mod.length()<2)
    {
      kdDebug() << "KeyValueMap::parseComplexString: no pair of brackets " << endl;
      return false;
    }
  if(mod[0]!='"')
    {
      kdDebug() << "KeyValueMap::parseComplexString: no opening bracket." << endl;
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
	  kdDebug(GUARD, KAB_KDEBUG_AREA).form("KeyValueMap::parseComplexString: found "
					       "a special character \"%c\".", mod[first]) << endl;
	  if((unsigned)first==mod.length())
	    {
		kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		    "KeyValueMap::parseComplexString: "
		    "string lacks the closing \".\n          "
		    "                       This is no valid "
		    "complex string." << endl;
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
		kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::parseComplexString: "
		    "invalid control character.\n            "
		    "                     This is no valid complex string." << endl;
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
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::parseComplexString: "
	    "string lacks the closing \".\n              "
	    "                   This is no valid complex string.\n";
	  return false;
	}
    }
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA).form(
	     "KeyValueMap::parseComplexString: finished parsing, no errors, "
	     "%i characters, %i in string.", count, temp.length()) << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
      "KeyValueMap::makeComplexString: coding the string\n           -->"
				  << orig <<
      "<--\n                                into a complex string.\n";
  QCString temp(2*orig.length());
  unsigned int count;
  // -----
  temp+='"'; // opening bracket
  for(count=0; count<orig.length(); count++)
    {
      switch(orig[count])
	{ // catch all special characters:
	case '"':
	kdDebug(GUARD, KAB_KDEBUG_AREA).form("KeyValueMap::makeComplexString: "
	"found the special char \"%c\".", orig[count]) << endl;
	  temp+='\\';
	  temp+='"';
	  break;
	case '\n':
	kdDebug(GUARD, KAB_KDEBUG_AREA).form("KeyValueMap::makeComplexString: "
	"found the special char \"%c\".", orig[count]) << endl;
	  temp+='\\';
	  temp+='n';
	  break;
	case '\t':
	  kdDebug(GUARD, KAB_KDEBUG_AREA).form("KeyValueMap::makeComplexString: "
	  "found the special char \"%c\".", orig[count]) << endl;
	  temp+='\\';
	  temp+='t';
	  break;
	case '\\':
	kdDebug(GUARD, KAB_KDEBUG_AREA).form("KeyValueMap::makeComplexString: "
	"found the special char \"%c\".", orig[count]) << endl;
	  temp+='\\';
	  temp+='\\';
	  break;
	default: temp+=orig[count];
	}
    }
  temp+='"'; // closing bracket
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
      "KeyValueMap::makeComplexString: result is\n           -->"
				  <<temp<<"<--.\n";
  return temp;
  // ###########################################################################
}

bool
KeyValueMap::getRaw(const QCString& key, QCString& value) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
      "KeyValueMap::getRaw: trying to get raw value for key \"" << key << "\" ...\n";
  StringStringMap::iterator pos=data->find(key);
  // -----
  if(pos==data->end())
    {
	kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "not in KeyValueMap." << endl;
      return false;
    } else {
      value=(*pos).second;
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "in KeyValueMap, value is "
				      << value << endl;
      return true;
    }
  // ###########################################################################
}

bool
KeyValueMap::insertRaw(const QCString& key, const QCString& value, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
      "KeyValueMap::insertRaw: inserting uncoded value "
				  << value <<
      " for key " << key << endl;
  int n=0;
  // -----
  if(key.isEmpty()) // empty KEYS are errors:
    {
      kdDebug() << "KeyValueMap::insertRaw: tried to insert empty key." << endl;
      return false;
    }
  if(force) // entry will be replaced
    {
      n=data->erase(key);
    }
  if(data->insert(StringStringMap::value_type(key, value)).second)
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insertRaw: success"
				      << (n==0 ? "" : " (forced)") << endl;
      return true;
    } else {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insertRaw: failed, "
	  "key already in KeyValueMap." << endl;
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
    kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	"KeyValueMap::insert: inserting value\n           -->"
				    << value <<
	"<-- \""
	"                     for key\n           -->"
				    << key <<
	"<--.\n";
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
      "KeyValueMap::insertLine: inserting line -->"<<line<<"<--.\n";
  int index;
  QCString key;
  QCString value;
  // ----- is the line empty or does it contain only whitespaces?
  for(index=0; isspace(line[index]) && (unsigned)index<line.length(); ++index);
  if(line.isEmpty() || (unsigned)index==line.length())
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	  "KeyValueMap::insertLine: line is empty." << endl;
      return false;
    }
  // -----
  index=line.find('=');
  if(index==-1)  // not found
      {
	  kdDebug() << "KeyValueMap::insertLine: no \"=\" found in \""<<line<<"\".\n";
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
	kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insertLine: done." << endl;
      if(encode)
	{ // the usual way:
	  return insert(key, value, force);
	} else { // while loading from a already coded file:
	  return insertRaw(key, value, force);
	}
    } else {
      kdDebug() << "KeyValueMap::insertLine: key " << (relax ? "" : "or value ") << " is empty." << endl;
      return false;
    }
  // ###########################################################################
}

bool
KeyValueMap::get(const QCString& key, QCString& value) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[string]: "
      "trying to get value for key \"" << key << "\" ... " << endl;
  QCString raw;
  QCString temp;
  // -----
  if(!getRaw(key, raw))
    {
      return false; // key does not exist
    } else {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[string]: checking "
		 "wether this is a complex string." << endl;
      {
	int count;
	if(parseComplexString(raw, 0, temp, count))
	  {
	    kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		       "KeyValueMap::get[string]: complex string found." << endl;
	    value=temp;
	  } else {
	    kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		       "KeyValueMap::get[string]: this is no complex string." << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insert[QString]: trying to "
      "insert \"" << (!value.isNull() ? "true" : "false")
				  << "\" for key\n           -->"
				  << v
				  << "<--.\n";
  return insert(key, v, force);
  // ###########################################################################
}

bool
KeyValueMap::get(const QCString& key, QString& value) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[QString]: trying to get "
      "a QString value for key " << key << endl;
  QCString v;
  // ----- get string representation:
  if(!get(key, v))
    {
	kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[QString]: key "
					<< key << " not in KeyValueMap.\n";
	return false;
    }
  // ----- find its state:
  value=QString::fromUtf8(v); // is there a better way?
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[QString]: success, value"
	     " (in UTF8) is " << v << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insert[bool]: trying to "
      "insert \""
				  << (value==true ? "true" : "false")
				  <<"\" for key\n           -->"
				  << key << "<--.\n";
  return insert(key, value==true ? "true" : "false", force);
  // ###########################################################################
}


bool
KeyValueMap::get(const QCString& key, bool& value) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[bool]: trying to get "
      "BOOL value for key " << key << endl;
  QCString v;
  // ----- get string representation:
  if(!get(key, v))
    {
	kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[bool]: key "
					<< key << " not in KeyValueMap.";
      return false;
    }
  // ----- find its state:
  v=v.stripWhiteSpace();
  if(v=="true")
    {
	kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[bool]: success, "
	    "value is TRUE." << endl;
      value=true;
      return true;
    }
  if(v=="false")
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[bool]: success, "
      "value is FALSE." << endl;
      value=false;
      return true;
    }
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	     "KeyValueMap::get[bool]: failure, unknown value." << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insert[int]: trying to "
      "insert value \""<<value << "\" for key\n           -->"<<key<<"<--.\n";
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[int]: trying to get "
      "INTEGER value for key " << key << endl;
  QCString v;
  bool ok;
  long temp;
  // -----
  if(!get(key, v))
    {
	kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[int]: key "
					<< key <<" not in KeyValueMap.\n";
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insert[long int list]: "
	     "trying to insert long int list into map." << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insert[long int list]: "
  "constructed string value is " << value << endl;
  return insert(key, value, force);
  // ###########################################################################
}

bool
KeyValueMap::get(const QCString& key, list<long>& values) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[long int list]: trying "
      "to decode int list for key " << key << endl;
  kdDebug(!values.empty(), KAB_KDEBUG_AREA) << "KeyValueMap::get[long int list]"
      ": attention - list should be empty but is not.\n";
  QCString value;
  list<QCString> tokens;
  list<QCString>::iterator pos;
  long temp;
  bool ok;
  // -----
  if(!get(key, value))
  {
    kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	       "KeyValueMap::get[long int list]: no such key." << endl;
    return false;
  }
  tokenize(tokens, value, ',');
  if(tokens.empty())
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "KeyValueMap::get[long int list]: no tokens." << endl;
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
	  kdDebug() << "KeyValueMap::get[long int list]: conversion error for " << endl;
	}
    }
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[long int list]: done." << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insert[int list]: trying to "
	     "insert int list into map." << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insert[int list]: "
      "constructed string value is " << value << endl;
  return insert(key, value, force);
  // ###########################################################################
}

bool
KeyValueMap::get(const QCString& key, list<int>& values) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[int list]: trying to "
      "decode int list for key " << key << endl;
  kdDebug(!values.empty(), KAB_KDEBUG_AREA) << "KeyValueMap::get[int list]: "
	     "attention - list should be empty but is not.\n";
  QCString value;
  list<QCString> tokens;
  list<QCString>::iterator pos;
  int temp;
  bool ok;
  // -----
  if(!get(key, value))
  {
    kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	       "KeyValueMap::get[int list]: no such key." << endl;
    return false;
  }
  tokenize(tokens, value, ',');
  if(tokens.empty())
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "KeyValueMap::get[int list]: no tokens." << endl;
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
	    kdDebug() << "KeyValueMap::get[int list]: conversion error for " << *pos << endl;
	}
    }
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[long int list]: done." << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA).form("KeyValueMap::insert[double]: trying to "
				       "insert value \"%f\" for key\n           -->", value) << key << "<--.\n";
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[double]: trying to get "
      "FLOAT value for key " << key << endl;
  QCString v;
  bool ok;
  double temp;
  // -----
  if(!get(key, v))
    {
	kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[int]: key "
					<<key<<" not in "
		 "KeyValueMap." << endl;
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
  kdDebug(!values.empty(), KAB_KDEBUG_AREA) << "KeyValueMap::get[string list]: "
      "attention!\n             \"values\" list reference is not "
      "empty!" << endl;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[string list]: trying to "
      "decode string list for key " << key << endl;
  QCString raw, part, value;
  int first=1, second=1, i;
  // ----- get the string value as a whole:
  if(!getRaw(key, raw))
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[list<string>]: key "
				      << key << " not in KeyValueMap." << endl;
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
	      // we use "\e" as token for the string-delimiter
	      if(raw[second+1]=='e' // the right character
		 && raw[second-1]!='\\') // not escaped
		{
		  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get"
			     "[list<string>]: found string end at pos " <<
			     second << endl;
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
	      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get"
		  "[list<string>]: found item " << value << endl;
	      values.push_back(value);
	    } else {
	      kdDebug() << "KeyValueMap::get[list<string>]: parse error." << endl;
	      return false;
	    }
	  if((unsigned)second<raw.length()-3)
	    { // ----- there may be another string
	      first=second+2;
	    } else { // ----- we are completely finished
	      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
			 "KeyValueMap::get[list<string>]: list end found." << endl;
	      break;
	    }
	} else { // ----- finished:
	  break;
	}
    }
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[list<string>]: done." << endl;
  return true;
  // ###########################################################################
}

bool
KeyValueMap::insert(const QCString& key, const list<QCString>& values, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insert[string list]: "
	     "coding string list." << endl;
  QCString value="\"";
  QCString temp;
  list<QCString>::const_iterator pos;
  // ----- create coded string list:
  for(pos=values.begin();
      pos!=values.end();
      pos++)
    { // create strings like "abc\efgh\eijk":
      temp=makeComplexString(*pos);
      temp.remove(0, 1); // remove the leading "\""
      temp.remove(temp.length()-1, 1); // the trailing "\""
      value+=temp;
      value+="\\e";
    }
  value+="\""; // finish the string
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insert[string list]: result "
      "of coding is " << value << endl;
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
  kdDebug(!values.isEmpty(), KAB_KDEBUG_AREA) << "KeyValueMap::get[QStrList]: "
      "attention!\n             \"values\" list reference is not "
      "empty!" << endl;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[QStrList]: trying to "
      "decode string list for key " << key << endl;
  QCString raw, part, value;
  int first=1, second=1, i;
  // ----- get the string value as a whole:
  if(!getRaw(key, raw))
    {
	kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[QStrList]: key "
					<< key <<" not in KeyValueMap." << endl;
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
	      // we use "\e" as token for the string-delimiter
	      if(raw[second+1]=='e' // the right character
		 && raw[second-1]!='\\') // not escaped
		{
		  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[QStrList]:"
		      " found string end at pos %i." << second << endl;
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
	      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[QStrList]: "
		  "found item " << value << endl;
	      values.append(value);
	    } else {
		kdDebug() << "KeyValueMap::get[QStrList]: parse error." << endl;
	      return false;
	    }
	  if((unsigned)second<raw.length()-3)
	    { // ----- there may be another string
	      first=second+2;
	    } else { // ----- we are completely finished
	      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[QStrList]: "
			 "list end found." << endl;
	      break;
	    }
	} else { // ----- finished:
	  break;
	}
    }
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[QStrList]: done." << endl;
  return true;
  // ###########################################################################
}

bool
KeyValueMap::insert(const QCString& key, const QStrList& values, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	     "KeyValueMap::insert[QStrList]: coding string list." << endl;
  QCString value="\"";
  QCString temp;
  unsigned int count;
  // ----- create coded string list:
  for(count=0; count<values.count(); ++count)
    { // create strings like "abc\efgh\eijk":
      temp=makeComplexString(((QStrList)values).at(count));
      temp.remove(0, 1); // remove the leading "\""
      temp.remove(temp.length()-1, 1); // the trailing "\""
      value+=temp;
      value+="\\e";
    }
  value+="\""; // finish the string
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
      "KeyValueMap::insert[QStrList]: result of coding is %s." <<
      value << endl;
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
  kdDebug(!values.isEmpty(), KAB_KDEBUG_AREA) << "KeyValueMap::get"
      "[QStringList]: attention!\n             \"values\" list reference"
      " is not empty!" << endl;
  // ###########################################################################
  /* The values are stored in a utf8-coded set of QCStrings.
     This list is retrieved and converted back to Unicode strings. */
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[QStringList]: trying to "
      "decode QStringList for key " << key << endl;
  QStrList temp;
  unsigned int count;
  // ----- get the plain C strings:
  if(!get(key, temp))
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[QStringList]: key "
				      << key <<
	  " not in KeyValueMap." << endl;
      return false;
    }
  // ----- do the conversion:
  for(count=0; count<temp.count(); ++count)
    {
      values.append(QString::fromUtf8(temp.at(count)));
    }
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[QStringList]: done." << endl;
  return true;
  // ###########################################################################
}

bool
KeyValueMap::insert(const QCString& key, const QStringList& values, bool force)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	     "KeyValueMap::insert[QStringList]: coding QStringList." << endl;
  // The method simply creates a list of utf8-coded strings and inserts it.
  QStrList utf8strings;
  unsigned int count;
  // ----- create QCString list:
  for(count=0; count<values.count(); ++count)
    {
      utf8strings.append((*values.at(count)).utf8());
    }
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insert[QStringList]: done." << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insert[double list]: trying "
	     "to insert double list into map." << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insert[double list]: "
      "constructed string value is " << value << endl;
  return insert(key, value, force);
  // ###########################################################################
}

bool
KeyValueMap::get(const QCString& key, list<double>& values) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[double list]: trying to "
      "decode double list for key " << key << endl;
  kdDebug(!values.empty(), KAB_KDEBUG_AREA) << "KeyValueMap::get[double list]: "
      "attention - list should be empty but is not." << endl;
  QCString value;
  list<QCString> tokens;
  list<QCString>::iterator pos;
  double temp;
  bool ok;
  // -----
  if(!get(key, value))
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "KeyValueMap::get[double list]: no such key." << endl;
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
	    kdDebug() << "KeyValueMap::get[double list]: conversion error for "
		      << *pos << endl;
	}
    }
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[int list]: done." << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insert[QDate]: trying to "
	     "insert QDate into map." << endl;
  list<long> values;
  // -----
  if(!value.isValid())
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::insert[QDate]: invalid "
		 "date, inserting a null date." << endl;
      for(int i=0; i<3; ++i) values.push_back(0);
    } else {
      values.push_back(value.year());
      values.push_back(value.month());
      values.push_back(value.day());
    }
  // -----
  return insert(key, values, force);
  // ###########################################################################
}

bool
KeyValueMap::get(const QCString& key, QDate& date) const
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "KeyValueMap::get[QDate]: trying to decode"
      " QDate for key " << key << endl;
  list<long> values;
  long y, m, d;
  QDate temp;
  // -----
  if(!get(key, values))
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "KeyValueMap::get[QDate]: no such key." << endl;
      return false;
    }
  if(values.size()!=3)
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "KeyValueMap::get[QDate]: more or less than 3 values." << endl;
      return false;
    }
  y=values.front(); values.pop_front();
  m=values.front(); values.pop_front();
  d=values.front();
  // -----
  if(y!=0 || m!=0 || d!=0) temp.setYMD(y, m, d); // avoid QDate messages
  if(!temp.isValid() && !temp.isNull())
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "KeyValueMap::get[QDate]: no valid date." << endl;
      return false;
    } else {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "KeyValueMap::get[QDate]: done." << endl;
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
  // ###########################################################################
}

bool
Section::add(const QCString& name)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "Section::add: adding section \""
				  <<name<<"\" to "
      "this section ..." << endl;
  Section* section;
  bool rc;
  // -----
  if(name.isEmpty())
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "Section::add: empty key." << endl;
      return false;
    }
  section=new Section; // create an empty section
  if(section==0)
    {
	kdDebug() << "Section::add: out of memory." << endl;
      return false;
    }
  rc=add(name, section);
  if(!rc)
    {
	kdDebug(GUARD && !rc, KAB_KDEBUG_AREA) << " failed.\n";
	delete section;
	return false;
    } else {
	kdDebug(GUARD && rc, KAB_KDEBUG_AREA) << " success.\n";
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
	kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	    "Section::add: added section "<<name<<" successfully.\n";
      return true;
    } else {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "Section::add: failed to add section "
				      << name << ", section already exists.\n";
      return false;
    }
  // ###########################################################################
}

bool
Section::find(const QCString& name, StringSectionMap::iterator& result)
{
  register bool GUARD; GUARD=false;
  // ###########################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "Section::find: trying to get section "
      "\""<<name<<"\" ... \n";
  StringSectionMap::iterator pos;
  // -----
  pos=sections.find(name);
  if(pos==sections.end())
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "failed, no such section." << endl;
      return false;
    } else {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "success." << endl;
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
	  kdDebug() << "Section::save: error saving child section \"" << (*pos).first.data() << "\"." << endl;
	  return false;
	} else {
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "Section::save: saved section \""
					  << (*pos).first
					  << "\".\n";
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
	  kdDebug() << "Section::save: error saving key-value-pairs." << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "Section::readSection: reading section." << endl;
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
	      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		  "Section::readSection: EOF, line is \""<<line<<"\".\n";
	      if(!line.isEmpty())
		{
		  if(!keys.insertLine(line, false, true, false))
		    {
		      kdWarning() << "Attention: unable to parse key-value-pair "
			   << endl << "\t\"" << line << "\"," << endl
			   << "ignoring and continuing (maybe duplicate "
			   << "declaration of the key)."
			   << endl;
		    }
		}
	      if(finish==true)
		{
		  kdDebug() << "Section::readSection: missing end of section." << endl;
		  return false;
		} else {
		  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
			     "Section::readSection: EOF (no error)." << endl;
		  return true;
		}
	    }
	}
      if(isBeginOfSection(line))
	{
	  name=nameOfSection(line);
	  add(name);
	  find(name, temp);
	  if(!temp->readSection(file))
	    {
		kdDebug() << "Section::readSection: unable to read "
		    "subsection \"" << name << "\".\n";
	      return false;
	    }
	} else { // ----- it has to be a key-value-pair:
	  if(!keys.insertLine(line, false, true, false))
	    {
	      kdWarning() << "Attention: unable to parse key-value-pair " << endl
		   << "\t\"" << line << "\"," << endl
		   << "ignoring and continuing (maybe duplicate declaration of"
		   << " the key)."
		   << endl;
	    }
	}
    }
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
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "Section::isBeginOfSection: too short "
		 "or empty line." << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "Section::isEndOfSection: is "
				  << line <<" the end of"
	     " a section?" << endl;
  int first=1, second;
  QCString temp;
  // -----
  line=line.simplifyWhiteSpace();
  if(line.isEmpty() || line.length()<2)
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "Section::isBeginOfSection: too short "
		 "or empty line." << endl;
      return false;
    }
  if(line[0]!='[' || line[line.length()-1]!=']')
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "Section::isBeginOfSection: does not match." << endl;
      return false;
    }
  // ----- find the word inside the brackets:
  for(first=1; line[first]==' '; ++first); // find first non-whitespace character
  for(second=first; line[second]!=' ' && line[second]!=']'; ++second);
  temp=line.mid(first, second-first);
  if(temp=="END")
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "Section::isBeginOfSection: yes, it is." << endl;
      return true;
    } else {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "Section::isBeginOfSection: no, it is not." << endl;
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
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "Section::isBeginOfSection: too short or empty line." << endl;
      return "";
    }
  if(temp[0]!='[' || temp[temp.length()-1]!=']')
    {
      return "";
    }
  // ----- find the word inside the brackets:
  for(first=1; temp[first]==' '; ++first); // find first non-whitespace character
  for(second=first; temp[second]!=' ' && temp[second]!=']'; ++second);
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
  // -----
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
      kdDebug() << "QConfigDB destructor: cannot remove me." << endl;
    }
  if(locked)
    {
      unlock();
    }
  // ############################################################################
}

bool QConfigDB::invariant()
{
  return true;
}

bool
QConfigDB::get(const list<QCString>& key, KeyValueMap*& map)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::get: trying to get keys ... " << endl;
  Section* section=&top;
  list<QCString>::const_iterator pos;
  // -----
  if(key.empty())
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "ConfigDB::get: path is empty, returning toplevel section." << endl;
      map=top.getKeys();
      return true;
    }
  for(pos=key.begin(); pos!=key.end(); pos++)
    {
      if(!section->find(*pos, section))
	{
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		     "failed,\n               at least the element \""
					  << *pos <<
	      "\" of "
		     "the key-list is not declared." << endl;
	  return false;
	}
    }
  // -----
  map=section->getKeys();
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "success." << endl;
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
{
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::stringToKeylist: parsing path " << desc << endl;
  // ############################################################################
  list<QCString> key;
  int first=0, second;
  QCString temp;
  // -----
  if(desc.isEmpty())
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "QConfigDB::stringToKeylist: path is empty." << endl;
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
	      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
			 "QConfigDB::stringToKeylist: found last part "
					      << temp << endl;
	      key.push_back(temp);
	    }
	  break;
	}
      temp=desc.mid(first, second-first);
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	  "QConfigDB::stringToKeylist: found part " << temp << endl;
      key.push_back(temp);
      first=second+1;
    }
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::stringToKeylist: done." << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::get: searching section ... " << endl;
  Section* temp=&top;
  list<QCString>::const_iterator pos;
  // -----
  for(pos=key.begin(); pos!=key.end(); pos++)
    {
      if(!temp->find(*pos, temp))
	{
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "failure, no such section.";
	  return false;
	}
    }
  // -----
  section=temp;
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "success, section found." << endl;
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
  if(access(QFile::encodeName(lockfile), F_OK)==0)
    {
      QFile f(lockfile);
      // -----
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::IsLocked: the file\n"
				      << file <<
	  "\nhas a lockfile.\n";
      if(f.open(IO_ReadOnly))
	{
	  QTextStream stream(&f);
	  stream.setEncoding(QTextStream::Latin1); // no conversion
	  // -----
	  stream >> pid;
	  if(pid==-1)
	    {
	      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::IsLocked: the file "
			 "does not contain the ID\n        of the process that "
			 "created it." << endl;
	      return -1;
	    }
	  f.close();
	} else {
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		     "QConfigDB::IsLocked: cannot open the lockfile." << endl;
	  return -1;
	}
      return pid;
    } else {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::IsLocked: the file\n"
				      << file << "\nhas no lockfile.\n";
      return 0;
    }
  // ############################################################################
}

bool
QConfigDB::lock()
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  if(locked)
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::lock (current file): file "
		 "is already locked by this object." << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::lock: locking the file "
				  << file << endl;
  QString lockfile=file+".lock";
  QFile f(lockfile);
  // -----
  if(access(QFile::encodeName(lockfile), F_OK)==0)
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::lock: the file is locked by"
		 " another process." << endl;
      return false;
    } else {
      if(f.open(IO_WriteOnly))
	{
	  QTextStream stream(&f);
	  stream.setEncoding(QTextStream::Latin1); // no conversion
	  // -----
	  stream << getpid() << endl;
	  f.close();
	} else {
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::lock: unable to create"
		     " lockfile." << endl;
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
  // ############################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::unlock: unlocking the file "
				  << filename << endl;
  QString lockfile=filename+".lock";
  list<QString>::iterator pos;
  // -----
  if(!locked)
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::unlock: this app did not "
		 "lock the file!" << endl;
      return false;
    }
  if(access(QFile::encodeName(lockfile), F_OK | W_OK)==0)
    {
      if(::remove(QFile::encodeName(lockfile))==0)
	{
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		     "QConfigDB::unlock: lockfile deleted." << endl;
	  for(pos=LockFiles.begin(); pos!=LockFiles.end(); ++pos)
	    {
	      if((*pos)==lockfile) break;
	    }
	  if(pos!=LockFiles.end())
	    {
	      LockFiles.erase(pos); --pos;
	    } else {
	      kdDebug() << "QConfigDB::unlock: file not mentioned in lockfile" << endl;
	    }
	  locked=false;
	  return true;
	} else {
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::unlock: unable to "
		     "delete lockfile.n" << endl;
	  return false;
	}
    } else {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::unlock: the file is not"
		 " locked or permission has been denied." << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA).form("QConfigDB::CleanLockFiles: removing %i "
	     "remaining lockfiles.", LockFiles.size()) << endl;
  for(pos=LockFiles.begin(); pos!=LockFiles.end(); pos++)
    {
      if(::remove(QFile::encodeName(*pos))==0)
	{
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	      "                          " << *pos << " removed.\n";
	  LockFiles.erase(pos); --pos;
	} else {
	    kdDebug() << "                          could not remove  " << *pos << endl;
	}
    }
  // -----
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::CleanLockFiles: done." << endl;
  // ############################################################################
}

void
QConfigDB::watch(bool state)
{
  // ############################################################################
  if(state)
    { // start timer
      if(timer==0)
	{
	  timer=new QTimer(this);
	  connect(timer, SIGNAL(timeout()), SLOT(checkFileChanged()));
	}
      timer->start(1000);
    } else { // stop timer
      if(timer!=0)
	{
	  timer->stop();
	}
    }
  // ############################################################################
}

bool
QConfigDB::CheckLockFile(const QString& file)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::CheckLockFile: called." << endl;
  int pid;
  // -----
  pid=IsLocked(file);
  if(pid==0)
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::CheckLockFile: the file is "
		 "not locked." << endl;
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
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::CheckLockFile: the file has "
		 "not been created by QConfigDB::lock." << endl;
    }
  // ----- check system time and creation time of lockfile:
  // WORK_TO_DO: not implemented
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::CheckLockFile: done." << endl;
  return true;
  // ############################################################################
}

bool
QConfigDB::checkFileChanged()
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  // kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::checkFileChanged: called." << endl;
  if(filename.isEmpty())
    { // ----- false, as file does not exist and thus may be stored anyway
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::checkFileChanged: no filename." << endl;
      return false;
    }
  QFileInfo file(filename);
  // -----
  if(file.exists())
    {
      if(file.lastModified() > *mtime)
	{
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		     "QConfigDB::checkFileChanged: file has been changed.n" << endl;
	  emit(fileChanged());
	  return true;
	} else {
	  return false;
	}
    } else {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::checkFileChanged: could "
		 "not stat file, file does not exist." << endl;
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
  // ############################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::storeFileAge: called." << endl;
  QFileInfo file(filename);
  // -----
  if(file.exists())
    {
      *mtime=file.lastModified();
      return true;
    } else {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::save: could not stat file." << endl;
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
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::setFileName: setting filename "
      "to \""
				  << filename_ <<"\"" << (readonly_ ? " (read only)" : "") << endl;
  // ----- remove previous lock:
  if(locked)
    {
      if(!unlock())
	{
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::setFileName: cannot "
		     "release previous lock." << endl;
	  return false;
	}
    }
  // ----- remove possible stale lockfile:
  if(IsLocked(filename_)!=0 && !CheckLockFile(filename_))
    { // ----- it is stale:
      if(::remove(QFile::encodeName(filename_+".lock"))==0)
	{
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		     "QConfigDB::setFileName: removed stale lockfile." << endl;
	} else {
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		     "QConfigDB::setFileName: cannot remove stale lockfile." << endl;
	  return false;
	}
    }
  // -----
  if(mustexist)
    {
      if(access(QFile::encodeName(filename_), readonly_==true ? R_OK : W_OK | R_OK)==0)
	{
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		     "QConfigDB::setFileName: permission granted." << endl;
	  if(!readonly_)
	    { //       we need r/w access:
	      if(lock(filename_))
		{
		  locked=true;
		} else {
		  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::setFileName: "
		     "could not lock the file." << endl;
		  return false;
		}
	    }
	  readonly=readonly_;
	  filename=filename_;
	  storeFileAge();
	  return true;
	} else {
	  kdDebug() << "QConfigDB::setFileName: permission denied, " << endl;
	  return false;
	}
    } else {
      if(access(QFile::encodeName(filename_), F_OK)==0)
	{
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::setFileName: file exists." << endl;
	  if(access(QFile::encodeName(filename_), W_OK | R_OK)==0)
	    {
	      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
			 "QConfigDB::setFileName: permission granted." << endl;
	      if(!readonly_)
		{ //       we need r/w access:
		  if(lock(filename_))
		    {
		      locked=true;
		    } else {
		      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::setFileName: "
		      "could not lock the file." << endl;
		      return false;
		    }
		}
	      readonly=readonly_;
	      filename=filename_;
	      storeFileAge();
	      return true;
	    } else {
	      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::setFileName: "
	      "permission denied, filename not set." << endl;
	      return false;
	    }
	} else {
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
	  "QConfigDB::setFileName: permission granted, new file." << endl;
	  readonly=readonly_;
	  filename=filename_;
	  if(!readonly)
	    {
	      if(!lock())
		{
		  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
			     "QConfigDB::setFileName: could not lock the file." << endl;
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
  // ############################################################################
  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
      "QConfigDB::save: saving database -->" << filename << "<--.\n";
  bool wasRO=false;
  bool rc;
  // -----
  if(checkFileChanged())
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		 "QConfigDB::save: file is newer, not saving." << endl;
      return false;
    }
  if(force && isRO())
    {
      if(setFileName(fileName(), true, false))
	{
	  wasRO=true;
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		     "QConfigDB::save: switched to (forced) r/w mode." << endl;
	} else {
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		     "QConfigDB::save: cannot switch to (forced) r/w mode." << endl;
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
	  stream.setEncoding(QTextStream::Latin1); // no conversion
	  // -----
	  if(header!=0)
	    {
	      stream << "# " << header << endl;
	    }
	  stream << '#' << " [File created by QConfigDB object "
		 << version() << "]" << endl;
	  if(!top.save(stream)) // traverse tree
	    {
	      kdDebug(GUARD, KAB_KDEBUG_AREA) <<
			 "QConfigDB::save: error saving subsections." << endl;
	    }
	  storeFileAge();
	  file.close();
	  rc=true;
	} else {
	    kdDebug() << "QConfigDB::save: error opening file \""
		      << filename <<
		"\" for writing.\n";
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
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		     "QConfigDB::save: reset (forced) r/w mode." << endl;
	} else {
	  kdDebug(GUARD, KAB_KDEBUG_AREA) <<
		     "QConfigDB::save: cannot reset (forced) r/w mode." << endl;
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
  register bool GUARD; GUARD=false ;
  // ############################################################################
  QFile file(filename);
  // -----
  if(file.open(IO_ReadOnly))
    {
      kdDebug(GUARD, KAB_KDEBUG_AREA) <<  "QConfigDB::load: file access OK." << endl;
      QTextStream stream(&file);
      stream.setEncoding(QTextStream::Latin1); // no conversion
      // -----
      clear();
      bool rc=top.readSection(stream, false);
      storeFileAge();
      file.close();
      emit(changed(this));
      kdDebug(GUARD, KAB_KDEBUG_AREA) << "QConfigDB::load: done." << endl;
      return rc;
    } else {
      kdDebug() << "QConfigDB::load: error opening file \"" << filename << "\" for reading." << endl;
      return false;
    }
  // ############################################################################
}
