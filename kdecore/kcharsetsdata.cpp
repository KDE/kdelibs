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
#include <qdir.h>
#include <qfile.h>
#include <stdlib.h>
#include <qfontinf.h>
#include <qintdict.h>
#include <kapp.h>
#include <ksimpleconfig.h>
#include <qregexp.h>

KCharsetConverterData::KCharsetConverterData(const char * inputCharset,bool iamps,
                       const char * outputCharset,bool oamps){

  convTable=0;
  convToUniDict=0;
  convFromUniDict=0;

  isOK=FALSE;
  inAmps=iamps;
  outAmps=oamps;
  if ( kcharsetsData == 0 ) fatal("KCharsets not initialized!");
  
  input=kcharsetsData->charsetEntry(inputCharset);
  if (!input) {
    kchdebug("Couldn't set input charset to %s\n",inputCharset);
    return;
  }  
  output=kcharsetsData->charsetEntry(outputCharset);
  if (!output) {
    kchdebug("Couldn't set output charset to %s\n",outputCharset);
    return;
  }
  
  setInputSettings();
  setOutputSettings();

  if ( !input || !output || input==output ) conversionType=NoConversion;
  else if (unicodeIn && unicodeOut) conversionType=UnicodeUnicode;
  else if (unicodeIn){
    conversionType=FromUnicode;
    kchdebug("Conversion: from unicode\n");
    if (!createFromUnicodeDict()) return;
  }  
  else if (unicodeOut){
    conversionType=ToUnicode;
    kchdebug("Conversion: to unicode\n");
    if (!getToUnicodeTable()) return;
  }  
  else{
    conversionType=EightToEight;
    kchdebug("Conversion: no unicode\n");
    if (!getToUnicodeTable()) return;
    if (!createFromUnicodeDict()) return;
  } 
  isOK=TRUE;
}

KCharsetConverterData::~KCharsetConverterData(){

  if (convFromUniDict) delete convFromUniDict;
}

bool KCharsetConverterData::getToUnicodeTable(){

  convTable=kcharsetsData->getToUnicodeTable(input->name);
  if (!convTable){
    convToUniDict=kcharsetsData->getToUnicodeDict(input->name);
    if (!convToUniDict){
      kchdebug("Couldn't get conversion table nor dictionary\n");
      return FALSE;
    }
  }
  return TRUE;
}

KCharsetConverterData::KCharsetConverterData(const char * inputCharset,bool iamps,bool oamps){

  convTable=0;
  convToUniDict=0;
  convFromUniDict=0;

  isOK=FALSE;
  if ( kcharsetsData == 0 ) fatal("KCharsets not initialized!");
  inAmps=iamps;
  outAmps=oamps;
  input=kcharsetsData->charsetEntry(inputCharset);
  if (!input) {
    kchdebug("Couldn't set output charset to %s\n",output->name);
    return;
  }
  output=kcharsetsData->charsetEntry("iso-8859-1");
  if (!output) {
    kchdebug("Couldn't set output charset to iso-8859-1\n");
    return;
  }
  setInputSettings();
  setOutputSettings();
  isOK=TRUE;
}

void KCharsetConverterData::setInputSettings(){

  const char *name=input->name;
  
  if ( ! stricmp(name,"utf7") ){
    warning("Sorry, UTF7 encoding is not supported yet\n");
    inputEnc=UTF7;
    inBits=0;
    unicodeIn=TRUE;
  }  
  else if ( ! stricmp(name,"utf8") ){
    inputEnc=UTF8;
    inBits=0;
    unicodeIn=TRUE;
  }  
  else if ( ! stricmp(name,"unicode") ){
    inputEnc=none;
    inBits=16;
    unicodeIn=TRUE;
  }
  else if ( ! stricmp(name,"us-ascii") ){
    inputEnc=none;
    inBits=7;
    unicodeIn=FALSE;
  }
  else{
    inputEnc=none;
    inBits=8;
    unicodeIn=FALSE;
  } 
}

void KCharsetConverterData::setOutputSettings(){

  const char *name=output->name;
  
  if ( ! stricmp(name,"utf7") ){
    warning("Sorry, UTF7 encoding is not supported yet\n");
    outputEnc=UTF7;
    outBits=0;
    unicodeOut=TRUE;
  }  
  else if ( ! stricmp(name,"utf8") ){
    outputEnc=UTF8;
    outBits=0;
    unicodeOut=TRUE;
  }  
  else if ( ! stricmp(name,"unicode") ){
    outputEnc=none;
    outBits=16;
    unicodeOut=TRUE;
  }
  else if ( ! stricmp(name,"us-ascii") ){
    outputEnc=none;
    outBits=7;
    unicodeOut=FALSE;
  }
  else{
    outputEnc=none;
    outBits=8;
    unicodeOut=FALSE;
  } 
}


