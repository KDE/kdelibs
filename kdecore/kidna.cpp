/*
    This file is part of the KDE libraries

    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kidna.h"

#include <kdebug.h>

#include "ltdl.h"
#include <stdlib.h>

#define IDNA_SUCCESS 0

static lt_dlhandle KIDNA_lib; // = 0
static bool KIDNA_lib_load_failed; // = false

typedef int (*KIDNA_utf8_to_ace_t)(const char *, char **, int);
typedef int (*KIDNA_utf8ace_to_utf8_t)(const char *, char **, int);

static KIDNA_utf8_to_ace_t KIDNA_utf8_to_ace; // = 0
static KIDNA_utf8ace_to_utf8_t KIDNA_utf8ace_to_utf8; // = 0

static void KIDNA_load_lib()
{
   KIDNA_lib_load_failed = true; // Unless proven otherwise
   KIDNA_lib = lt_dlopen("/usr/local/lib/libidn.la");
   if (!KIDNA_lib) 
   {
      KIDNA_lib = lt_dlopen("/usr/lib/libidn.la");
   }
   
   if (!KIDNA_lib) 
      return; // Error

   KIDNA_utf8_to_ace = (KIDNA_utf8_to_ace_t) lt_dlsym(KIDNA_lib, "idna_to_ascii_8z");
   if (!KIDNA_utf8_to_ace)
   {
      kdWarning() << "Symbol idna_utf8_to_ace not found." << endl;   
      return; // Error
   }
         
   KIDNA_utf8ace_to_utf8 = (KIDNA_utf8ace_to_utf8_t) lt_dlsym(KIDNA_lib, "idna_to_unicode_8z8z");
   if (!KIDNA_utf8ace_to_utf8)
   {
      kdWarning() << "Symbol idna_utf8ace_to_utf8 not found." << endl;   
      return; // Error
   }
   KIDNA_lib_load_failed = false; // Succes
}

QCString KIDNA::toAsciiCString(const QString &idna)
{
   int l = idna.length();
   const QChar *u = idna.unicode();
   bool needConversion = false;
   for(;l--;)
   {
      if ((*u++).unicode() > 127)
      {
          needConversion = true;
          break;
      }
   }
   if (!needConversion)
      return idna.lower().latin1();

   if (!KIDNA_lib && !KIDNA_lib_load_failed)
   {
      KIDNA_load_lib();
   }

   if (KIDNA_lib_load_failed)
   {
      return 0; // Can't convert
   }
   else 
   {
      // Also handle names that start with "." even though libidn
      // doesn't like those
      bool bStartsWithDot = (idna[0] == '.');
      char *pOutput;
      if ((*KIDNA_utf8_to_ace)(idna.utf8().data()+(bStartsWithDot ? 1: 0), &pOutput, 0) == IDNA_SUCCESS)
      {
         QCString result = pOutput;
         free(pOutput);
         if (bStartsWithDot)
            return "."+result;
         return result;
      }
      else
      {
         return 0; // Can't convert
      }
   }
}

QString KIDNA::toAscii(const QString &idna)
{
   int l = idna.length();
   const QChar *u = idna.unicode();
   bool needConversion = false;
   for(;l--;)
   {
      if ((*u++).unicode() > 127)
      {
          needConversion = true;
          break;
      }
   }
   if (!needConversion)
      return idna.lower();

   if (!KIDNA_lib && !KIDNA_lib_load_failed)
   {
      KIDNA_load_lib();
   }

   if (KIDNA_lib_load_failed)
   {
      return QString::null; // Can't convert
   }
   else 
   {
      // Also handle names that start with "." even though libidn
      // doesn't like those
      bool bStartsWithDot = (idna[0] == '.');
      char *pOutput;
      if ((*KIDNA_utf8_to_ace)(idna.utf8().data()+(bStartsWithDot ? 1: 0), &pOutput, 0) == IDNA_SUCCESS)
      {
         QString result(pOutput);
         free(pOutput);
         if (bStartsWithDot)
            return "."+result;
         return result;
      }
      else
      {
         return QString::null; // Can't convert
      }
   }
}

QString KIDNA::toUnicode(const QString &idna)
{
   if (!KIDNA_lib && !KIDNA_lib_load_failed)
   {
      KIDNA_load_lib();
   }

   if (KIDNA_lib_load_failed)
   {
      return idna.lower(); // Return as is
   }
   else 
   {
      char *pOutput;
      if ((*KIDNA_utf8ace_to_utf8)(idna.utf8(), &pOutput, 0) == IDNA_SUCCESS)
      {
         QString result = QString::fromUtf8(pOutput);
         free(pOutput);
         return result;
      }
      else
      {
         return idna.lower(); // Return as is.
      }
   }
}
