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

#define KCHARSETS_CPP 
#include "kcharsetsdata.h"
#include "qstrlist.h"
#include "qfontinf.h"
#include "qregexp.h"
#include <kapp.h>


KCharsetsData *KCharsets::data=0;
KCharsetsData *KCharset::data=0;
KCharsets *KCharset::charsets=0;
KCharsetsData *KCharsetConverterData::kcharsetsData=0;

/////////////////////////////////////////////////////////////////

KCharset::KCharset(const KCharsetEntry *e){
  
   if (!data || !charsets){
      fatal("KCharset constructor called when no KCharsets object created");
      return;
   }   
   entry=e;
}

KCharset::KCharset(const char *str){

   if (!data || !charsets){
      fatal("KCharset constructor called when no KCharsets object created");
      return;
   }   
   entry=data->charsetEntry(str);
}

KCharset::KCharset(QString s){

   if (!data || !charsets){
      fatal("KCharset constructor called when no KCharsets object created");
      return;
   }   
   entry=data->charsetEntry(s);
}

KCharset::KCharset(QFont::CharSet){

  if (!data || !charsets){
      fatal("KCharset constructor called when no KCharsets object created");
      return;
   }  
  entry=data->charsetEntry("us-ascii");
}

const char * KCharset::name(){
  return entry->name;
}

bool KCharset::isDisplayable(){

  if (!entry) {
    warning("KCharset: Wrong charset!\n");
    return FALSE;
  }  
  
  return data->isDisplayable(entry);
}