bool KCharsetConverterData::decodeUTF7(const char*,unsigned int &,int &){

  return FALSE;
}

bool KCharsetConverterData::decodeUTF8(const char*str,unsigned int &code
                                      ,int &extrachars){
  code=0;
  extrachars=0;
  unsigned char chr=*str;
  kchdebug("str: ");
  for(int i=0;i<6 && str[i];i++)
    kchdebug("%02x ",(int)(unsigned char)str[i]);
  kchdebug("\n");
  if ( (chr&0x80)==0 ){
    code=chr&0x7f;
    extrachars=0;
  }
  else if ( (chr&0xe0)==0xc0 ){
    code=chr&0x1f;
    extrachars=1;
  }
  else if ( (chr&0xf0)==0xe0 ){
    code=chr&0x0f;
    extrachars=2;
  }  
  else if ( (chr&0xf8)==0xf0 ){
    code=chr&0x07;
    extrachars=3;
  }  
  else if ( (chr&0xfc)==0xf8 ){
    code=chr&0x03;
    extrachars=4;
  }  
  else if ( (chr&0xfe)==0xf8 ){
    code=chr&0x01;
    extrachars=5;
  }  
  else {
    warning("Invalid UTF8 sequence!");
    return FALSE;
  }  

  int chars=extrachars;
  while(chars>0){
    str++;
    code<<=6;
    kchdebug("Code: %4x char: %2x masked char: %2x\n",code,*str,(*str)&0x3f);
    code|=(*str)&0x3f;
    chars--;
  }
  return TRUE;
}

bool KCharsetConverterData::encodeUTF7(unsigned int,QString &){

  return FALSE;
}

bool KCharsetConverterData::encodeUTF8(unsigned int code,QString &result){

  if (code<0x80){
    result+=(char)code;
    return TRUE;
  }  
  kchdebug("Code: %4x\n",code);
  int octets=2;
  unsigned mask1=0xc0;
  unsigned mask2=0x1f;
  unsigned range=0x800;
  int left=24;
  while(code>range){
    if (range>=0x40000000){
      warning("Unicode value too big!");
      return FALSE;
    }
    mask2=(mask2>>1)&0x80;
    mask1>>=1;
    range<<=5;
    octets++;
    left-=6;
  }
  kchdebug("octets: %i  mask1: %x mask2: %x range: %x left: %i\n"
            ,octets,mask1,mask2,range,left);
  unsigned char chr=((code>>((octets-1)*6))&mask2)|mask1;
  kchdebug("Chars: %02x ",chr);
  result+=chr;
  octets--;
  unsigned int tmp=(code<<left)&0xffffffff;
  while(octets>0){
    chr=((tmp>>24)&0x3f)|0x80;
    kchdebug("%02x ",chr);
    result+=chr;
    tmp<<=6;
    octets--;
  }
  kchdebug("\n");
  return TRUE;
}
   
