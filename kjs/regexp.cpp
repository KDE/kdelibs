// -*- c-basic-offset: 2 -*-
/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2001,2004 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003,2004 Apple Computer, Inc.
 *  Copyright (C) 2006      Maksim Orlovich (maksim@kde.org)
 *  Copyright (C) 2007      Sune Vuorela (debian@pusling.com)
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
#include <config.h>
#include "lexer.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wtf/Vector.h>
using WTF::Vector;

// GCC cstring uses these automatically, but not all implementations do.
using std::strlen;
using std::strcpy;
using std::strncpy;
using std::memset;
using std::memcpy;

namespace KJS {

RegExp::UTF8SupportState RegExp::utf8Support = RegExp::Unknown;

// JS regexps can contain Unicode escape sequences (\uxxxx) which
// are rather uncommon elsewhere. As our regexp libs don't understand
// them we do the unescaping ourselves internally.
// Also make sure to expand out any nulls as pcre_compile
// expects null termination..
static UString sanitizePattern(const UString &p)
{
  UString newPattern;

  const char* const nil = "\\x00";
  if (p.find("\\u") >= 0 || p.find(KJS::UChar('\0')) >= 0) {
    bool escape = false;
    for (int i = 0; i < p.size(); ++i) {
      UChar c = p[i];
      if (escape) {
        escape = false;
        // we only care about \u
        if (c == 'u') {
	  // standard unicode escape sequence looks like \uxxxx but
	  // other browsers also accept less than 4 hex digits
	  unsigned short u = 0;
	  int j = 0;
	  for (j = 0; j < 4; ++j) {
	    if (i + 1 < p.size() && Lexer::isHexDigit(p[i + 1].unicode())) {
	      u = (u << 4) + Lexer::convertHex(p[i + 1].unicode());
	      ++i;
	    } else {
	      // sequence incomplete. restore index.
	      // TODO: cleaner way to propagate warning
	      fprintf(stderr, "KJS: saw %d digit \\u sequence.\n", j);
	      i -= j;
	      break;
	    }
	  }
	  if (j < 4) {
	    // sequence was incomplete. treat \u as u which IE always
	    // and FF sometimes does.
	     newPattern.append(UString('u'));
	  } else {
            c = UChar(u);
            switch (u) {
            case 0:
	      // Make sure to encode 0, to avoid terminating the string
	       newPattern += UString(nil);
	      break;
            case '^':
            case '$':
            case '\\':
            case '.':
            case '*':
            case '+':
            case '?':
            case '(': case ')':
            case '{': case '}':
            case '[': case ']':
            case '|':
	      // escape pattern characters have to remain escaped
	       newPattern.append(UString('\\'));
	      // intentional fallthrough
            default:
	      newPattern += UString(&c, 1);
	      break;
	    }
          }
          continue;
        }
        newPattern += UString('\\');
        newPattern += UString(&c, 1);
      } else {
        if (c == '\\')
          escape = true;
        else if (c == '\0')
          newPattern += UString(nil);
        else
          newPattern += UString(&c, 1);
      }
    }
    return newPattern;
  } else {
    return p;
  }
}

// For now, the only 'extension' to standard we are willing to deal with is
// a non-escaped closing bracket, outside of a character class. e.g. /.*]/
static bool sanitizePatternExtensions(UString &p)
{
  UString newPattern;

  static const int StateNominal = 0, StateOpenBracket = 1;
  WTF::Vector<int> v;
  bool escape = false;

  int state = StateNominal;
  for (int i = 0; i < p.size(); ++i) {
      UChar c = p[i];
      if (escape) {
        escape = false;
      } else {
        if (c == '\\') {
          escape = true;
        } else if (c == ']') {
            if (state == StateOpenBracket) {
                state = StateNominal;
            } else if (state == StateNominal) {
                v.append(i);
            }
        } else if (c == '[') {
            if (state == StateOpenBracket) {
                v.append(i);
            } else if (state == StateNominal) {
                state = StateOpenBracket;
            }
        }
    }
  }
  if (state == StateOpenBracket) {
      // this is not recoverable.
      return false;
  }
  if (v.size()) {
      int pos=0;
      Vector<int>::const_iterator end = v.end();
      for (Vector<int>::const_iterator it = v.begin(); it != end; ++it) {
          newPattern += p.substr(pos, *it-pos);
          pos = *it;
          newPattern += UString('\\');
      }
      newPattern += p.substr(pos);
      p = newPattern;
      return true;
  } else {
    return false;
  }
}

RegExp::RegExp(const UString &p, char flags)
  : _pat(p), _flags(flags), _valid(true), _numSubPatterns(0), _buffer(0), _originalPos(0)
{
#ifdef HAVE_PCREPOSIX
  // Determine whether libpcre has unicode support if need be..
  if (utf8Support == Unknown) {
    int supported;
    pcre_config(PCRE_CONFIG_UTF8, (void*)&supported);
    utf8Support = supported ? Supported : Unsupported;
  }
#endif

  UString intern = sanitizePattern(p);

#ifdef HAVE_PCREPOSIX
  int options = 0;

  // we are close but not 100% the same as Perl
#ifdef PCRE_JAVASCRIPT_COMPAT // introduced in PCRE 7.7
  options |= PCRE_JAVASCRIPT_COMPAT;
#endif

  // Note: the Global flag is already handled by RegExpProtoFunc::execute.
  // FIXME: That last comment is dubious. Not all RegExps get run through RegExpProtoFunc::execute.
  if (flags & IgnoreCase)
    options |= PCRE_CASELESS;
  if (flags & Multiline)
    options |= PCRE_MULTILINE;

  if (utf8Support == Supported)
    options |= (PCRE_UTF8 | PCRE_NO_UTF8_CHECK);

  const char *errorMessage;
  int errorOffset;
  bool secondTry = false;

  while (1) {
    // Fill our buffer with an encoded version, whether utf-8, or,
    // if PCRE is incapable, truncated.
    prepareMatch(intern);
    _regex = pcre_compile(_buffer, options, &errorMessage, &errorOffset, NULL);
    doneMatch(); //Cleanup buffers
    if (!_regex) {
#ifdef PCRE_JAVASCRIPT_COMPAT
      // The compilation failed. It is likely the pattern contains non-standard extensions.
      // We may try to tolerate some of those extensions.
      bool doRecompile = !secondTry && sanitizePatternExtensions(intern);
      if (doRecompile) {
        secondTry = true;
#ifndef NDEBUG
        fprintf(stderr, "KJS: pcre_compile() failed with '%s' - non-standard extensions detected in pattern, trying second compile after correction.\n", errorMessage);
#endif
        continue;
      }
#endif
#ifndef NDEBUG
      fprintf(stderr, "KJS: pcre_compile() failed with '%s'\n", errorMessage);
#endif
      _valid = false;
      return;
    }
    break;
  }

#ifdef PCRE_INFO_CAPTURECOUNT
  // Get number of subpatterns that will be returned.
  pcre_fullinfo(_regex, NULL, PCRE_INFO_CAPTURECOUNT, &_numSubPatterns);
#endif

#else /* HAVE_PCREPOSIX */

  int regflags = 0;
#ifdef REG_EXTENDED
  regflags |= REG_EXTENDED;
#endif
#ifdef REG_ICASE
  if ( flags & IgnoreCase )
    regflags |= REG_ICASE;
#endif

  //NOTE: Multiline is not feasible with POSIX regex.
  //if ( f & Multiline )
  //    ;
  // Note: the Global flag is already handled by RegExpProtoFunc::execute

  int errorCode = regcomp(&_regex, intern.ascii(), regflags);
  if (errorCode != 0) {
#ifndef NDEBUG
    char errorMessage[80];
    regerror(errorCode, &_regex, errorMessage, sizeof errorMessage);
    fprintf(stderr, "KJS: regcomp failed with '%s'\n", errorMessage);
#endif
    _valid = false;
  }
#endif
}

RegExp::~RegExp()
{
  doneMatch(); // Be 100% sure buffers are freed
#ifdef HAVE_PCREPOSIX
  pcre_free(_regex);
#else
  /* TODO: is this really okay after an error ? */
  regfree(&_regex);
#endif
}

void RegExp::prepareUtf8(const UString& s)
{
  // Allocate a buffer big enough to hold all the characters plus \0
  const int length = s.size();
  _buffer = new char[length * 3 + 1];

  // Also create buffer for positions. We need one extra character in there,
  // even past the \0 since the non-empty handling may jump one past the end
  _originalPos = new int[length * 3 + 2];

  // Convert to runs of 8-bit characters, and generate indices
  // Note that we do NOT combine surrogate pairs here, as
  // regexps operate on them as separate characters
  char *p      = _buffer;
  int  *posOut = _originalPos;
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

  _bufferSize = p - _buffer;

  *p++ = '\0';

  // Record positions for \0, and the fictional character after that.
  *posOut     = length;
  *(posOut+1) = length+1;
}

void RegExp::prepareASCII (const UString& s)
{
  _originalPos = 0;

  // Best-effort attempt to get something done
  // when we don't have utf 8 available -- use
  // truncated version, and pray for the best
  CString truncated = s.cstring();
  _buffer = new char[truncated.size() + 1];
  memcpy(_buffer, truncated.c_str(), truncated.size());
  _buffer[truncated.size()] = '\0'; // For _compile use
  _bufferSize = truncated.size();
}

void RegExp::prepareMatch(const UString &s)
{
  delete[] _originalPos; // Just to be sure..
  delete[] _buffer;
  if (utf8Support == Supported)
    prepareUtf8(s);
  else
    prepareASCII(s);

#ifndef NDEBUG
  _originalS = s;
#endif
}

void RegExp::doneMatch()
{
  delete[] _originalPos; _originalPos = 0;
  delete[] _buffer;      _buffer      = 0;
}

UString RegExp::match(const UString &s, bool *error, int i, int *pos, int **ovector)
{
#ifndef NDEBUG
  assert(s.data() == _originalS.data()); // Make sure prepareMatch got called right..
#endif

  if (i < 0)
    i = 0;
  int dummyPos;
  if (!pos)
    pos = &dummyPos;
  *pos = -1;
  if (ovector)
    *ovector = 0;

  if (i > s.size() || s.isNull())
    return UString::null();

#ifdef HAVE_PCREPOSIX

  if (!_regex)
    return UString::null();

  // Set up the offset vector for the result.
  // First 2/3 used for result, the last third used by PCRE.
  int *offsetVector;
  int offsetVectorSize;
  int fixedSizeOffsetVector[3];
  if (!ovector) {
    offsetVectorSize = 3;
    offsetVector = fixedSizeOffsetVector;
  } else {
    offsetVectorSize = (_numSubPatterns + 1) * 3;
    offsetVector = new int [offsetVectorSize];
  }

  int startPos;
  if (utf8Support == Supported) {
    startPos = i;
    while (_originalPos[startPos] < i)
      ++startPos;
  } else {
    startPos = i;
  }

  int baseFlags = utf8Support == Supported ? PCRE_NO_UTF8_CHECK : 0;
  
  // See if we have to limit stack space...
  *error = false;  
  int stackGlutton = 0;
  pcre_config(PCRE_CONFIG_STACKRECURSE, (void*)&stackGlutton);
  pcre_extra limits;
  if (stackGlutton) {
    limits.flags = PCRE_EXTRA_MATCH_LIMIT_RECURSION;
    // libPCRE docs claim that it munches about 500 bytes per recursion.
    // The crash in #160792 actually showed pcre 7.4 using about 1300 bytes
    // (and I've measured 800 in an another instance)
    // So the usual 8MiB rlimit on Linux produces about 6452 frames.
    // We go somewhat conservative, and use about 2/3rds of that, for 4300
    // especially since we're not exactly light on the stack, either
    // ### TODO: get some build system help to use getrlimit.
    limits.match_limit_recursion = 4300;
  }
  
  const int numMatches = pcre_exec(_regex, stackGlutton ? &limits : 0, _buffer, _bufferSize, startPos, baseFlags, offsetVector, offsetVectorSize);

  //Now go through and patch up the offsetVector
  if (utf8Support == Supported)
    for (int c = 0; c < 2 * numMatches; ++c)
      if (offsetVector[c] != -1)
        offsetVector[c] = _originalPos[offsetVector[c]];

  if (numMatches < 0) {
#ifndef NDEBUG
    if (numMatches != PCRE_ERROR_NOMATCH)
      fprintf(stderr, "KJS: pcre_exec() failed with result %d\n", numMatches);
#endif
    if (offsetVector != fixedSizeOffsetVector)
      delete [] offsetVector;
    if (numMatches == PCRE_ERROR_MATCHLIMIT || numMatches == PCRE_ERROR_RECURSIONLIMIT)
      *error = true;
    return UString::null();
  }

  *pos = offsetVector[0];
  if (ovector)
    *ovector = offsetVector;
  return s.substr(offsetVector[0], offsetVector[1] - offsetVector[0]);

#else

  if (!_valid)
    return UString::null();

  const unsigned maxMatch = 10;
  regmatch_t rmatch[maxMatch];

  char *str = strdup(s.ascii()); // TODO: why ???
  if (regexec(&_regex, str + i, maxMatch, rmatch, 0)) {
    free(str);
    return UString::null();
  }
  free(str);

  if (!ovector) {
    *pos = rmatch[0].rm_so + i;
    return s.substr(rmatch[0].rm_so + i, rmatch[0].rm_eo - rmatch[0].rm_so);
  }

  // map rmatch array to ovector used in PCRE case
  _numSubPatterns = 0;
  for(unsigned j = 1; j < maxMatch && rmatch[j].rm_so >= 0 ; j++)
      _numSubPatterns++;
  int ovecsize = (_numSubPatterns+1)*3; // see above
  *ovector = new int[ovecsize];
  for (unsigned j = 0; j < _numSubPatterns + 1; j++) {
    if (j>maxMatch)
      break;
    (*ovector)[2*j] = rmatch[j].rm_so + i;
    (*ovector)[2*j+1] = rmatch[j].rm_eo + i;
  }

  *pos = (*ovector)[0];
  return s.substr((*ovector)[0], (*ovector)[1] - (*ovector)[0]);

#endif
}

} // namespace KJS