bool KCharset::isDisplayable(const char *face){

  if (!entry) {
    warning("KCharset: Wrong charset!\n");
    return FALSE;
  }  
  if (!face){
    warning("KCharset: NULL face parameter!\n");
    return FALSE;
  }  
  
  kchdebug("Testing if %s is displayable\n",name());
  if ( stricmp(name(),"any")==0 ) return TRUE;

  if (data->charsetOfFace(entry,face)) return TRUE;
  
  QFont::CharSet qcharset=entry->qtCharset;
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

bool KCharset::isRegistered(){

  if (!entry) {
    warning("KCharset: Wrong charset!\n");
    return FALSE;
  }  
  if (entry->registered) return TRUE;
  else return FALSE;
}

QFont::CharSet KCharset::qtCharset(){

  if (!entry) {
    warning("KCharset: Wrong charset!\n");
    return QFont::AnyCharSet;
  }  
  if (!stricmp(name(),"any")) return QFont::AnyCharSet;
  if (entry) return entry->qtCharset;
  return QFont::AnyCharSet;
}

int KCharset::bits(){

  if (!entry) {
    warning("KCharset: Wrong charset!\n");
    return 8;
  }  
  if ( stricmp(name(),"unicode") == 0 ) return 16;
  else if ( stricmp(name(),"iso-10640") == 0 ) return 16;
  else if ( stricmp(name(),"us-ascii") ==0 ) return 7;
  else if ( stricmp(name(),"UTF-8") == 0 ) return 8;
  else if ( stricmp(name(),"UTF-7") == 0 ) return 7;
  else return 8;
}

QFont &KCharset::setQFont(QFont &fnt){

  if (!entry) {
    warning("KCharset: Wrong charset!\n");
    return fnt;
  }  
  if ( (stricmp(charsets->name(fnt),name()) == 0)
     || data->charsetOfFace(entry,fnt.family())) return fnt;
  kchdebug("Setting font to: \"%s\"\n",name());
  QString faceStr=data->faceForCharset(entry);
  kchdebug("Face for font: \"%s\"\n",(const char *)faceStr);
  if (faceStr){
     faceStr.replace("\\*",fnt.family());
     kchdebug("New face for font: \"%s\"\n",(const char *)faceStr);
     fnt.setFamily(faceStr);
     fnt.setCharSet(QFont::AnyCharSet);
  }
  else{
    fnt.setCharSet(qtCharset());
    QString family=fnt.family();
    if (family=="roman") fnt.setFamily("courier");	//  workaround for bug	
    else fnt.setFamily("roman");			//  in Qt
    fnt.setFamily(family);
  }  
  kchdebug("New charset: \"%s\"\n",charsets->name(fnt));
  return fnt;
}

KCharset::operator const KCharsetEntry *(){
 
  return entry;
}

/////////////////////////////////////////////////////////////////
	
KCharsetConverter::KCharsetConverter(const char * inputCharset
				    ,int flags){
				    
  warning("KCharsetConverter::KCharsetConverter(const char *,int) called. Recompile the application");		     
  if (!inputCharset) {
    warning("KCharsetConverter: NULL charset on input!\n");
    inputCharset="us-ascii";    
  }  
  data=new KCharsetConverterData(KCharset(inputCharset),flags);
}

KCharsetConverter::KCharsetConverter(const char * inputCharset
				    ,const char *outputCharset
				    ,int flags){
				    
  warning("KCharsetConverter::KCharsetConverter(const char *,const char *,int) called. Recompile the application");		     
  if (!inputCharset) {
    warning("KCharsetConverter: NULL charset on input!\n");
    inputCharset="us-ascii";    
  }  
  if (!outputCharset) {
    warning("KCharsetConverter: NULL charset on output!\n");
    outputCharset="us-ascii";    
  }  
  data=new KCharsetConverterData(KCharset(inputCharset)
                                 ,KCharset(outputCharset),flags);
}

KCharsetConverter::KCharsetConverter(KCharset inputCharset
				    ,int flags){
				    
  if (!inputCharset.ok()) {
    warning("KCharsetConverter: NULL charset on input!\n");
    inputCharset="us-ascii";    
  }  
  data=new KCharsetConverterData(inputCharset,flags);
}

KCharsetConverter::KCharsetConverter(KCharset inputCharset
				    ,KCharset outputCharset
				    ,int flags){
  if (!inputCharset.ok()) {
    warning("KCharsetConverter: NULL charset on input!\n");
    inputCharset="us-ascii";    
  }  
  if (!outputCharset.ok()) {
    warning("KCharsetConverter: NULL charset on output!\n");
    outputCharset="us-ascii";    
  }  
  data=new KCharsetConverterData(inputCharset,outputCharset,flags);
}

KCharsetConverter::~KCharsetConverter(){
  delete data;
}

bool  KCharsetConverter::ok(){

  return data->ok();
}

const char * KCharsetConverter::outputCharset(){

  return data->outputCharset();
}
   
const KCharsetConversionResult & KCharsetConverter::convert(const char *str){

  data->convert(str,result);
  return result;
}

static QList<KCharsetConversionResult> resultList;

const QList<KCharsetConversionResult> & KCharsetConverter::multipleConvert(
                                                             const char *str){
  resultList.setAutoDelete(TRUE);
  resultList.clear();
  data->convert(str,resultList);
  return resultList;
}

const KCharsetConversionResult & KCharsetConverter::convert(unsigned ch){
  
  return data->convert(ch);
}

const KCharsetConversionResult & KCharsetConverter::convertTag(const char *tag){

  int tmp;
  return data->convertTag(tag,tmp);
}
   
const KCharsetConversionResult & KCharsetConverter::convertTag(const char *tag
							       ,int &l){

  return data->convertTag(tag,l);
}

char * KCharsetConversionResult::copy()const{

  char *ptr=new char [cText.length()+1];
  strcpy(ptr,cText);
  return ptr;
}
   
/////////////////////////////////////////////////////////////////////
KCharsets::KCharsets(){

  if (!data){
     data=new KCharsetsData();
     KCharsetConverterData::kcharsetsData=data;
     KCharset::data=data;
     KCharset::charsets=this;
  }   
}

KCharsets::~KCharsets(){

}

QString KCharsets::defaultCharset()const{

   warning("KCharsets::defaultCharset() is obsolete. Use KCharsets::defaultCh() insteed");
   return defaultCh().name();
}

KCharset KCharsets::defaultCh()const{

  return data->defaultCharset();
}

bool KCharsets::setDefault(KCharset ch){

  if ( ch.ok() ){
     data->setDefaultCharset(ch.entry);
     return TRUE; 
  }   
  warning("Wrong charset! Setting to default (us-ascii)");
  const KCharsetEntry *ce=data->charsetEntry("us-ascii");
  data->setDefaultCharset(ce);
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
    if (isDisplayable(charset))
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

bool KCharsets::isAvailable(KCharset charset){

  return charset.isAvailable();
}

bool KCharsets::isDisplayable(KCharset charset){

  return charset.isDisplayable();
}

bool KCharsets::isRegistered(KCharset charset){

  return charset.isRegistered();
}

int KCharsets::bits(KCharset charset){

  return charset.bits();
}

const char * KCharsets::name(QFont::CharSet qtcharset){

  if (qtcharset==QFont::AnyCharSet) return "unknown";
  return KCharset(qtcharset);
}

QFont::CharSet KCharsets::qtCharset(){

  return qtCharset(data->defaultCharset()->name);
}

QFont &KCharsets::setQFont(QFont &fnt,KCharset charset){

  return charset.setQFont(fnt);
}


QFont &KCharsets::setQFont(QFont &fnt){

  return KCharset(data->defaultCharset()).setQFont(fnt);
}

const char * KCharsets::name(const QFont& font){

  if (font.charSet()!=QFont::AnyCharSet) return name(font.charSet());
  const KCharsetEntry * ce=data->charsetOfFace(font.family());
  kchdebug("ce=%p ce->name=%s\n",ce,ce?ce->name:0);
  if (ce) return ce->name;
  else return "unknown";
}

KCharset KCharsetConversionResult::charset()const{

  if (cCharset == 0) return "";
  return cCharset;
}

QFont & KCharsetConversionResult::setQFont(QFont &font)const{

  if (!cCharset) return font;
  return KApplication::getKApplication()
         ->getCharsets()->setQFont(font,cCharset->name);
}

static KCharsetConversionResult convResult;

const KCharsetConversionResult & KCharsets::convert(unsigned ch){
  
  data->convert(ch,convResult);
  return convResult;
}

const KCharsetConversionResult & KCharsets::convertTag(const char *tag){

  int tmp;
  data->convertTag(tag,convResult,tmp);
  return convResult;
}
 
const KCharsetConversionResult & KCharsets::convertTag(const char *tag
							,int &l){

  data->convertTag(tag,convResult,l);
  return convResult;
}
 
