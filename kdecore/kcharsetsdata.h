/* This file is part of the KDE libraries
    Copyright (C) 1997 Jacek Konieczny (jajcus@zeus.polsl.gliwice.pl)

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

// $Id$
// $Log$
// Revision 1.1  1997/12/08 17:13:24  jacek
// *** empty log message ***
//
// Revision 1.5  1997/12/08 15:25:40  jacek
// *** empty log message ***
//
// Revision 1.4  1997/12/08 11:22:24  jacek
// *** empty log message ***
//
// Revision 1.3  1997/12/06 10:25:54  jacek
// Copyright information added
//
// Revision 1.5  1997/12/06 10:22:05  jacek
// kcharsets.h docified
//
#ifndef _CHARSETSDATA_H
#define _CHARSETSDATA_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qfont.h>

#include <qdict.h>
#include <qintdict.h>

struct KCharsetEntry{
  const char *name;
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

#endif
