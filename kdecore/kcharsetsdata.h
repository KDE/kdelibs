/* This file is part of the KDE libraries
    Copyright (C) 1997 Jacek Konieczny (jajcus@zeus.polsl.gliwice.pl)
    $Id$

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

#ifndef _CHARSETSDATA_H
#define _CHARSETSDATA_H

// #define KCH_DEBUG

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qfont.h>

#include <qdict.h>
#include <qintdict.h>

struct KCharsetEntry{
  char *name;
  QFont::CharSet qtCharset;
  const unsigned *toUnicode;
  bool registered;
  QIntDict<unsigned> * toUnicodeDict;
};

class KCharsetConversionResult;
class KCharsetsData;


class KCharsetConverterData{
friend class KCharsets;
   static KCharsetsData *kcharsetsData;
   bool inAmps,outAmps;
   bool unicodeIn,unicodeOut;
   enum{
   	NoConversion,
 	ToUnicode,
     	FromUnicode,
     	UnicodeUnicode,
	EightToEight,
	Optimal
   }conversionType;
   enum Encoding{
        none,
        UTF7,
	UTF8
   }inputEnc,outputEnc;
   int inBits;
   int outBits;
   
   QIntDict<unsigned> *convToUniDict;
   QIntDict<unsigned> *convFromUniDict;
   const unsigned *convTable;
   
   KCharsetEntry * input;
   KCharsetEntry * output;
   bool isOK;

   void setInputSettings();
   void setOutputSettings();
   bool getToUnicodeTable();
   bool createFromUnicodeDict();
   bool decodeUTF7(const char*,unsigned int &code,int &extrachars);
   bool decodeUTF8(const char*,unsigned int &code,int &extrachars);
   bool encodeUTF7(unsigned int code,QString &result);
   bool encodeUTF8(unsigned int code,QString &result);
   bool createFromUnicodeTable();
public:
   KCharsetConverterData(const char * inputCharset,bool iamps,
                         const char * outputCharset,bool oamps);
   KCharsetConverterData(const char * inputCharset,bool iamps,bool oamps);
   ~KCharsetConverterData();
   void convert(const QString &str,KCharsetConversionResult &r);
   bool ok()const{ return isOK; }
};

struct KCharTags{
  const char *tag;
  unsigned code;
};

#define CHAR_TAGS_COUNT 247

class KSimpleConfig;
    
class KCharsetsData{
  static KCharsetEntry charsets[];
  static const KCharTags tags[];
  QDict<KCharsetEntry> aliases;
  QDict<KCharsetEntry> i18nCharsets;
  const char * defaultCh;
  KSimpleConfig *config;
  void scanDirectory(const char *path);
  void createDictFromi18n(KCharsetEntry *e);
public:
  KCharsetsData();
  ~KCharsetsData();
  const char *faceForCharset(const char *charset);
  QString charsetFace(const char *charset,const QString &face);
  bool charsetOfFace(const char *charset,const QString &face);
  KCharsetEntry * charsetOfFace(const QString &face);
  KCharsetEntry * charsetEntry(const char *name);
  KCharsetEntry * charsetEntry(int index);
  const char * defaultCharset()
                 { return defaultCh; }
  bool setDefaultCharset(const char *name);
  const unsigned *getToUnicodeTable(const char *charset);
  QIntDict<unsigned> *getToUnicodeDict(const char *charset);
};

#ifdef KCH_DEBUG
#include <stdio.h>
#include <stdarg.h>
inline void kchdebug(const char *msg,...){
    va_list ap;
    va_start( ap, msg );                // use variable arg list
    vfprintf( stdout, msg, ap );
    va_end( ap );                    
}    
#else /* KCH_DEBUG */
inline void kchdebug(const char *,...){}
#endif /* KCH_DEBUG */

#endif
