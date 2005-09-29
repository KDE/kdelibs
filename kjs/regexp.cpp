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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "regexp.h"

#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace KJS;

RegExp::RegExp(const UString &p, int f)
  : pat(p), flgs(f), m_notEmpty(false), valid(true)
{
  nrSubPatterns = 0; // determined in match() with POSIX regex.

  // JS regexps can contain Unicode escape sequences (\uxxxx) which
  // are rather uncommon elsewhere. As our regexp libs don't understand
  // them we do the unescaping ourselves internally.
  UString intern;
  if (p.find('\\') >= 0) {
    bool escape = false;
    for (int i = 0; i < p.size(); ++i) {
      UChar c = p[i];
      if (escape) {
        escape = false;
        // we only care about \uxxxx
        if (c == 'u' && i + 4 < p.size()) {
          int c0 = p[i+1].unicode();
          int c1 = p[i+2].unicode();
          int c2 = p[i+3].unicode();
          int c3 = p[i+4].unicode();
          if (Lexer::isHexDigit(c0) && Lexer::isHexDigit(c1) &&
              Lexer::isHexDigit(c2) && Lexer::isHexDigit(c3)) {
            c = Lexer::convertUnicode(c0, c1, c2, c3);
            intern += UString(&c, 1);
            i += 4;
            continue;
          }
        }
        intern += UString('\\');
        intern += UString(&c, 1);
      } else {
        if (c == '\\')
          escape = true;
        else
          intern += UString(&c, 1);
      }
    }
  } else {
    intern = p;
  }

#ifdef HAVE_PCREPOSIX
  int pcreflags = 0;
  const char *perrormsg;
  int errorOffset;

  if (flgs & IgnoreCase)
    pcreflags |= PCRE_CASELESS;

  if (flgs & Multiline)
    pcreflags |= PCRE_MULTILINE;

  pcregex = pcre_compile(intern.ascii(), pcreflags,
			 &perrormsg, &errorOffset, NULL);
  if (!pcregex) {
#ifndef NDEBUG
    fprintf(stderr, "KJS: pcre_compile() failed with '%s'\n", perrormsg);
#endif
    valid = false;
    return;
  }

#ifdef PCRE_INFO_CAPTURECOUNT
  // Get number of subpatterns that will be returned
  int rc = pcre_fullinfo( pcregex, NULL, PCRE_INFO_CAPTURECOUNT, &nrSubPatterns);
  if (rc != 0)
#endif
    nrSubPatterns = 0; // fallback. We always need the first pair of offsets.

#else /* HAVE_PCREPOSIX */

  int regflags = 0;
#ifdef REG_EXTENDED
  regflags |= REG_EXTENDED;
#endif
#ifdef REG_ICASE
  if ( f & IgnoreCase )
    regflags |= REG_ICASE;
#endif

  //NOTE: Multiline is not feasible with POSIX regex.
  //if ( f & Multiline )
  //    ;
  // Note: the Global flag is already handled by RegExpProtoFunc::execute

  int errorCode = regcomp(&preg, intern.ascii(), regflags);
  if (errorCode != 0) {
#ifndef NDEBUG
    char errorMessage[80];
    regerror(errorCode, &preg, errorMessage, sizeof errorMessage);
    fprintf(stderr, "KJS: regcomp failed with '%s'", errorMessage);
#endif
    valid = false;
  }
#endif
}

RegExp::~RegExp()
{
#ifdef HAVE_PCREPOSIX
  if (pcregex)
    pcre_free(pcregex);
#else
  /* TODO: is this really okay after an error ? */
  regfree(&preg);
#endif
}

UString RegExp::match(const UString &s, int i, int *pos, int **ovector)
{
  if (i < 0)
    i = 0;
  if (ovector)
    *ovector = 0L;
  int dummyPos;
  if (!pos)
    pos = &dummyPos;
  *pos = -1;
  if (i > s.size() || s.isNull())
    return UString::null;

#ifdef HAVE_PCREPOSIX
  CString buffer(s.cstring());
  int bufferSize = buffer.size();
  int ovecsize = (nrSubPatterns+1)*3; // see pcre docu
  if (ovector) *ovector = new int[ovecsize];
  if (!pcregex)
    return UString::null;

  if (pcre_exec(pcregex, NULL, buffer.c_str(), bufferSize, i,
                m_notEmpty ? (PCRE_NOTEMPTY | PCRE_ANCHORED) : 0, // see man pcretest
                ovector ? *ovector : 0L, ovecsize) == PCRE_ERROR_NOMATCH)
  {
    // Failed to match.
    if ((flgs & Global) && m_notEmpty && ovector)
    {
      // We set m_notEmpty ourselves, to look for a non-empty match
      // (see man pcretest or pcretest.c for details).
      // So we don't stop here, we want to try again at i+1.
#ifdef KJS_VERBOSE
      fprintf(stderr, "No match after m_notEmpty. +1 and keep going.\n");
#endif
      m_notEmpty = 0;
      if (pcre_exec(pcregex, NULL, buffer.c_str(), bufferSize, i+1, 0,
                    ovector ? *ovector : 0L, ovecsize) == PCRE_ERROR_NOMATCH)
        return UString::null;
    }
    else // done
      return UString::null;
  }

  // Got a match, proceed with it.

  if (!ovector)
    return UString::null; // don't rely on the return value if you pass ovector==0
#else
  const uint maxMatch = 10;
  regmatch_t rmatch[maxMatch];

  char *str = strdup(s.ascii()); // TODO: why ???
  if (regexec(&preg, str + i, maxMatch, rmatch, 0)) {
    free(str);
    return UString::null;
  }
  free(str);

  if (!ovector) {
    *pos = rmatch[0].rm_so + i;
    return s.substr(rmatch[0].rm_so + i, rmatch[0].rm_eo - rmatch[0].rm_so);
  }

  // map rmatch array to ovector used in PCRE case
  nrSubPatterns = 0;
  for(uint j = 1; j < maxMatch && rmatch[j].rm_so >= 0 ; j++)
      nrSubPatterns++;
  int ovecsize = (nrSubPatterns+1)*3; // see above
  *ovector = new int[ovecsize];
  for (uint j = 0; j < nrSubPatterns + 1; j++) {
    if (j>maxMatch)
      break;
    (*ovector)[2*j] = rmatch[j].rm_so + i;
    (*ovector)[2*j+1] = rmatch[j].rm_eo + i;
  }
#endif

  *pos = (*ovector)[0];
#ifdef HAVE_PCREPOSIX  // TODO check this stuff in non-pcre mode
  if ( *pos == (*ovector)[1] && (flgs & Global) )
  {
    // empty match, next try will be with m_notEmpty=true
    m_notEmpty=true;
  }
#endif
  return s.substr((*ovector)[0], (*ovector)[1] - (*ovector)[0]);
}

#if 0 // unused
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

  char *str = strdup(s.ascii());
  int r = regexec(&preg, str, 0, 0, 0);
  free(str);

  return r == 0;
#endif
}
#endif
