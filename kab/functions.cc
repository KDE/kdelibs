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

#ifndef KDE_USE_FINAL
#pragma implementation "functions.h"
#endif

#include "functions.h"
#include <qwidget.h>
#include "debug.h"
#include <stdlib.h>

string AuthorEmailAddress;

string ReadLineFromStream(ifstream& stream)
{
  register bool GUARD; GUARD=false;
  REQUIRE(stream.good());
  // ############################################################################
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
	      LG(GUARD, "ReadLineFromStream: line \"%s\" is a comment, "
		 "skipping.\n", line.c_str());
	      line.ERASE(); // empty line
	      continue;
	    }
	}
      break;
    }
  LG(GUARD, "ReadLineFromStream:: line \"%s\" read.\n", line.c_str());
  return line;
  // ############################################################################
}

bool isComment(string line)
{
  // ############################################################################
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
    } else { // line is empty but not a comment
      return false;
    }
  // ############################################################################
}

bool getHomeDirectory(string& ref)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  char* temp=getenv("HOME");
  // -----
  if(temp!=0)
    {
      ref=temp;
      ref+="/";
      LG(GUARD, "getHomeDirectory: home is \"%s\".\n", ref.c_str());
      return true;
    } else {
      LG(GUARD, "getHomeDirectory: cannot find home of user"
	 " by searching environment var \"HOME\".\n");
      return false;
    }
  // ############################################################################
}

bool htmlizeString(const string& orig, string& target)
{
  register bool GUARD; GUARD=true;
  LG(GUARD, "htmlizeString: called.\n");
  // ############################################################################
  string temp;
  char current;
  unsigned int index;
  // -----
  for(index=0; index<orig.size(); index++)
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
  LG(GUARD, "htmlizeString: done.\n");
  return false; // not implemented
  // ############################################################################
}

void tokenize(list<string>& resultat, const string& text, char tr, bool strikt)
{
  register bool GUARD; GUARD=false;
  // ############################################################################
  LG(GUARD, "tokenize: called.\n");
  string::size_type eins=0, zwei=0;
  string teil;
  // -----
  LG(GUARD, "tokenize: partening -->%s<--.\n", text.c_str());
  resultat.erase(resultat.begin(), resultat.end());
  // -----
  if(text.empty())
    {
      LG(GUARD, "tokenize: empty string, done.\n");
      return;
    }
  while(zwei!=string::npos)
    {
      teil="";
      zwei=text.find(tr, eins);
      if(zwei!=string::npos)
	{
	  teil.assign(text, eins, zwei-eins);
	  CHECK(teil.size()==zwei-eins);
	  resultat.push_back(teil);
	} else { // last element
	  if(!strikt && zwei>eins+1) // nur wenn dazwischen Zeichen sind
	    {
	      teil.assign(text, eins);
	      CHECK(teil.size()==text.size()-eins);
	      resultat.push_back(teil);
	    }
	}
      eins=zwei+1;
      if(eins>=text.size()) break;
    }
  LG(GUARD, "tokenize: partened in %i parts.\n", resultat.size());
  // -----
  LG(GUARD, "tokenize: done.\n");
  // ############################################################################
}

