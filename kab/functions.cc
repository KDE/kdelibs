/* -*- C++ -*-
 * This file implements the global functions.
 * 
 * the Configuration Database library
 * copyright:  (C) Mirko Sucker, 1998
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Sucker <mirko.sucker@unibw-hamburg.de>
 * requires:   C++-compiler, STL, string class,
 *             Nana for debugging
 * $Revision$
 */

#pragma implementation

#include "functions.h"
#include "debug.h"
#include <stdlib.h>

string ReadLineFromStream(ifstream& stream)
{
  // ########################################################
  ID(bool GUARD=false);
  REQUIRE(stream.good());
  string line;
  // -----
  LG(GUARD, "ReadLineFromStream:: reading line.\n");
  while(!stream.eof())
    {
      getline(stream, line);
      if(!line.empty())
	{
	  if(isComment(line))
	    {
	      LG(GUARD, "ReadLineFromStream: line \"%s\" is"
		 " a comment, skipping.\n",
		 line.c_str());
	      line.ERASE(); // empty line
	      continue;
	    }
	}
      break;
    }
  LG(GUARD, "ReadLineFromStream:: line \"%s\" read.\n",
     line.c_str());
  return line;
  // ########################################################
}

bool isComment(string line)
{
  // ########################################################
  string::size_type first, second;
  // -----
  if(!line.empty())
    {
      first=line.find('#');
      second=line.find_first_not_of("# \t");
      if(first!=string::npos && first<second)
	{
	  return true;
	}
      return false;
    } else {
      // line is empty but not a comment
      return false;
    }
  // ########################################################
}
  
bool getHomeDirectory(string& ref)
{
  ID(bool GUARD=false);
  // ########################################################  
  char* temp=getenv("HOME");
  // -----
  if(temp!=0)
    {
      ref=temp;
      ref+="/";
      LG(GUARD, "getHomeDirectory: home is \"%s\".\n",
	 ref.c_str());
      return true;
    } else {
      LG(GUARD, "getHomeDirectory: cannot find home of user"
	 " by searching environment var \"HOME\".\n");
      return false;
    }
  // ########################################################
}

bool htmlizeString(const string& orig, string& target)
{
  ID(bool GUARD=true);
  LG(GUARD, "htmlizeString: called.\n");
  // ########################################################
  string temp;
  char current;
  unsigned int index;
  // -----
  for(index=0; index<orig.size(); index++)
    { // ----- the following code is character set depending 
      //       and will possibly need recompilation for EVERY
      //       system using another charset than the machine
      //       it was compiled on!
      current=orig[index];
      switch(current)
	{
	case 'ä':
	default:
	  temp+=current;
	}
    }
  target=temp;
  LG(GUARD, "htmlizeString: done.\n");
  return false; // not implemented
  // ########################################################
}
  
