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
#include <qlist.h>
#include <qintdict.h>
#include <kcharsets.h>

struct KCharsetEntry{
  char *name;
  QFont::CharSet qtCharset;
  const unsigned *toUnicode;
  bool registered;
  QIntDict<unsigned> * toUnicodeDict;
};

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
   
   const KCharsetEntry * input;
   const KCharsetEntry * output;
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
   const char * convert(const char *str,KCharsetConversionResult &r
                       ,unsigned *pUnicode);
public:
   KCharsetConverterData(const char * inputCharset
                         ,const char * outputCharset
                         ,int flags);
   KCharsetConverterData(const char * inputCharset
                         ,int flags);
   ~KCharsetConverterData();
   bool initialize(const char * inputCharset,const char * outputCharset);
   void convert(const char *str,KCharsetConversionResult &r);
   void convert(const char *str,QList<KCharsetConversionResult> &r);
   const KCharsetConversionResult & convert(unsigned code);
   const KCharsetConversionResult & convertTag(const char *tag,int &len);
   const char * outputCharset()const;
   bool ok()const{ return isOK; }
};

struct KCharTags{
  const char *tag;
  unsigned code;
};

#define CHAR_TAGS_COUNT 247

class KSimpleConfig;

struct KDispCharEntry{
  KCharsetEntry *charset;
  unsigned code;
};
    
class KCharsetsData{
  static KCharsetEntry charsets[];
  QDict<KCharsetEntry> aliases;
  QDict<KCharsetEntry> i18nCharsets;
  QIntDict<KDispCharEntry> *displayableCharsDict;
  const KCharsetEntry * defaultCh;
  KSimpleConfig *config;
  void scanDirectory(const char *path);
  void createDictFromi18n(KCharsetEntry *e);
  KCharsetEntry * varCharsetEntry(const char *name);
public:
  static const KCharTags tags[];
  KCharsetsData();
  ~KCharsetsData();
  const char *faceForCharset(const KCharsetEntry *charset);
  QString charsetFace(const KCharsetEntry *charset,const QString &face);
  bool charsetOfFace(const KCharsetEntry *charset,const QString &face);
  const KCharsetEntry * charsetOfFace(const QString &face);
  const KCharsetEntry * charsetEntry(const char *name){
    return varCharsetEntry(name);
  }
  const KCharsetEntry * charsetEntry(int index);
  const KCharsetEntry * defaultCharset()const
                 { return defaultCh; }
  bool setDefaultCharset(const KCharsetEntry *charset);
  const unsigned *getToUnicodeTable(const  KCharsetEntry *charset);
  QIntDict<unsigned> *getToUnicodeDict(const KCharsetEntry *charset);
  const KCharsetEntry * conversionHint(const KCharsetEntry *charset);
  bool isDisplayable(const KCharsetEntry * charset);
  unsigned decodeAmp(const char *seq,int &len);
  void convert(unsigned code,KCharsetConversionResult & r);
  void convertTag(const char *tag,KCharsetConversionResult & r,int &l);
  const QIntDict<KDispCharEntry> * getDisplayableDict();
};

#ifdef KCH_DEBUG
#include <stdio.h>
#include <stdarg.h>
inline void kchdebug(const char *msg,...){
    va_list ap;
    va_start( ap, msg );                // use variable arg list
    vfprintf( stderr, msg, ap );
    va_end( ap );                    
}    
#else /* KCH_DEBUG */
inline void kchdebug(const char *,...){}
#endif /* KCH_DEBUG */

#endif