void KCharsetConverterData::convert(const QString &str
                                    ,KCharsetConversionResult &result) {


  if (!isOK) return;
  if (conversionType == NoConversion ){
    result.text=str;
    return ;
  }
  result.text="";
  
  int i;
  int tmp;
  unsigned *ptr=0;
  unsigned index=0;
  unsigned index2=0;
  unsigned chr=0;
  
  for(i=0;(inBits<=8)?str[i]:(str[i]&&str[i+1]);){
    switch(inputEnc){
       case UTF7:
         if (decodeUTF7(((const char *)str)+i,index,tmp)) i+=tmp;
	 else index=(unsigned char)str[i];
	 break;
       case UTF8:
         if (decodeUTF8(((const char *)str)+i,index,tmp)) i+=tmp;
	 else index=(unsigned char)str[i];
	 break;
       default:
         if (inBits<=8) index=(unsigned char)str[i];
	 else if (inBits==16) index=(((unsigned char)str[i++])<<8)+(unsigned char)str[i];
	 break;
    }
    kchdebug("Got index: %x\n",index);
    if (index>0) switch(conversionType){
       case ToUnicode:
         if (convTable)
	   chr=convTable[index];
	 else if (convToUniDict) {
           ptr=(*convToUniDict)[index];
	   if (ptr) chr=*ptr;
	   else chr=0;
	 }  
	 break;
       case FromUnicode:
         ptr=(*convFromUniDict)[index];
	 if (ptr) chr=*ptr;
	 else chr=0;
	 break;
       case UnicodeUnicode:
         chr=index;
	 break;
       default:
         if (convTable)
	   index2=convTable[index];
	 else{
           ptr=(*convToUniDict)[index];
 	   if (ptr) index2=*ptr;
	   else index2=0;
	 }  
         kchdebug("Converted to unicode: %4x\n",index);
	 if (index2){
            ptr=(*convFromUniDict)[index2];
  	    if (ptr) chr=*ptr;
	    else chr=0;
	 }   
	 else chr=0;
         break;
    }
    else chr=0;
    kchdebug("Converted to: %x\n",chr);
    if (outputEnc==UTF8) encodeUTF8(chr,result.text);
    else if (outputEnc==UTF7) encodeUTF7(chr,result.text);
    else if (chr==0)
      if (outAmps){
        if (conversionType!=FromUnicode){
	  if (convTable)
	     index2=convTable[index];
	   else{
             ptr=(*convToUniDict)[index];
 	     if (ptr) index2=*ptr;
	     else index2=0;
	   }  
	}   
        else index2=index;
	if (index2) result.text+="&#"+QString().setNum(index2)+';';
	else result.text+="?";
      }  
      else result.text+="?";
    else
      if (outBits==16){
        result.text+=(char)(chr>>8);
	result.text+=(char)(chr&255);
      }
      else result.text+=(char)chr;
      
    if (inBits<=8)
      i++;
    else
      i+=2;
  }
}

bool KCharsetConverterData::createFromUnicodeDict(){

  QIntDict<unsigned> * dict=new QIntDict<unsigned>;
  dict->setAutoDelete(TRUE);
  const unsigned *tbl=kcharsetsData->getToUnicodeTable(output->name);
  if (tbl)
    for(int i=0;i<(1<<outBits);i++)
      dict->insert(tbl[i],new unsigned(i));
  else{
    QIntDict<unsigned> * dict2=kcharsetsData->getToUnicodeDict(output->name);
    if (!dict2){
      kchdebug("Couldn't get to unicode table for %s\n",output->name);
      delete dict;
      return FALSE;
    }
    QIntDictIterator<unsigned> it(*dict2);
    while( it.current() ){
      dict->insert(*it.current(),new unsigned(it.currentKey()));
      ++it;
    }
  }
  convFromUniDict=dict;
  return TRUE;
}

/////////////////////////////////////////////////

KCharsetsData::KCharsetsData(){

  QString fileName=KApplication::kdedir();
  fileName+="/share/config/charsets";
  config=new KSimpleConfig(fileName);
  config->setGroup("general");
  const char * i18dir=config->readEntry("i18ndir");
  if (i18dir) scanDirectory(i18dir);
}

void KCharsetsData::scanDirectory(const char *path){

  kchdebug("Scanning directory: %s\n",path);
  QDir d(path);
  d.setFilter(QDir::Files);
  d.setSorting(QDir::Name);
  const QFileInfoList *list=d.entryInfoList();
  QFileInfoListIterator it(*list);
  QFileInfo *fi;
  while( (fi=it.current()) ){
    QString name=fi->fileName();
    QString alias=name;
    int comma=alias.find(',');
    if (comma) alias.remove(comma,alias.length()-comma);
    else alias="";
    if (!charsetEntry(alias) && !charsetEntry(name)){
      KCharsetEntry *entry=new KCharsetEntry;
      char *ptr=new char [fi->fileName().length()+1];
      strcpy(ptr,name);
      entry->name=ptr;
      entry->qtCharset=QFont::AnyCharSet;
      entry->toUnicode=0;
      entry->registered=FALSE;
      entry->toUnicodeDict=0;
      i18nCharsets.insert(entry->name,entry);
      if (alias!="") aliases.insert(alias,entry);
    }	
    ++it;
  }
}

