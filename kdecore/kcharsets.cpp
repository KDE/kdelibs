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

#include "kcharsetsdata.h"
#include "kcharsets.h"
#include "qstrlist.h"
#include "qfontinf.h"
#include "qregexp.h"
#include <stdio.h>

KCharsetsData *KCharsets::data=0;
KCharsetsData *KCharsetConverterData::kcharsetsData=0;

/////////////////////////////////////////////////////////////////

KCharsetConverter::KCharsetConverter(const char * inputCharset
                                    ,bool iamps
				    ,bool oamps){
				    
  data=new KCharsetConverterData(inputCharset,iamps,oamps);
}

KCharsetConverter::KCharsetConverter(const char * inputCharset
				    ,bool iamps
				    ,const char *outputCharset
				    ,bool oamps){
  data=new KCharsetConverterData(inputCharset,iamps,outputCharset,oamps);
}

KCharsetConverter::KCharsetConverter(const char * inputCharset
                                    ,const char * outputCharset
				    ,bool oamps){

  data=new KCharsetConverterData(inputCharset,FALSE,outputCharset,oamps);
}

KCharsetConverter::~KCharsetConverter(){
  delete data;
}

const KCharsetConversionResult & KCharsetConverter::convert(const char *str){

  data->convert(str,result);
  return result;
}

bool  KCharsetConverter::ok(){

  return data->ok();
}
   
/////////////////////////////////////////////////////////////////////
KCharsets::KCharsets(){

  if (!data){
     data=new KCharsetsData();
     KCharsetConverterData::kcharsetsData=data;
  }   
  data->setDefaultCharset("iso-8859-1");
}

KCharsets::~KCharsets(){

}

QString KCharsets::defaultCharset()const{

  return data->defaultCharset();
}

bool KCharsets::setDefault(const char *ch){

  if ( data->charsetEntry(ch) ){
     data->setDefaultCharset(ch);
     return TRUE; 
  }   
  warning("Wrong charset! Setting to default (iso-8859-1)");
  data->setDefaultCharset("iso-8859-1");
  return FALSE;  
}

QStrList KCharsets::available()const{

  QStrList w;
  int i;
  for(i=0;data->charsetEntry(i);i++)
    w.append(data->charsetEntry(i)->name);
  return w;  
}

QStrList KCharsets::displayable(const char *face){

  QStrList w;
  int i;
  for(i=0;data->charsetEntry(i);i++)
    if (isDisplayable(data->charsetEntry(i)->name,face))
          w.append(data->charsetEntry(i)->name);
  return w;  
}

QStrList KCharsets::displayable(){

  QStrList w;
  int i;
  for(i=0;data->charsetEntry(i);i++){
    const char *charset=data->charsetEntry(i)->name;
    if (isDisplayable(charset,"courier"))
          w.append(charset);
    else if ( data->faceForCharset(charset)!=0 ) 
          w.append(charset);
  }
	
  return w;  
}

QStrList KCharsets::registered()const{

  QStrList w;
  int i;
  for(i=0;data->charsetEntry(i);i++)
    if (data->charsetEntry(i)->registered)
       w.append(data->charsetEntry(i)->name);
  return w;  
}

bool KCharsets::isAvailable(const char* charset){

  if (data->charsetEntry(charset)) return TRUE;
  else return FALSE;
}

bool KCharsets::isDisplayable(const char* charset,const char *face){

  kchdebug("Testing if %s is displayable\n",charset);
  if ( stricmp(charset,"any")==0 ) return TRUE;

  KCharsetEntry *ce=data->charsetEntry(charset);
  if (!ce){
    kchdebug("there is no %s charset!\n",charset);
    return FALSE;
  }  

  if (data->charsetOfFace(charset,face)) return TRUE;
  
  QFont::CharSet qcharset=ce->qtCharset;
  kchdebug("qtcharset=%i\n",qcharset);
  
  if ( qcharset==QFont::AnyCharSet ) return FALSE;
  else{
    QFont f(face);
    f.setCharSet(qcharset);
    QFontInfo fi(f);
    kchdebug("fi.charset()=%i\n",fi.charSet());
    if (fi.charSet()!=qcharset) return FALSE;
    else return TRUE;
  }  
}

bool KCharsets::isRegistered(const char* charset){

  KCharsetEntry *ce=data->charsetEntry(charset);
  if (!ce) return FALSE;
  if (ce->registered) return TRUE;
  else return FALSE;
}

QFont::CharSet KCharsets::qtCharset(const char *charset){

  if (!stricmp(charset,"any")) return QFont::AnyCharSet;
  KCharsetEntry *ce=data->charsetEntry(charset);
  if (ce) return ce->qtCharset;
  return QFont::AnyCharSet;
}

int KCharsets::bits(const char * charset){

  if ( stricmp(charset,"unicode") == 0 ) return 16;
  else if ( stricmp(charset,"iso-10640") == 0 ) return 16;
  else if ( stricmp(charset,"us-ascii") ==0 ) return 7;
  else if ( stricmp(charset,"UTF8") == 0 ) return 8;
  else if ( stricmp(charset,"UTF7") == 0 ) return 7;
  else return 8;
}

const char * KCharsets::name(QFont::CharSet qtcharset){

  if (qtcharset==QFont::AnyCharSet) return "unknown";
  for(int i=1;data->charsetEntry(i);i++)
    if (data->charsetEntry(i)->qtCharset==qtcharset)
       return data->charsetEntry(i)->name;
  return "unknown";
}

QFont::CharSet KCharsets::qtCharset(){

  return qtCharset(data->defaultCharset());
}

QFont &KCharsets::setQFont(QFont &fnt,const char *charset){

  if ( (stricmp(name(fnt),charset) == 0)
     || data->charsetOfFace(charset,fnt.family())) return fnt;
  kchdebug("Setting font to: \"%s\"\n",charset);
  QString faceStr=data->faceForCharset(charset);
  kchdebug("Face for font: \"%s\"\n",(const char *)faceStr);
  if (faceStr){
     faceStr.replace("\\*",fnt.family());
     kchdebug("New face for font: \"%s\"\n",(const char *)faceStr);
     fnt.setFamily(faceStr);
     fnt.setCharSet(QFont::AnyCharSet);
  }
  else{
    fnt.setCharSet(qtCharset(charset));
    QString family=fnt.family();
    if (family=="roman") fnt.setFamily("courier");	//  workaround for bug	
    else fnt.setFamily("roman");			//  in Qt
    fnt.setFamily(family);
  }  
  kchdebug("New charset: \"%s\"\n",name(fnt));
  return fnt;
}

QFont &KCharsets::setQFont(QFont &fnt){

  return setQFont(fnt,data->defaultCharset());
}

const char * KCharsets::name(const QFont& font){

  if (font.charSet()!=QFont::AnyCharSet) return name(font.charSet());
  KCharsetEntry * ce=data->charsetOfFace(font.family());
  kchdebug("ce=%p ce->name=%s\n",ce,ce?ce->name:0);
  if (ce) return ce->name;
  else return "unknown";
} 
