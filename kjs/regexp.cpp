// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003,2004 Apple Computer, Inc.
 *  Copyright (C) 2006      Maksim Orlovich (maksim@kde.org)
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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace KJS;

RegExp::UTF8SupportState RegExp::utf8Support = RegExp::Unknown;

RegExp::RegExp(const UString &p, int f)
  : pat(p), flgs(f), m_notEmpty(false), valid(true), buffer(0), originalPos(0)
{
  // Determine whether libpcre has unicode support if need be..
#ifdef PCRE_CONFIG_UTF8
  if (utf8Support == Unknown) {
    int supported;
    pcre_config(PCRE_CONFIG_UTF8, (void*)&supported);
    utf8Support = supported ? Supported : Unsupported;
  }
#endif

  nrSubPatterns = 0; // determined in match() with POSIX regex.

  // JS regexps can contain Unicode escape sequences (\uxxxx) which
  // are rather uncommon elsewhere. As our regexp libs don't understand
  // them we do the unescaping ourselves internally.
  // Also make sure to expand out any nulls as pcre_compile 
  // expects null termination..
  UString intern;
  const char* const nil = "\\x00";
  if (p.find('\\') >= 0 || p.find(KJS::UChar('\0')) >= 0) {
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
            if (c.unicode() == 0) {
                // Make sure to encode 0, to avoid terminating the string
                intern += UString(nil);
            } else {
                intern += UString(&c, 1);
            }
            i += 4;
            continue;
          }
        }
        intern += UString('\\');
        intern += UString(&c, 1);
      } else {
        if (c == '\\')
          escape = true;
        else if (c == '\0')
          intern += UString(nil);
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

  if (utf8Support == Supported)
    pcreflags |= PCRE_UTF8;

  // Fill our buffer with an encoded version, whether utf-8, or, 
  // if PCRE is incapable, truncated.
  prepareMatch(intern);

  pcregex = pcre_compile(buffer, pcreflags,
			 &perrormsg, &errorOffset, NULL);
  doneMatch(); // Cleanup buffers
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
  doneMatch(); // Be 100% sure buffers are freed
#ifdef HAVE_PCREPOSIX
  if (pcregex)
    pcre_free(pcregex);
#else
  /* TODO: is this really okay after an error ? */
  regfree(&preg);
#endif
}

void RegExp::prepareUtf8(const UString& s)
{
  // Allocate a buffer big enough to hold all the characters plus \0
  const int length = s.size();
  buffer = new char[length * 3 + 1];

  // Also create buffer for positions. We need one extra character in there,
  // even past the \0 since the non-empty handling may jump one past the end
  originalPos = new int[length * 3 + 2];

  // Convert to runs of 8-bit characters, and generate indeces
  // Note that we do NOT combine surrogate pairs here, as 
  // regexps operate on them as separate characters
  char *p      = buffer;
  int  *posOut = originalPos;
  const UChar *d = s.data();
  for (int i = 0; i != length; ++i) {
    unsigned short c = d[i].unicode();

    int sequenceLen;
    if (c < 0x80) {
      *p++ = (char)c;
      sequenceLen = 1;
    } else if (c < 0x800) {
      *p++ = (char)((c >> 6) | 0xC0); // C0 is the 2-byte flag for UTF-8
      *p++ = (char)((c | 0x80) & 0xBF); // next 6 bits, with high bit set
      sequenceLen = 2;
    } else {
      *p++ = (char)((c >> 12) | 0xE0); // E0 is the 3-byte flag for UTF-8
      *p++ = (char)(((c >> 6) | 0x80) & 0xBF); // next 6 bits, with high bit set
      *p++ = (char)((c | 0x80) & 0xBF); // next 6 bits, with high bit set
      sequenceLen = 3;
    }

    while (sequenceLen > 0) {
      *posOut = i;
      ++posOut;
      --sequenceLen;
    }
  }

  bufferSize = p - buffer;

  *p++ = '\0';

  // Record positions for \0, and the fictional character after that.
  *posOut     = length;
  *(posOut+1) = length+1;
}

void RegExp::prepareASCII (const UString& s)
{
  originalPos = 0;

  // Best-effort attempt to get something done
  // when we don't have utf 8 available -- use 
  // truncated version, and pray for the best 
  CString truncated = s.cstring();
  buffer = new char[truncated.size() + 1];
  memcpy(buffer, truncated.c_str(), truncated.size());
  buffer[truncated.size()] = '\0'; // For _compile use
  bufferSize = truncated.size();
}

void RegExp::prepareMatch(const UString &s)
{
  delete[] originalPos; // Just to be sure..
  delete[] buffer;
  if (utf8Support == Supported)
    prepareUtf8(s);
  else
    prepareASCII(s);

#ifndef NDEBUG
  originalS = s;
#endif
}

void RegExp::doneMatch() 
{
  delete[] originalPos; originalPos = 0;
  delete[] buffer;      buffer      = 0;
}

UString RegExp::match(const UString &s, int i, int *pos, int **ovector)
{
#ifndef NDEBUG
  assert(s.data() == originalS.data()); // Make sure prepareMatch got called right..
#endif
  assert(valid);

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
  int ovecsize = (nrSubPatterns+1)*3; // see pcre docu
  if (ovector) *ovector = new int[ovecsize];
  if (!pcregex)
    return UString::null;

  int startPos;
  int nextPos;

  if (utf8Support == Supported) {
    startPos = i;
    while (originalPos[startPos] < i)
      ++startPos;

    nextPos = startPos;
    while (originalPos[nextPos] < (i + 1))
      ++nextPos;
  } else {
    startPos = i;
    nextPos  = i + 1;
  }

  if (pcre_exec(pcregex, NULL, buffer, bufferSize, startPos,
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
      if (pcre_exec(pcregex, NULL, buffer, bufferSize, nextPos, 0,
                    ovector ? *ovector : 0L, ovecsize) == PCRE_ERROR_NOMATCH)
        return UString::null;
    }
    else // done
      return UString::null;
  }

  // Got a match, proceed with it.
  // But fix up the ovector if need be..
  if (ovector && originalPos) {
    for (unsigned c = 0; c < 2 * (nrSubPatterns + 1); ++c) {
      if ((*ovector)[c] != -1)
        (*ovector)[c] = originalPos[(*ovector)[c]];
    }
  }

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
  for (uint j = 0; j < maxMatch && rmatch[j].rm_so >= 0 ; j++) {
    nrSubPatterns++;
    // if the nonEmpty flag is set, return a failed match if any of the
    // subMatches happens to be an empty string.
    if (m_notEmpty && rmatch[j].rm_so == rmatch[j].rm_eo) 
      return UString::null;
  }
  // Allow an ovector slot to return the (failed) match result.
  if (nrSubPatterns == 0) nrSubPatterns = 1;
  
  int ovecsize = (nrSubPatterns)*3; // see above
  *ovector = new int[ovecsize];
  for (uint j = 0; j < nrSubPatterns; j++) {
      (*ovector)[2*j] = rmatch[j].rm_so + i;
      (*ovector)[2*j+1] = rmatch[j].rm_eo + i;
  }
#endif

  *pos = (*ovector)[0];
  if ( *pos == (*ovector)[1] && (flgs & Global) )
  {
    // empty match, next try will be with m_notEmpty=true
    m_notEmpty=true;
  }
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