void KCharsetsData::createDictFromi18n(KCharsetEntry *e){


  kchdebug("Creating unicode dict for %s\n",e->name);
  config->setGroup("general");
  QString dir=config->readEntry("i18ndir");
  kchdebug("Dir: %s\n",(const char *)dir);
  QString filename=dir+'/'+e->name;
  kchdebug("Trying to open file %s\n",(const char *)filename);
  QFile f(filename);
  if (!f.open(IO_ReadOnly)) return;
  QTextStream t(&f);
  QString l;
  do{
    l=t.readLine();
  }while(!t.eof() && l!="CHARMAP");
  if (t.eof()){
    f.close();
    return;
  }
  
  QIntDict<unsigned> *dict=new QIntDict<unsigned>;
  dict->setAutoDelete(TRUE);
  
  char codeBuf[20];
  char unicodeBuf[10];
  unsigned unicode;
  unsigned code;
  
  while(!t.eof()){
    l=t.readLine();
    if (l=="END CHARMAP") break;
    sscanf(l,"%*s %16s %8s %*s",codeBuf,unicodeBuf);
    sscanf(unicodeBuf,"<U%x>",&unicode);
    code=0;
    if ( sscanf(codeBuf,"/x%X",&code) < 1 )
      code=codeBuf[0];
    kchdebug("(%s %s) %x->%x\n",codeBuf,unicodeBuf,code,unicode);
    dict->insert(code,new unsigned(unicode));	
  }
  e->toUnicodeDict=dict;
}
  
KCharsetsData::~KCharsetsData(){

  QDictIterator<KCharsetEntry> it(i18nCharsets);
  KCharsetEntry *e;
  while( (e=it.current()) ){
    if (e->toUnicodeDict) delete e->toUnicodeDict;
    if (e->name) delete e->name;
    delete e;
  }
  delete config;
}

KCharsetEntry * KCharsetsData::charsetEntry(const char *name){

  config->setGroup("aliases");
  const char *alias=config->readEntry(name);
  if (alias) name=alias;
  for(int i=0;charsets[i].name;i++)
    if ( stricmp(name,charsets[i].name) == 0 ) return charsets+i;
  KCharsetEntry *e=i18nCharsets[name];
  if (!e) e=aliases[name];
  return e;
}

KCharsetEntry * KCharsetsData::charsetEntry(int index){

  int i;
  for(i=0;charsets[i].name;i++)
    if ( i==index ) return charsets+i;
    
  QDictIterator<KCharsetEntry> it(i18nCharsets);
  KCharsetEntry *e;
  while( (e=it.current()) ){
    if (i==index) return e;
    ++i;
    ++it;
  }
  return 0;
}

bool KCharsetsData::setDefaultCharset(const char *name){

  if (charsetEntry(name)){
    defaultCh=name;
    return TRUE;
  }
  return FALSE;
}

QString KCharsetsData::charsetFace(const char *name,const QString &face){

  config->setGroup("faces");
  const char *faceStr=config->readEntry(name);
  if (!faceStr) return face;
  QString newFace(faceStr);
  newFace.replace(QRegExp("\\*"),face);
  return newFace;
}

bool KCharsetsData::charsetOfFace(const char * charset,const QString &face){

  kchdebug("Testing if face %s is of charset %s...",(const char *)face,charset);
  config->setGroup("faces");
  const char *faceStr=config->readEntry(charset);
  kchdebug("%s...",faceStr);
  QRegExp rexp(faceStr,FALSE,TRUE);
  if (face.contains(rexp)){
    kchdebug("Yes, it is\n");
    return TRUE;
  }  
  kchdebug("No, it isn't\n");
  return FALSE;
}

KCharsetEntry* KCharsetsData::charsetOfFace(const QString &face){

  kchdebug("Searching for charset for face %s...\n",(const char *)face);
  KEntryIterator * it=config->entryIterator("faces");
  if (!it) return 0;
  while( it->current() ){
    const char * faceStr=it->current()->aValue;
    kchdebug("testing if it is %s (%s)...",(const char *)it->currentKey(),faceStr);
    QRegExp rexp(faceStr,FALSE,TRUE);
    kchdebug("regexp: %s face: %s\n",rexp.pattern(),(const char *)face);
    if (face.contains(rexp)){
      kchdebug("Yes, it is\n");
      return charsetEntry(it->currentKey());
    }  
    kchdebug("No, it isn't\n");
    ++(*it);
  }
  return 0;
}
  
const unsigned *KCharsetsData::getToUnicodeTable(const char *charset){

  KCharsetEntry *ce=charsetEntry(charset);
  if (!ce) return 0;
  return ce->toUnicode;
}
  
QIntDict<unsigned> *KCharsetsData::getToUnicodeDict(const char *charset){

  KCharsetEntry *ce=charsetEntry(charset);
  if (!ce) return 0;
  if (ce->toUnicodeDict == 0) createDictFromi18n(ce);
  return ce->toUnicodeDict;
}

const char *KCharsetsData::faceForCharset(const char *charset){

  config->setGroup("faces");
  return config->readEntry(charset);
}
