/* -*- C++ -*-
 * This file declares global functions.
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

#ifndef MS_FUNCTIONS_H_INCLUDED
#define MS_FUNCTIONS_INCLUDED

#pragma interface

#include "stl_headers.h"
#include <fstream.h>
#include "debug.h"

/** Read a line, ignore comments. */
string ReadLineFromStream(ifstream&);

/** Find out wether a line is a comment or not (comments start with #). */
bool isComment(string);

/** Get the users home directory by checking for ENV variable "HOME". */
bool getHomeDirectory(string&);

/** Parse the string and replace all non-standard characters with its
  * HTML equivalents. */
bool htmlizeString(const string& orig, string& target);

/** #tokenize partens the given string "text" into its tokens at every occurence
 *  of the designator "des" and delivers the result in the list reference. 
 *  If "strict" is true, the last token is only added if there are characters 
 *  the last designator end the string end. */
void tokenize(list<string>& res, const string& text, char tr, bool strikt=false);

#endif // MS_FUNCTIONS_H_INCLUDED
