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
// Revision 1.2  1997/12/09 17:14:24  denis
// Some modifications to compile more strict compilers that gcc
// Guys!!! If you want to delete a static member ... Think before...
//
// Revision 1.1  1997/12/08 17:13:23  jacek
// *** empty log message ***
//
// Revision 1.8  1997/12/08 15:25:40  jacek
// *** empty log message ***
//
// Revision 1.7  1997/12/08 11:22:24  jacek
// *** empty log message ***
//
// Revision 1.6  1997/12/06 10:25:54  jacek
// Copyright information added
//
// Revision 1.5  1997/12/06 10:22:05  jacek
// kcharsets.h docified
//
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
    printf("Couldn't set input charset to %s\n",inputCharset);
    return;
  }  
  output=kcharsetsData->charsetEntry(outputCharset);
  if (!output) {
    printf("Couldn't set output charset to %s\n",outputCharset);
    return;
  }
  
  setInputSettings();
  setOutputSettings();

  if ( !input || !output || input==output ) conversionType=NoConversion;
  else if (unicodeIn && unicodeOut) conversionType=UnicodeUnicode;
  else if (unicodeIn){
    conversionType=FromUnicode;
    if (!createFromUnicodeDict()) return;
  }  
  else if (unicodeOut){
    conversionType=ToUnicode;
    if (!getToUnicodeTable()) return;
  }  
  else{
    conversionType=EightToEight;
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
      printf("Couldn't get conversion table nor dictionary\n");
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
    printf("Couldn't set output charset to %s\n",output->name);
    return;
  }
  output=kcharsetsData->charsetEntry("iso-8859-1");
  if (!output) {
    printf("Couldn't set output charset to iso-8859-1\n");
    return;
  }
  setInputSettings();
  setOutputSettings();
  isOK=TRUE;
}

void KCharsetConverterData::setInputSettings(){

  const char *name=input->name;
  
  if ( ! stricmp(name,"utf7") ){
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

bool KCharsetConverterData::decodeUTF8(const char*,unsigned int &,int &){

  return FALSE;
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
  for(i=0;inBits?str[i]:(str[i]&&str[i+1]);inBits?i++:i+=2){
      i++;
    else
      i+=2;

    switch(inputEnc){
       case UTF7:
         if (decodeUTF7(str+i,index,tmp)) i+=tmp;
	 else index=str[i];
	 break;
       case UTF8:
         if (decodeUTF8(str+i,index,tmp)) i+=tmp;
	 else index=str[i];
	 break;
       default:
         if (inBits<=8) index=(unsigned char)str[i];
	 else if (inBits==16) index=(unsigned char)str[i++]+(unsigned char)str[i]<<8;
	 break;
    }
    printf("Got index: %x\n",index);
    switch(conversionType){
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
         printf("Converted to unicode: %x\n",index);
	 if (index2){
            ptr=(*convFromUniDict)[index2];
  	    if (ptr) chr=*ptr;
	    else chr=0;
	 }   
	 else chr=0;
         break;
    }
    printf("Converted to: %x\n",chr);
    if (chr==0)
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
        result.text+=(char)(chr&255);
	result.text+=(char)(chr>>8);
      }
      else result.text+=(char)chr;
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
      printf("Couldn't get to unicode table for %s\n",output->name);
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

  printf("Scanning directory: %s\n",path);
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


  printf("Creating unicode dict for %s\n",e->name);
  config->setGroup("general");
  QString dir=config->readEntry("i18ndir");
  printf("Dir: %s\n",(const char *)dir);
  QString filename=dir+'/'+e->name;
  printf("Trying to open file %s\n",(const char *)filename);
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
    printf("(%s %s) %x->%x\n",codeBuf,unicodeBuf,code,unicode);
    dict->insert(code,new unsigned(unicode));	
  }
  e->toUnicodeDict=dict;
}
  
KCharsetsData::~KCharsetsData(){

    if (e->name) delete e->name;
  while( (e=it.current()) ){
    if (e->toUnicodeDict) delete e->toUnicodeDict;
// delete static members ... It is a new vision of C++ :-)
//    if (e->name) delete e->name;
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

  printf("Testing if face %s is of charset %s...",(const char *)face,charset);
  config->setGroup("faces");
  const char *faceStr=config->readEntry(charset);
  printf("%s...",faceStr);
  QRegExp rexp(faceStr,FALSE,TRUE);
  if (face.contains(rexp)){
    printf("Yes, it is\n");
    return TRUE;
  }  
  printf("No, it isn't\n");
  return FALSE;
}

KCharsetEntry* KCharsetsData::charsetOfFace(const QString &face){

  printf("Searching for charset for face %s...\n",(const char *)face);
  KEntryIterator * it=config->entryIterator("faces");
  if (!it) return 0;
  while( it->current() ){
    QString faceStr=it->current()->aValue;
    printf("testing if it is %s (%s)...",(const char *)it->currentKey()
                                             ,(const char *)faceStr);
    QRegExp rexp(faceStr,FALSE,TRUE);
    if (face.contains(rexp)){
      printf("Yes, it is\n");
      return charsetEntry(it->currentKey());
    }  
    printf("No, it isn't\n");
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
