#ifndef MS_CONFIGDATABASE_FUNCTIONS_H_INCLUDED
#define MS_CONFIGDATABASE_FUNCTIONS_INCLUDED

/* This file declares global functions.
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

#pragma interface

#include "stl_headers.h"
#include <fstream.h>

/** read a line, ignore comments
  */
string ReadLineFromStream(ifstream&);

/** find out wether a line is a comment or not 
  * (comments start with #)
  */
bool isComment(string);

/// get the users home directory by checking for ENV variable "HOME"
bool getHomeDirectory(string&);

/** Parse the string and replace all non-standard characters with its
  * HTML equivalents.
  */
bool htmlizeString(const string& orig, string& target);

#endif // MS_CONFIGDATABASE_FUNCTIONS_H_INCLUDED
