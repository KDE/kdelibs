// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include <stdio.h>

#include "regexp.h"

using namespace KJS;

RegExp::RegExp(const UString &p, int f)
  : pattern(p), flags(f)
{

#ifdef HAVE_PCREPOSIX
  int pcreflags = 0;
  const char *perrormsg;
  int errorOffset;

  if (flags & IgnoreCase)
    pcreflags |= PCRE_CASELESS;

  if (flags & Multiline)
    pcreflags |= PCRE_MULTILINE;

  pcregex = pcre_compile(p.ascii(), pcreflags,
			 &perrormsg, &errorOffset, NULL);


#else /* HAVE_PCREPOSIX */

  int regflags = 0;
#ifdef REG_EXTENDED
  regflags |= REG_EXTENDED;
#endif
#ifdef REG_ICASE
  if ( f & IgnoreCase )
    regflags |= REG_ICASE;
#endif

  //NOTE: Multiline is not feasble with POSIX regex.
  //if ( f & Multiline )
  //    ;
  // Note: the Global flag is already handled by RegExpProtoFunc::execute

  regcomp(&preg, p.ascii(), regflags);
  /* TODO check for errors */
#endif

}

RegExp::~RegExp()
{
#ifdef HAVE_PCREPOSIX
  pcre_free(pcregex);

#else
  /* TODO: is this really okay after an error ? */
  regfree(&preg);
#endif
}

UString RegExp::match(const UString &s, int i, int *pos)
{

#ifdef HAVE_PCREPOSIX
  // A very large number, but should cover the cases we need.
  //
  int ovector[300];
  CString buffer(s.cstring());

  if (i < 0)
    i = 0;

  if (i > s.size() || s.isNull() ||
      pcre_exec(pcregex, NULL, buffer.c_str(), buffer.size() - i, i,
		0, ovector, 300) == PCRE_ERROR_NOMATCH) {

    if (pos)
      *pos = -1;
    return UString::null;
  }

  if (pos)
    *pos = ovector[0];
  return s.substr(ovector[0], ovector[1] - ovector[0]);

#else
  regmatch_t rmatch[10];

  if (i < 0)
    i = 0;

  if (i > s.size() || s.isNull() ||
      regexec(&preg, s.ascii() + i, 10, rmatch, 0)) {
    if (pos)
      *pos = -1;
    return UString::null;
  }

  if (pos)
    *pos = rmatch[0].rm_so + i;
  return s.substr(rmatch[0].rm_so + i, rmatch[0].rm_eo - rmatch[0].rm_so);
#endif
}

bool RegExp::test(const UString &s, int)
{
#ifdef HAVE_PCREPOSIX
  int ovector[300];
  CString buffer(s.cstring());

  if (s.isNull() ||
      pcre_exec(pcregex, NULL, buffer.c_str(), buffer.size(), 0,
		0, ovector, 300) == PCRE_ERROR_NOMATCH)
    return false;
  else
    return true;

#else

  int r = regexec(&preg, s.ascii(), 0, 0, 0);

  return r == 0;
#endif
}
